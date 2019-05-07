#include "nvse/PluginAPI.h"
#include "nvse/GameAPI.h"
#include "nvse/SafeWrite.h"


IDebugLog		gLog("ManualReload.log");

const UInt32 bypassReloadAddr = 0x8A8D63;
const UInt32 reloadAddr = 0x8A8B2D;

/*_declspec(naked) void HookAmmoCall()
{

		_asm
		{

			jnz bypassReload
			push eax
			mov eax, dword ptr [0x11F2258]
			cmp eax, 0x00000004
			pop eax
			jne bypassReload
			jmp reloadAddr
			bypassReload:
			jmp bypassReloadAddr
		}
}
*/

_declspec(naked) void __fastcall HookAmmoCall() {
	static const UInt32 bypassReloadAddr = 0x8A8D63;
	static const UInt32 reloadAddr = 0x8A8B2D;
	_asm {
		/* still have result of cmp in register, so will jump if ammo count not equal to 0 */
		jnz dontReload
			nonPlayerCheck :
		mov eax, dword ptr ds : [0x11DEA3C] // g_thePlayer
			cmp ecx, eax
			jne doReload

			vatsCheck :
		mov eax, dword ptr ds : [0x11F2258] // g_VatsStruct+4
			cmp eax, 4
			je doReload

			dontReload :
		jmp bypassReloadAddr

			doReload :
		jmp reloadAddr
	}
}


void PatchReload()
{
	SafeWriteBuf(0x8A8B27, "\x90\x90\x90\x90\x90\x90", 0x06);
	WriteRelJump(0x8A8B27, (UInt32)HookAmmoCall);
}



extern "C" {

	//I use dllmain in some of my other plugins to get the dll HANDLE, but you can leave this out, as it is already in dllmain.c
	BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
	{
		return TRUE;
	}



	bool NVSEPlugin_Query(const NVSEInterface * nvse, PluginInfo * info)
	{
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "ManualReload";
		info->version = 1;
		if (nvse->isEditor)return false;
		if (nvse->nvseVersion < NVSE_VERSION_INTEGER)
		{
			_ERROR("NVSE version too old (got %08X expected at least %08X)", nvse->nvseVersion, NVSE_VERSION_INTEGER);
			return false;
		}
		if (nvse->isNogore)
		{
			_ERROR("The NoGore version is not supported");
			return false;

		}
		if (nvse->runtimeVersion < RUNTIME_VERSION_1_4_0_525)
		{
			_ERROR("Unsupported NV exe version");
			return false;
		}

		return true;
	}

	bool NVSEPlugin_Load(const NVSEInterface * nvse)
	{
		PatchReload();
		return true;
	}
};



