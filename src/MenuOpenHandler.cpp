#include "MenuOpenHandler.h"
#include "Offsets.h"
#include "UIFuncs.h"
#include "Settings.h"

namespace Gotobed
{
	namespace Hooks
	{
		stl::HookData ProcessButton{&MenuOpenHandler::ProcessButtonHook};
		stl::HookData CanProcess{&MenuOpenHandler::CanProcessHook};
	}

	namespace details
	{
		bool IsKeyPressed(std::uint32_t a_key) {
			return RE::BSInputDeviceManager::GetSingleton()->GetKeyboard()->IsPressed(a_key);
		}

		bool IsSleepKey(RE::ButtonEvent* a_event) {
			auto& settings = Settings::Get();
			return a_event && (settings.keys.sleep != -1 ? a_event->idCode == settings.keys.sleep : a_event->userEvent == "Wait");
		}

		bool IsSleepModPressed() {
			auto& settings = Settings::Get();
			return settings.keys.sleepMod == -1 || IsKeyPressed(settings.keys.sleepMod);
		}

		bool IsServeTimeKey(RE::ButtonEvent* a_event) {
			auto& settings = Settings::Get();
			return a_event && (settings.keys.serveTime != -1 ? a_event->idCode == settings.keys.serveTime : a_event->userEvent == "Wait");
		}

		bool IsServeTimeModPressed() {
			auto& settings = Settings::Get();
			return settings.keys.serveTimeMod == -1 || IsKeyPressed(settings.keys.serveTimeMod);
		}

	}

	bool MenuOpenHandler::CanProcessHook(RE::InputEvent* a_event) {
		if (a_event && a_event->eventType == RE::INPUT_EVENT_TYPE::kButton) {
			if (details::IsSleepKey(a_event->AsButtonEvent()) || details::IsServeTimeKey(a_event->AsButtonEvent())) {
				return true;
			}
		}

		return Hooks::CanProcess.call_orig(this, a_event);
	}

	bool MenuOpenHandler::ProcessButtonHook(RE::ButtonEvent* a_event) {
		// REL::Relocation<std::uint8_t*> unk_2FE95F8{Offsets::unk_2FE95F8};

		auto& settings = Settings::Get();

		if (a_event && a_event->IsDown()) {
			if (details::IsServeTimeKey(a_event) && details::IsServeTimeModPressed() && OnServeTimeButtonDown()) {
				return true;
			}

			if (details::IsSleepKey(a_event) && details::IsSleepModPressed() && OnSleepButtonDown()) {
				return true;
			}
		}

		return Hooks::ProcessButton.call_orig(this, a_event);
	}

	bool MenuOpenHandler::OnSleepButtonDown() {
		auto ui = RE::UI::GetSingleton();

		if (ui->numPausesGame || ui->IsMenuOpen(RE::FaderMenu::MENU_NAME)) {
			return false;
		}

		auto player = RE::PlayerCharacter::GetSingleton();

		if (player->actorState1.lifeState == RE::ACTOR_LIFE_STATE::kDying || player->actorState1.lifeState == RE::ACTOR_LIFE_STATE::kDead) {
			return false;
		}

		if (player->actorState1.sitSleepState != RE::SIT_SLEEP_STATE::kIsSleeping) {
			return false;
		}

		UIFuncs::ShowSleepWaitMenu(true);

		return true;
	}

	bool MenuOpenHandler::OnServeTimeButtonDown() {
		auto ui = RE::UI::GetSingleton();

		if (ui->numPausesGame || ui->IsMenuOpen(RE::FaderMenu::MENU_NAME)) {
			return false;
		}

		auto player = RE::PlayerCharacter::GetSingleton();

		if (player->actorState1.lifeState == RE::ACTOR_LIFE_STATE::kDying || player->actorState1.lifeState == RE::ACTOR_LIFE_STATE::kDead) {
			return false;
		}

		if (player->jailSentence <= 0 || (player->unkBD8 & 0x40) != 0) {
			return false;
		}

		UIFuncs::ShowServeSentenceQuestion();

		return true;
	}

	void MenuOpenHandler::InstallHooks() {
		Hooks::CanProcess.write_detour(Offsets::MenuOpenHandler::CanProcess.address());
		Hooks::ProcessButton.write_detour(Offsets::MenuOpenHandler::ProcessButton.address());
	}
} 