// forza_horizon_1 - ReXGlue Recompiled Project
//
// Customize the app by overriding virtual hooks from rex::ReXApp.

#pragma once

#include <cstdlib>
#include <filesystem>

#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/input/device_assignment.h>
#include <rex/input/input_system.h>
#include <rex/input/mnk/mnk_input_driver.h>
#include <rex/input/nop/nop_input_driver.h>
#include <rex/input/sdl/sdl_input_driver.h>
#include <rex/input/xinput/xinput_input_driver.h>
#include <rex/logging.h>
#include <rex/rex_app.h>
#include <rex/system/kernel_state.h>
#include <rex/system/user_module.h>

#include "crash_stack.h"

class ForzaHorizon1App : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<ForzaHorizon1App>(new ForzaHorizon1App(ctx, "forza_horizon_1",
        PPCImageConfig));
  }

 protected:
  void OnConfigurePaths(rex::PathConfig& paths) override {
    const auto exe_folder = rex::filesystem::GetExecutableFolder();
    const auto candidate_from_exe = (exe_folder / ".." / ".." / ".." / "assets").lexically_normal();
    const auto candidate_from_cwd_fh1 =
        (std::filesystem::current_path() / "FH1" / "assets").lexically_normal();
    const auto candidate_from_cwd =
        (std::filesystem::current_path() / "assets").lexically_normal();

    if (std::filesystem::exists(candidate_from_exe)) {
      paths.game_data_root = candidate_from_exe;
    } else if (std::filesystem::exists(candidate_from_cwd_fh1)) {
      paths.game_data_root = candidate_from_cwd_fh1;
    } else if (std::filesystem::exists(candidate_from_cwd)) {
      paths.game_data_root = candidate_from_cwd;
    }
  }

  void OnPreSetup(rex::RuntimeConfig& config) override {
    forza_crash::InstallCrashStackDumper();
    // Input (2026-09-08): the rexglue-init template forced a Nop-only driver,
    // so the guest always saw "no controller connected" and sat at the
    // title/press-start screen. Wire up the real drivers instead:
    // SDL (gamepads) + XInput (native Xbox pads) + MnK (keyboard/mouse
    // emulating pad 0) + Nop fallback for tool mode / no devices.
    // SharedAssignment merges every physical device into guest user 0, which
    // is what this single-player title polls; MergeInto ORs buttons, takes
    // max triggers / larger stick deflection, and neutral pads never steal
    // the slot. Deterministic on purpose: --input_backend is bypassed.
    // Keyboard needs mnk_mode; default it on unless the user already chose
    // (config file, REX_ env, or --mnk_mode CLI flag wins).
    if (rex::cvar::GetFlagSource("mnk_mode") ==
        rex::cvar::Source::kDefault) {
      rex::cvar::SetFlagByName("mnk_mode", "true");
    }
    config.input_factory = REX_INPUT_BACKEND([](bool tool_mode) {
      auto input_system =
          std::make_unique<rex::input::InputSystem>(nullptr);
      if (tool_mode) {
        input_system->AddDriver(
            std::make_unique<rex::input::nop::NopInputDriver>(nullptr, 0));
        return input_system;
      }
      input_system->AddDriver(
          std::make_unique<rex::input::sdl::SDLInputDriver>(nullptr, 0));
      input_system->AddDriver(
          std::make_unique<rex::input::xinput::XinputInputDriver>(
              nullptr, 0));
      input_system->AddDriver(
          std::make_unique<rex::input::mnk::MnkInputDriver>(nullptr, 0));
      input_system->AddDriver(
          std::make_unique<rex::input::nop::NopInputDriver>(nullptr, 0));
      input_system->SetDeviceAssignment(
          std::make_unique<rex::input::SharedAssignment>());
      return input_system;
    });
  }

  void OnPostSetup() override {
    // NOTE (2026-09-07): preloading the facades here runs on a HOST thread,
    // so the runtime skips their DllMain ("not on a guest thread") and the
    // modules stay uninitialized -> XMediaFacade KeBugChecks on first media
    // use (null context, see guest_crash_stack.log sub_88052E38). The game
    // is expected to XexLoadImage these itself on a guest thread, which
    // runs DllMain properly, so do NOT preload anymore. If the game turns
    // out to never load them (FATAL on unregistered 0x88.../0x89...), the
    // fallback is: preload(false) + LoadUserModule(name, true) from an
    // early guest-thread import hook.
    (void)0;
  }
};
