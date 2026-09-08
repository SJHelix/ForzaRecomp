// forza_horizon_1 - ReXGlue Recompiled Project
//
// Customize the app by overriding virtual hooks from rex::ReXApp.

#pragma once

#include <cstdlib>
#include <filesystem>

#include <rex/filesystem.h>
#include <rex/input/input_system.h>
#include <rex/input/nop/nop_input_driver.h>
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
    config.input_factory = REX_INPUT_BACKEND([](bool) {
      auto input_system = std::make_unique<rex::input::InputSystem>(nullptr);
      input_system->AddDriver(std::make_unique<rex::input::nop::NopInputDriver>(nullptr, 0));
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
