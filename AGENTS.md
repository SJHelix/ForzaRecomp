# AGENTS.md — ForzaRecomp (FH1 via ReXGlue)

Static recompilation of Forza Horizon 1 (Xbox 360) with ReXGlue.
Local workflow targets Windows (`win-amd64-debug`).

Status (2026-09-07): builds clean under SDK 0.10.0, boots through intro
and plays the video intro (with `--gpu_plugin=xenos`). Crash triage is
ongoing; see "Triage loop" below.

## Never commit game content or build output

`.gitignore` blocks `assets/`, `out/`, `*.exe`, `*.log`, `*.pdb/.ilk/.dmp`.
`assets/` (e.g. `default.xex`, `XMediaFacade_default.xex`,
`SpeechFacade_default.xex`) is copyrighted and local-only. Do not add
extracted game files. `*.log` covers smoke-test logs and
`guest_crash_stack.log` — keep them out of the tree.

## Layout

- `FH1/` — the actual project: `CMakeLists.txt`, `CMakePresets.json`,
  `forza_horizon_1_manifest.toml`, `*_functions.toml`, `src/`, `generated/`.
- `FH1/forza_horizon_1_manifest.toml` — manifest-first codegen config:
  `[entrypoint]` (default.xex) + two `[[modules]]` (facades). Each section
  has `includes = [...]` pointing at a functions file.
- `FH1/forza_horizon_1_functions.toml`,
  `FH1/xmediafacade_default_functions.toml`,
  `FH1/speechfacade_default_functions.toml` — manual `[functions]`
  entries (`0x<addr> = {}`) forcing discovery of code the static analysis
  misses (vtable-only targets, no direct callers). Comment each entry with
  the crash that found it.
- `FH1/src/forza_horizon_1_app.h` — `ReXApp` subclass: asset-path
  resolution, Nop input driver. The game XexLoadImages the facades itself
  on guest threads (DllMain runs); do NOT preload them in `OnPostSetup`
  (host thread → DllMain skipped → facade aborts on first use).
- `FH1/src/crash_stack.h` — header-only native crash-stack dumper
  (VEH on breakpoint + SIGABRT handler + facade export-table dump).
  Writes `guest_crash_stack.log` in CWD. Debug PDBs resolve `sub_<addr>`
  frames to the exact guest call chain. Keep it; it is the triage tool.
- `FH1/src/import_shims.cpp` — hand-owned Xbox import shims
  (`DEFINE_IMPORT_SHIM` no-ops, real shims, and the `XexGetProcedureAddress`
  bypass below). Crash-triage shims go here.
- `FH1/src/main.cpp`, `FH1/CMakeLists.txt`, `FH1/generated/` —
  SDK-managed (`rexglue init`/codegen output). Never hand-edit; the only
  sanctioned knob used is `rexglue_setup_target` args (currently none).
- `include/`, `lib/`, `share/`, `cmake/`, `licenses/` — vendored
  SDK/runtime deps. Treat as read-only.
- `pyghidra_mcp_projects/my_project.gpr` — empty Ghidra project; Ghidra
  has no XEX loader, so static RE currently goes through generated
  recomp code + codegen metadata instead. MCP at
  `http://127.0.0.1:8000/mcp` (requires local server).

## Build

- Toolchain: VS 2026 Community (`D:\Program Files\Microsoft
  Visual Studio\18\Community`), LLVM clang 22, CMake 4.x, Ninja.
- SDK 0.10.0 is installed at `D:\dev\rexglue` (bin dir on PATH).
  Configure from `FH1/`: `cmake --preset win-amd64-debug` (prefix path is
  cached; a fresh configure needs
  `-DCMAKE_PREFIX_PATH=D:/dev/rexglue`), then
  `cmake --build out/build/win-amd64-debug --parallel 12`.
- `make.txt` loads the VS 2026 env and builds `win-amd64-debug`.
- `.vscode/tasks.json` builds the same dir; `.vscode/launch.json`
  launches the exe with CWD at repo root plus `--gpu_plugin=xenos`.

## Codegen

- Manifest-first flow. Codegen runs as the `forza_horizon_1_codegen`
  build target and must VALIDATE cleanly (no `--force` in the
  day-to-day loop).
- Adding a `[functions]` entry (or touching the manifest/XEXs) triggers
  re-analysis (~5 min for default.xex). New unresolved branch targets
  appear as `UnresolvedCall` errors — add them to the matching functions
  file (entrypoint vs module scope matters).
- DLL targets: `rexglue init --project-name forza_horizon_1 --xex-path
  assets/default.xex module --project-root FH1 --xex-path
  assets/<Facade>.xex --guest-path 'game:\<Facade>.xex'`.
  (Parent `init` options come before `module`; `--xex-path` resolves
  from CWD, not the project root.)

## Triage loop

1. Run with logging: `forza_horizon_1.exe
   --log_file=<abs path> --log_level=debug [--gpu_plugin=xenos]`,
   CWD at repo root so `assets/` resolves.
2. `FATAL: Call to invalid or unregistered function at 0x...` → check
   the address against `generated/<mod>/codegen.partition.json`
   (hole between registered functions = missed table entry) →
   add `0x... = {}` to the right functions file → rebuild (codegen
   validates; a bogus address fails validation instead) → retest.
3. Hard crash (0x80000003 / abort): read `guest_crash_stack.log` —
   `sub_<addr>` frames name the guest chain; `exports:` section maps
   facade ordinals. SIGABRT now also dumps instead of dying silent.
4. `XexGetProcedureAddress` MUST stay hooked (see below). `d:\DebugOptions.ini`,
   `cache:\`, `update:\media.zip`, `game:\media\stringtables\en\` VFS
   failures are tolerated by the game so far.

## Runtime specifics (do not regress)

- GPU: `rexglue_setup_target(... GPU_PLUGINS xenos)` is the sanctioned
  staging, but `CMakeLists.txt` is SDK-managed, so the Debug xenos DLL
  is currently copied by hand next to the exe
  (`D:/dev/rexglue/bin/rexgpu-xenosd.dll` → out dir) and enabled with
  `--gpu_plugin=xenos`. Without it the game boots headless (Vd* ignored).
- GPU Debug assert (2026-09-07, SDK 0.10.0): `rexgpu-xenosd.dll`
  asserts at `draw.cpp:885` (`x0<x1 && y0<y1`) on an empty scissor /
  resolve rect the game issues during the intro video (host-side
  `wassert` on a GPU worker thread — `guest_crash_stack.log` shows no
  guest frames, only `rex_gpu_create+...`). Real HW treats this as a
  no-op. Workaround: `win-amd64-release` build — the Release plugin
  logs recoverable `[gpu] Resolve region is empty` +
  `PM4_DRAW_INDX_2 ... Failed in backend` and keeps playing.
  Upstream fix wanted: clamp/early-out instead of assert.
  Do NOT mix configs: Release `rexgpu-xenos.dll` renamed over the
  Debug filename AVs instantly (0xC0000005, empty log); restored.
  Release out dir needs `rexgpu-xenos.dll` + `TracyClient.dll` copied
  by hand (`rexruntime.dll` stages automatically).
- `XexGetProcedureAddress` bypass (`import_shims.cpp`): the runtime's
  per-module thunk pool exhausts mid-boot and every dynamic resolution
  then fails. The hook resolves via `XModule::GetFromHModule` +
  `GetProcAddressByOrdinal` and returns the RAW guest address (guest
  indirect calls dispatch through the registry; no pool thunk needed).
  Values match the runtime's own success results. If the SDK fixes the
  pool upstream, re-test with the hook removed.
- Facade DllMain only runs on guest-thread loads. Host-thread preloads
  poison this (module resident, entry never runs).

## Upstream / SDK issues to report

- Per-module dynamic-thunk pool exhaustion (`Thunk address space
  exhausted for module at 823E0000`): default.xex has 383 static
  imports; pool appears sized ~static count + small slack, so normal
  lazy/dynamic resolutions (XInput/Xam/facade exports) overflow it
  mid-boot. Worked around locally via the bypass hook.
- `LoadUserModule(name, true)` from a host thread silently skips
  DllMain; there is no deferred-attach API.
