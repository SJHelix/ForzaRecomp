// forza_horizon_1 - ReXGlue Recompiled Project

#include "generated/default/forza_horizon_1_init.h"

#include "forza_horizon_1_app.h"

namespace {
const bool kDisableSdlWgi = []() {
  _putenv_s("SDL_JOYSTICK_WGI", "0");
  return true;
}();
}

REX_DEFINE_APP(forza_horizon_1, ForzaHorizon1App::Create)
