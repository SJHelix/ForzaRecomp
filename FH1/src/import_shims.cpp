// forza_horizon_1 - unimplemented Xbox import stubs.
//
// Each REX_STUB defines the strong __imp__ symbol the generated
// registrar references; it overrides the weak generated alias.
// Add newly discovered crash-triage stubs here.

#include <rex/hook.h>
#include <rex/system/xmodule.h>

#include <windows.h>

#include <atomic>
#include <cstdint>

REX_STUB(__imp__EtxProducerLog)
REX_STUB(__imp__EtxProducerRegister)
REX_STUB(__imp__EtxProducerUnregister)
REX_STUB(__imp__IoCheckShareAccess)
REX_STUB(__imp__IoCompleteRequest)
REX_STUB(__imp__IoDeleteDevice)
REX_STUB(__imp__IoDismountVolume)
REX_STUB(__imp__IoInvalidDeviceRequest)
REX_STUB(__imp__IoRemoveShareAccess)
REX_STUB(__imp__IoSetShareAccess)
REX_STUB(__imp__KeInitializeMutant)
REX_STUB(__imp__KeReleaseMutant)
REX_STUB(__imp__KeRestoreFloatingPointState)
REX_STUB(__imp__KeSaveFloatingPointState)
REX_STUB(__imp__LDICreateDecompression)
REX_STUB(__imp__LDIDecompress)
REX_STUB(__imp__LDIDestroyDecompression)
REX_STUB(__imp__NetDll_getpeername)
REX_STUB(__imp__NetDll_getsockname)
REX_STUB(__imp__NetDll_getsockopt)
REX_STUB(__imp__NetDll_WSACancelOverlappedIO)
REX_STUB(__imp__NetDll_WSAEventSelect)
REX_STUB(__imp__NetDll_WSAGetOverlappedResult)
REX_STUB(__imp__NetDll_WSARecv)
REX_STUB(__imp__NetDll_WSASend)
REX_STUB(__imp__NetDll_XNetConnect)
REX_STUB(__imp__NetDll_XNetCreateKey)
REX_STUB(__imp__NetDll_XNetGetConnectStatus)
REX_STUB(__imp__NetDll_XNetQosLookup)
REX_STUB(__imp__NetDll_XNetRegisterKey)
REX_STUB(__imp__NetDll_XNetServerToInAddr)
REX_STUB(__imp__NetDll_XNetUnregisterInAddr)
REX_STUB(__imp__NetDll_XNetUnregisterKey)
REX_STUB(__imp__NtCancelIoFile)
REX_STUB(__imp__NtWriteFileGather)
REX_STUB(__imp__ObCreateObject)
REX_STUB(__imp__ObIsTitleObject)
REX_STUB(__imp__ObReferenceObject)
REX_STUB(__imp__PsCamDeviceRequest)
REX_STUB(__imp__RtlUpcaseUnicodeChar)
REX_STUB(__imp__XamCacheCloseFile)
REX_STUB(__imp__XamCacheOpenFile)
REX_STUB(__imp__XamCacheReset)
REX_STUB(__imp__XamGetActiveDashAppInfo)
REX_STUB(__imp__XamInputRawState)
REX_STUB(__imp__XamNuiCameraElevationGetAngle)
REX_STUB(__imp__XamNuiCameraElevationSetAngle)
REX_STUB(__imp__XamNuiCameraElevationStopMovement)
REX_STUB(__imp__XamNuiCameraRememberFloor)
REX_STUB(__imp__XamNuiCameraTiltGetStatus)
REX_STUB(__imp__XamNuiCameraTiltReportStatus)
REX_STUB(__imp__XamNuiCameraTiltSetCallback)
REX_STUB(__imp__XamNuiIdentityGetSessionId)
REX_STUB(__imp__XamProfileCreateEnumerator)
REX_STUB(__imp__XamProfileEnumerate)
REX_STUB(__imp__XamReadBiometricData)
REX_STUB(__imp__XamShowAchievementsUI)
REX_STUB(__imp__XamShowFriendsUI)
REX_STUB(__imp__XamShowGameInviteUI)
REX_STUB(__imp__XamShowGamerCardUIForXUID)
REX_STUB(__imp__XamShowMarketplaceDownloadItemsUI)
REX_STUB(__imp__XamShowMarketplaceUI)
REX_STUB(__imp__XamShowNuiDeviceSelectorUI)
REX_STUB(__imp__XamShowNuiGuideUI)
REX_STUB(__imp__XamShowNuiMessageBoxUI)
REX_STUB(__imp__XamShowNuiSigninUI)
REX_STUB(__imp__XamShowPlayersUI)
REX_STUB(__imp__XamUserGetAgeGroup)
REX_STUB(__imp__XamUserGetIndexFromXUID)
REX_STUB(__imp__XamUserGetMembershipTierFromXUID)
REX_STUB(__imp__XamUserGetOnlineCountryFromXUID)
REX_STUB(__imp__XamUserNuiEnableBiometric)
REX_STUB(__imp__XamUserNuiGetEnrollmentIndex)
REX_STUB(__imp__XamUserNuiGetUserIndex)
REX_STUB(__imp__XamVoiceSubmitPacket)
REX_STUB(__imp__XamWriteBiometricData)
REX_STUB(__imp__XamXlfsInitializeUploadQueue)
REX_STUB(__imp__XamXlfsMountUploadQueueInstance)
REX_STUB(__imp__XamXlfsUninitializeUploadQueue)
REX_STUB(__imp__XamXlfsUnmountUploadQueueInstance)
REX_STUB(__imp__XamXStudioRequest)
REX_STUB(__imp__XAudioGetDuckerAttackTime)
REX_STUB(__imp__XAudioGetDuckerHoldTime)
REX_STUB(__imp__XAudioGetDuckerLevel)
REX_STUB(__imp__XAudioGetDuckerReleaseTime)
REX_STUB(__imp__XAudioGetDuckerThreshold)
REX_STUB(__imp__XCustomGetCurrentGamercard)
REX_STUB(__imp__XCustomGetLastActionPressEx)
REX_STUB(__imp__XCustomSetDynamicActions)
REX_STUB(__imp__XCustomUnregisterDynamicActions)
REX_STUB(__imp__XeCryptSha384Final)
REX_STUB(__imp__XeCryptSha384Init)
REX_STUB(__imp__XeCryptSha384Update)
REX_STUB(__imp__XeCryptSha512Final)
REX_STUB(__imp__XeCryptSha512Init)
REX_STUB(__imp__XeCryptSha512Update)
REX_STUB(__imp__XeKeysGetConsoleID)
REX_STUB(__imp__XNetLogonGetMachineID)
REX_STUB(__imp__XNetLogonGetTitleID)

// Real shim: report STATUS_NOT_SUPPORTED instead of crashing when
// the game queries volume information.
REX_HOOK_RAW(__imp__NtQueryVolumeInformationFile) {  static constexpr uint32_t kStatusNotSupported = 0xC00000BBu;
  const uint32_t io_status_block_ptr = ctx.r4.u32;
  if (io_status_block_ptr) {
    // Guest is big-endian; match REX_STORE_U32 byte order.
    auto store_u32 = [&](uint32_t guest_addr, uint32_t v) {
      *reinterpret_cast<volatile uint32_t*>(base + guest_addr) =
          __builtin_bswap32(v);
    };
    store_u32(io_status_block_ptr + 0, kStatusNotSupported);
    store_u32(io_status_block_ptr + 4, 0);
  }
  static std::atomic<uint32_t> s_counter{0};
  const uint32_t call_num = ++s_counter;
  if (call_num <= 5) {
    REXKRNL_WARN("__imp__NtQueryVolumeInformationFile [#{}] returning STATUS_NOT_SUPPORTED", call_num);
  }
  ctx.r3.u64 = kStatusNotSupported;
}

// XEX export bypass (2026-09-07): the runtime's per-module thunk pool
// exhausts mid-boot ("Thunk address space exhausted"), after which every
// XexGetProcedureAddress fails (observed: ordinal 6 of XMediaFacade during
// media init). This hook resolves exports to RAW guest addresses instead:
// direct/indirect guest calls dispatch through the function registry and
// need no pool thunk. HMODULE is a guest X_LDR_DATA_TABLE_ENTRY*; the
// runtime maps it via XModule::GetFromHModule.
REX_HOOK_RAW(__imp__XexGetProcedureAddress) {
  auto* ks = rex::system::kernel_state();
  const uint32_t h = ctx.r3.u32;
  const uint32_t ordinal = ctx.r4.u32;
  const uint32_t out_ptr = ctx.r5.u32;
  static constexpr uint32_t kStatusProcedureNotFound = 0xC000007Au;
  uint32_t addr = 0;
  if (ks && h && out_ptr) {
    auto* mem = ks->memory();
    uint8_t* ldr_host = mem->TranslateVirtual<uint8_t*>(h);
    // HMODULE is a guest LDR entry; never fault the host on it.
    if (!IsBadReadPtr(ldr_host, 0x64)) {
      auto mod = rex::system::XModule::GetFromHModule(ks, ldr_host);
      if (mod) {
        addr = mod->GetProcAddressByOrdinal(static_cast<uint16_t>(ordinal), 0);
      }
    }
    if (addr) {
      uint8_t* out_host = mem->TranslateVirtual<uint8_t*>(out_ptr);
      if (!IsBadWritePtr(out_host, 4)) {
        *reinterpret_cast<volatile uint32_t*>(out_host) =
            __builtin_bswap32(addr);
        ctx.r3.u64 = 0;
        static std::atomic<uint32_t> s_ok{0};
        if (++s_ok <= 30) {
          REXKRNL_WARN("__imp__XexGetProcedureAddress(handle={:#x} ordinal={}) -> {:#x} (raw)",
                       h, ordinal, addr);
        }
        return;
      }
    }
  }
  static std::atomic<uint32_t> s_fail{0};
  if (++s_fail <= 10) {
    REXKRNL_WARN("__imp__XexGetProcedureAddress(handle={:#x} ordinal={}) -> NOT FOUND",
                 h, ordinal);
  }
  ctx.r3.u64 = kStatusProcedureNotFound;
}
