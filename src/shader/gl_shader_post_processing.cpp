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

bool prosper::util::convert_glsl_set_bindings_to_opengl_binding_points(std::vector<std::string> &inOutGlslCodePerStage, std::string &outErrMsg)
{
	std::vector<std::optional<prosper::IPrContext::ShaderDescriptorSetInfo>> descSetInfos {};
	struct StageData {
		std::unordered_map<std::string, int32_t> definitions {};
		std::vector<prosper::IPrContext::ShaderMacroLocation> macroLocations {};
	};
	std::vector<StageData> stages {};
	stages.reserve(inOutGlslCodePerStage.size());
	// We need to collect the descriptor set infos for ALL shader stages before we continue
	for(auto &inOutGlslCode : inOutGlslCodePerStage) {
		stages.push_back({});
		auto &stageData = stages.back();
		auto &definitions = stageData.definitions;

		auto &macroLocations = stageData.macroLocations;
		IPrContext::ParseShaderUniforms(inOutGlslCode, definitions, descSetInfos, macroLocations);
	}

	uint32_t descSetIdx = 0;
	uint32_t bufferBindingPointIdx = 1; // 0 is reserved for push constants
	uint32_t imageBindingPointIdx = 0;  // Image binding points are independent of buffers in OpenGL
	for(auto &dsInfo : descSetInfos) {
		if(dsInfo.has_value() == false) {
			outErrMsg = "DescriptorSet " + std::to_string(descSetIdx) + " is undefined!";
			return false;
		}
		for(auto &bp : dsInfo->bindingPoints) {
			switch(bp.type) {
			case prosper::IPrContext::ShaderDescriptorSetBindingInfo::Type::Buffer:
				bp.bindingPoint = bufferBindingPointIdx;
				bufferBindingPointIdx += bp.bindingCount;
				break;
			case prosper::IPrContext::ShaderDescriptorSetBindingInfo::Type::Image:
				bp.bindingPoint = imageBindingPointIdx;
				imageBindingPointIdx += bp.bindingCount;
				break;
			}
		}
		++descSetIdx;
	}

	// Apply binding points to GLSL shader code
	uint32_t stageIdx = 0;
	for(auto &stageData : stages) {
		auto &inOutGlslCode = inOutGlslCodePerStage.at(stageIdx++);
		auto &newGlslShader = inOutGlslCode;
		int64_t offset = 0;
		for(auto &loc : stageData.macroLocations) {
			auto strStart = loc.macroStart;
			auto strEnd = loc.macroEnd;
			auto oldLen = newGlslShader.length();
			newGlslShader = newGlslShader.substr(0, strStart + offset) + "binding = " + std::to_string(descSetInfos.at(loc.setIndexIndex)->bindingPoints.at(loc.bindingIndexIndex).bindingPoint) + newGlslShader.substr(strEnd + offset);
			offset += (newGlslShader.length() - oldLen);
		}
	}
	return true;
}
