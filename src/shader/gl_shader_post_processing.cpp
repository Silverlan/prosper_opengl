/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gl_api.hpp"
#include "shader/gl_shader_post_processing.hpp"
#include "gl_context.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <functional>
#include <sharedutils/util_string.h>
#include <mpParser.h>
#pragma optimize("",off)
static size_t parse_definition(const std::string &glslShader,std::unordered_map<std::string,std::string> &outDefinitions,size_t startPos=0)
{
	auto pos = glslShader.find("#define ",startPos);
	if(pos == std::string::npos)
		return pos;
	pos += 8;
	auto nameStart = glslShader.find_first_not_of(" ",pos);
	auto nameEnd = glslShader.find(' ',nameStart);
	if(nameEnd == std::string::npos)
		return nameEnd;
	auto name = glslShader.substr(nameStart,nameEnd -nameStart);
	auto valStart = glslShader.find_first_not_of(" ",nameEnd);
	auto valEnd = glslShader.find_first_of("\r\n",valStart);
	if(valEnd == std::string::npos)
		return valEnd;
	auto val = glslShader.substr(valStart,valEnd -valStart);
	ustring::remove_whitespace(name);
	ustring::remove_whitespace(val);
	auto it = outDefinitions.find(name);
	if(it == outDefinitions.end())
		outDefinitions.insert(std::make_pair(name,val));
	return valEnd;
}

static void parse_definitions(const std::string &glslShader,std::unordered_map<std::string,std::string> &outDefinitions)
{
	auto nextPos = parse_definition(glslShader,outDefinitions);
	while(nextPos != std::string::npos)
		nextPos = parse_definition(glslShader,outDefinitions,nextPos);
}

static void parse_definitions(const std::string &glslShader,std::unordered_map<std::string,int32_t> &outDefinitions)
{
	std::unordered_map<std::string,std::string> definitions {};
	parse_definitions(glslShader,definitions);

	struct Expression
	{
		std::string expression;
		mup::ParserX parser {};
		std::optional<int> value {};

		std::vector<std::string> variables {};
	};
	std::unordered_map<std::string,Expression> expressions {};
	expressions.reserve(definitions.size());
	for(auto &pair : definitions)
	{
		Expression expr {};
		expr.expression = pair.second;
		expr.parser.SetExpr(pair.second);
		try
		{
			for(auto &a : expr.parser.GetExprVar())
				expr.variables.push_back(a.first);
		}
		catch(const mup::ParserError &err)
		{
			continue;
		}
		expressions.insert(std::make_pair(pair.first,expr));
	}
	std::function<bool(Expression&)> fParseExpression = nullptr;
	fParseExpression = [&fParseExpression,&expressions](Expression &expr) {
		for(auto &var : expr.variables)
		{
			auto it = expressions.find(var);
			if(it == expressions.end())
				return false;
			if(it->second.value.has_value() == false && fParseExpression(it->second) == false)
				return false;
			if(expr.parser.IsConstDefined(var))
				continue;
			try
			{
				expr.parser.DefineConst(var,*it->second.value);
			}
			catch(const mup::ParserError &err)
			{
				return false;
			}
		}
		try
		{
			expr.parser.SetExpr(expr.expression);
			auto &val = expr.parser.Eval();
			expr.value = val.GetFloat();
		}
		catch(const mup::ParserError &err)
		{
			return false;
		}
		return true;
	};
	for(auto &pair : expressions)
	{
		auto &expr = pair.second;
		fParseExpression(expr);
		if(pair.second.value.has_value() == false)
			continue;
		outDefinitions.insert(std::make_pair(pair.first,*pair.second.value));
	}
}

struct ShaderDescriptorSetBindingInfo
{
	enum class Type : uint8_t
	{
		Buffer = 0,
		Image
	};
	uint32_t bindingPoint = 0;
	uint32_t bindingCount = 1;
	Type type = Type::Buffer;
};
struct ShaderDescriptorSetInfo
{
	std::vector<ShaderDescriptorSetBindingInfo> bindingPoints {};
};
struct ShaderMacroLocation
{
	size_t macroStart = 0;
	size_t macroEnd = 0;
	uint32_t setIndexIndex = 0;
	uint32_t bindingIndexIndex = 0;
};
static const std::unordered_set<std::string> g_imageTypes = {
	"sampler1D",
	"sampler2D",
	"sampler3D",
	"samplerCube",
	"sampler2DRect",
	"sampler1DArray",
	"sampler2DArray",
	"samplerCubeArray",
	"samplerBuffer",
	"sampler2DMS",
	"sampler2DMSArray",
	"sampler1DShadow",
	"sampler2DShadow",
	"samplerCubeShadow",
	"sampler2DRectShadow",
	"sampler1DArrayShadow",
	"sampler2DArrayShadow",
	"samplerCubeArrayShadow",
	"image1D",
	"image2D",
	"image3D",
	"imageCube",
	"image2DRect",
	"image1DArray",
	"image2DArray",
	"imageCubeArray",
	"imageBuffer",
	"image2DMS",
	"image2DMSArray"
};
static size_t parse_layout_binding(
	const std::string &glslShader,const std::unordered_map<std::string,int32_t> &definitions,
	std::vector<std::optional<ShaderDescriptorSetInfo>> &outDescSetInfos,
	std::vector<ShaderMacroLocation> &outMacroLocations,size_t startPos
)
{
	auto pos = glslShader.find("LAYOUT_ID(",startPos);
	if(pos == std::string::npos)
		return pos;
	auto macroPos = pos;
	pos += 10;
	auto end = glslShader.find(")",pos);
	if(end == std::string::npos)
		return end;
	auto macroEnd = end +1;
	auto strArgs = glslShader.substr(pos,end -pos);
	std::vector<std::string> args {};
	ustring::explode(strArgs,",",args);
	if(args.size() < 2)
		return end;
	auto &set = args.at(0);
	auto &binding = args.at(1);
	std::optional<uint32_t> setIndex {};
	std::optional<uint32_t> bindingIndex {};
	if(ustring::is_integer(set))
		setIndex = ustring::to_int(set);
	else
	{
		// Set is a definition
		auto itSet = definitions.find(set);
		if(itSet != definitions.end())
			setIndex = itSet->second;
	}
	if(ustring::is_integer(binding))
		bindingIndex = ustring::to_int(binding);
	else
	{
		// Binding is a definition
		auto itBinding = definitions.find(binding);
		if(itBinding != definitions.end())
			bindingIndex = itBinding->second;
	}

	if(setIndex.has_value() && bindingIndex.has_value())
	{
		// Determine type
		auto defEndPos = umath::min(glslShader.find(';',macroEnd),glslShader.find('{',macroEnd));
		auto type = ShaderDescriptorSetBindingInfo::Type::Buffer;
		uint32_t numBindings = 1;
		if(defEndPos != std::string::npos)
		{
			auto line = glslShader.substr(macroEnd,defEndPos -macroEnd);
			auto argStartPos = line.find_last_of(')');
			line = line.substr(argStartPos +1);
			std::vector<std::string> args {};
			ustring::explode_whitespace(line,args);
			if(args.size() > 1)
			{
				auto &strType = args.at(1);
				auto itType = g_imageTypes.find(strType);
				if(itType != g_imageTypes.end())
					type = ShaderDescriptorSetBindingInfo::Type::Image;

				if(type == ShaderDescriptorSetBindingInfo::Type::Image) // TODO: We also need this for buffers
				{
					auto &arg = args.back();
					auto brStart = arg.find('[');
					auto brEnd = arg.find(']',brStart);
					if(brEnd != std::string::npos)
					{
						++brStart;
						// It's an array
						auto strNumBindings = arg.substr(brStart,brEnd -brStart);
						if(ustring::is_integer(strNumBindings))
							numBindings = ustring::to_int(strNumBindings);
						else
						{
							// Binding count is a definition
							auto itBinding = definitions.find(strNumBindings);
							if(itBinding != definitions.end())
								numBindings = itBinding->second;
						}
					}
				}
			}
		}
		//

		if(*setIndex >= outDescSetInfos.size())
			outDescSetInfos.resize(*setIndex +1);
		auto &setInfo = outDescSetInfos.at(*setIndex);
		if(setInfo.has_value() == false)
			setInfo = ShaderDescriptorSetInfo{};
		if(*bindingIndex >= setInfo->bindingPoints.size())
			setInfo->bindingPoints.resize(*bindingIndex +1);
		setInfo->bindingPoints.at(*bindingIndex).type = type;
		setInfo->bindingPoints.at(*bindingIndex).bindingCount = numBindings;

		outMacroLocations.push_back({});
		auto &macroLocation = outMacroLocations.back();
		macroLocation.macroStart = macroPos;
		macroLocation.macroEnd = macroEnd;
		macroLocation.setIndexIndex = *setIndex;
		macroLocation.bindingIndexIndex = *bindingIndex;
		// setInfo->bindingPoints.at(*bindingIndex) = true;
	}
	return end;
}

bool prosper::util::convert_glsl_set_bindings_to_opengl_binding_points(std::vector<std::string> &inOutGlslCodePerStage,std::string &outErrMsg)
{
	std::vector<std::optional<ShaderDescriptorSetInfo>> descSetInfos {};
	struct StageData
	{
		std::unordered_map<std::string,int32_t> definitions {};
		std::vector<ShaderMacroLocation> macroLocations {};
	};
	std::vector<StageData> stages {};
	stages.reserve(inOutGlslCodePerStage.size());
	// We need to collect the descriptor set infos for ALL shader stages before we continue
	for(auto &inOutGlslCode : inOutGlslCodePerStage)
	{
		stages.push_back({});
		auto &stageData = stages.back();
		auto &definitions = stageData.definitions;
		parse_definitions(inOutGlslCode,definitions);

		auto &macroLocations = stageData.macroLocations;
		auto nextPos = parse_layout_binding(inOutGlslCode,definitions,descSetInfos,macroLocations,0u);
		while(nextPos != std::string::npos)
			nextPos = parse_layout_binding(inOutGlslCode,definitions,descSetInfos,macroLocations,nextPos);
	}

	uint32_t descSetIdx = 0;
	uint32_t bufferBindingPointIdx = 1; // 0 is reserved for push constants
	uint32_t imageBindingPointIdx = 0; // Image binding points are independent of buffers in OpenGL
	for(auto &dsInfo : descSetInfos)
	{
		if(dsInfo.has_value() == false)
		{
			outErrMsg = "DescriptorSet " +std::to_string(descSetIdx) +" is undefined!";
			return false;
		}
		for(auto &bp : dsInfo->bindingPoints)
		{
			switch(bp.type)
			{
			case ShaderDescriptorSetBindingInfo::Type::Buffer:
				bp.bindingPoint = bufferBindingPointIdx;
				bufferBindingPointIdx += bp.bindingCount;
				break;
			case ShaderDescriptorSetBindingInfo::Type::Image:
				bp.bindingPoint = imageBindingPointIdx;
				imageBindingPointIdx += bp.bindingCount;
				break;
			}
		}
		++descSetIdx;
	}

	// Apply binding points to GLSL shader code
	uint32_t stageIdx = 0;
	for(auto &stageData : stages)
	{
		auto &inOutGlslCode = inOutGlslCodePerStage.at(stageIdx++);
		auto &newGlslShader = inOutGlslCode;
		int64_t offset = 0;
		for(auto &loc : stageData.macroLocations)
		{
			auto strStart = loc.macroStart;
			auto strEnd = loc.macroEnd;
			auto oldLen = newGlslShader.length();
			newGlslShader = newGlslShader.substr(0,strStart +offset) +"binding = " +std::to_string(descSetInfos.at(loc.setIndexIndex)->bindingPoints.at(loc.bindingIndexIndex).bindingPoint) +newGlslShader.substr(strEnd +offset);
			offset += (newGlslShader.length() -oldLen);
		}
	}
	return true;
}
#pragma optimize("",on)
