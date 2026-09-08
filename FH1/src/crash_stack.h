// forza_horizon_1 - native crash-stack dumper (debugging aid, header-only).
//
// Installs a vectored exception handler that, on a breakpoint exception
// (what guest PPC `trap` lowers to via __builtin_debugtrap), captures the
// native stack and writes it to guest_crash_stack.log in the working
// directory. Generated recomp functions are real C++ functions
// (sub_<addr>), so with Debug PDBs next to the exe the log names the exact
// guest call chain leading to the trap. Returns EXCEPTION_CONTINUE_SEARCH
// so normal crash handling proceeds afterwards.
//
// Raw Win32 file I/O only: the process may hold runtime locks when this
// fires, so no CRT file APIs and no REXLOG here.

#pragma once

#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <csignal>

#include <dbghelp.h>

// Linked via pragma so FH1/CMakeLists.txt (SDK-managed) stays untouched.
#pragma comment(lib, "dbghelp.lib")

#include <rex/system/kernel_state.h>
#include <rex/system/user_module.h>
#include <rex/system/xex_module.h>

namespace forza_crash {

inline void WriteStr(HANDLE file, const char* s) {
  DWORD written = 0;
  WriteFile(file, s, (DWORD)strlen(s), &written, nullptr);
}

// Best-effort dump of a loaded facade's export table (ordinal -> guest
// address via the runtime's own resolver, plus raw table dwords) to help
// map XexGetProcedureAddress failures to codegen [functions] entries.
inline void DumpFacadeExports(HANDLE file) {
  char line[256];
  auto* ks = rex::system::kernel_state();
  if (!ks) {
    WriteStr(file, "exports: no kernel_state\r\n");
    return;
  }
  uint8_t* base = nullptr;
  try {
    base = ks->memory()->virtual_membase();
  } catch (...) {
    WriteStr(file, "exports: no memory base\r\n");
    return;
  }
  static const char* kMods[] = {"xmediafacade_default", "speechfacade_default"};
  for (const char* mod : kMods) {
    auto m = ks->GetModule(mod, true);
    if (!m) {
      _snprintf_s(line, sizeof(line), _TRUNCATE, "exports: module '%s' not loaded\r\n", mod);
      WriteStr(file, line);
      continue;
    }
    const uint32_t tab =
        m.get<rex::runtime::XexModule>()->export_table_address();
    _snprintf_s(line, sizeof(line), _TRUNCATE,
                "exports: module '%s' table=0x%08X\r\n", mod, tab);
    WriteStr(file, line);
    // Resolver results for ordinals 1..64.
    for (uint16_t ord = 1; ord <= 64; ++ord) {
      uint32_t addr = 0;
      try {
        addr = m->GetProcAddressByOrdinal(ord, 0);
      } catch (...) {
        addr = 0;
      }
      if (addr) {
        _snprintf_s(line, sizeof(line), _TRUNCATE, "  ord %u -> 0x%08X\r\n",
                    (unsigned)ord, addr);
        WriteStr(file, line);
      }
    }
    // Raw first 64 dwords of the table for format forensics. No SEH here
    // (host TU may lack async-exceptions); probe readability first.
    if (tab && base &&
        !IsBadReadPtr(reinterpret_cast<const void*>(base + tab), 256)) {
      WriteStr(file, "  raw table dwords:\r\n");
      for (unsigned i = 0; i < 64; ++i) {
        const uint32_t w = __builtin_bswap32(
            *reinterpret_cast<volatile uint32_t*>(base + tab + i * 4));
        _snprintf_s(line, sizeof(line), _TRUNCATE, "    [%02u] 0x%08X\r\n", i,
                    w);
        WriteStr(file, line);
      }
    }
  }
}

inline LONG WINAPI VehHandler(EXCEPTION_POINTERS* info) {
  if (info->ExceptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT) {
    return EXCEPTION_CONTINUE_SEARCH;
  }

  void* frames[64] = {};
  const WORD captured =
      CaptureStackBackTrace(0, 64, frames, nullptr);

  char dir[MAX_PATH] = {};
  DWORD dir_len = GetCurrentDirectoryA(MAX_PATH, dir);

  char path[MAX_PATH + 32] = {};
  if (dir_len > 0 && dir_len < MAX_PATH) {
    _snprintf_s(path, sizeof(path), _TRUNCATE, "%s\\guest_crash_stack.log",
                dir);
  } else {
    _snprintf_s(path, sizeof(path), _TRUNCATE, "guest_crash_stack.log");
  }

  HANDLE file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    return EXCEPTION_CONTINUE_SEARCH;
  }

  auto write_str = [&](const char* s) {
    DWORD written = 0;
    WriteFile(file, s, (DWORD)strlen(s), &written, nullptr);
  };

  char line[512];
  _snprintf_s(line, sizeof(line), _TRUNCATE,
              "guest trap -> EXCEPTION_BREAKPOINT, %u native frames\r\n",
              (unsigned)captured);
  write_str(line);

  HANDLE proc = GetCurrentProcess();
  const BOOL sym_ok = SymInitialize(proc, nullptr, TRUE);
  alignas(SYMBOL_INFO) char sym_buf[sizeof(SYMBOL_INFO) + 256];
  SYMBOL_INFO* sym = reinterpret_cast<SYMBOL_INFO*>(sym_buf);
  sym->SizeOfStruct = sizeof(SYMBOL_INFO);
  sym->MaxNameLen = 255;

  for (WORD i = 0; i < captured; ++i) {
    const DWORD64 addr = (DWORD64)frames[i];
    DWORD64 disp = 0;
    if (sym_ok && SymFromAddr(proc, addr, &disp, sym)) {
      _snprintf_s(line, sizeof(line), _TRUNCATE, "#%u 0x%llX %s+0x%llX\r\n",
                  (unsigned)i, addr, sym->Name, disp);
    } else {
      _snprintf_s(line, sizeof(line), _TRUNCATE, "#%u 0x%llX <no symbol>\r\n",
                  (unsigned)i, addr);
    }
    write_str(line);
  }

  if (sym_ok) {
    SymCleanup(proc);
  }
  DumpFacadeExports(file);
  CloseHandle(file);
  return EXCEPTION_CONTINUE_SEARCH;
}

inline void InstallCrashStackDumper() {
  static bool installed = false;
  if (!installed) {
    installed = true;
    AddVectoredExceptionHandler(1, &VehHandler);
    // REX_FATAL lowers to abort(): capture the native stack (named
    // sub_<addr> frames) without the CRT dialog, then let it die.
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
    std::signal(SIGABRT, [](int) {
      EXCEPTION_POINTERS ep = {};
      EXCEPTION_RECORD rec = {};
      rec.ExceptionCode = EXCEPTION_BREAKPOINT;
      ep.ExceptionRecord = &rec;
      VehHandler(&ep);
      std::_Exit(3);
    });
  }
}

}  // namespace forza_crash
