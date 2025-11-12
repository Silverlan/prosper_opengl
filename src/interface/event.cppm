// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.prosper.opengl:event;

export import pragma.prosper;

export namespace prosper {
	class PR_EXPORT GLEvent : public prosper::IEvent {
	  public:
		static std::shared_ptr<IEvent> Create(IPrContext &context);

		virtual ~GLEvent() override;
		virtual bool IsSet() const override;
	  private:
		GLEvent(IPrContext &context);
	};
};
