// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.prosper.opengl:buffer.uniform_resizable_buffer;

export import :buffer.buffer;

export namespace prosper {
	class PR_EXPORT GLUniformResizableBuffer : public IUniformResizableBuffer, virtual public GLBuffer {
	  public:
		GLUniformResizableBuffer(IPrContext &context, IBuffer &buffer, uint64_t bufferInstanceSize, uint64_t alignedBufferBaseSize, uint64_t maxTotalSize, uint32_t alignment);
	  protected:
		virtual void MoveInternalBuffer(IBuffer &other) override;
	};
};
