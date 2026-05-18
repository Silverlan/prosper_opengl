// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.prosper.opengl:buffer.resizable_buffer;

export import :buffer.buffer;

export namespace prosper {
	class PR_EXPORT GLResizableBuffer : public IResizableBuffer, virtual public GLBuffer {
	  public:
		GLResizableBuffer(IBuffer &buffer);
	  protected:
		virtual void MoveInternalBuffer(IBuffer &other) override;
	};
};
