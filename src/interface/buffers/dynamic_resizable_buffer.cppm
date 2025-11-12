// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.prosper.opengl:buffer.dynamic_resizable_buffer;

export import :buffer.buffer;

export namespace prosper {
	class PR_EXPORT GLDynamicResizableBuffer : public IDynamicResizableBuffer, virtual public GLBuffer {
	  public:
		GLDynamicResizableBuffer(IPrContext &context, IBuffer &buffer, const util::BufferCreateInfo &createInfo, uint64_t maxTotalSize);
	  protected:
		virtual void MoveInternalBuffer(IBuffer &other) override;
	};
};
