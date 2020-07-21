// NFSC Dynamic shader reload/update code

#include "stdafx.h"
#include "stdio.h"
#include <windows.h>
#include "includes\injector\injector.hpp"
#include "includes\IniReader.h"
#define NFSC_SHADEROBJ_COUNT 29

char ShaderNames[NFSC_SHADEROBJ_COUNT][22] = {
	"WorldShader",
	"WorldReflectShader",
	"WorldBoneShader",
	"WorldNormalMap",
	"CarShader",
	"CARNORMALMAP",
	"WorldMinShader",
	"FEShader",
	"FEMaskShader",
	"FilterShader",
	"ScreenFilterShader",
	"RainDropShader",
	"VisualTreatmentShader",
	"WorldPrelitShader",
	"ParticlesShader",
	"skyshader",
	"shadow_map_mesh",
	"CarShadowMapShader",
	"WorldDepthShader",
	"shadow_map_mesh_depth",
	"NormalMapNoFog",
	"InstanceMesh",
	"ScreenEffectShader",
	"HDRShader",
	"UCAP",
	"GLASS_REFLECT",
	"WATER",
	"RVMPIP",
	"GHOSTCAR"
};

int hkReload = 0x39;
int* ShaderObjects = (int*)0x00B1F660;
int ShaderToReload = 0;
int ShaderToReloadPointerInt = 0;
bool bFastReloading = false;
int uDisplayFrameThing4_func = 0x00710100;
int uDisplayFrameThing5_func = 0x0072B370;
int UpdateShaders_func = 0x0073E740;
int UnloadShaders_func = 0x0073E700;

int sub_72B660 = 0x72B660;
void __declspec(naked) UpdateSingleShaderFunc()
{
	if (!bFastReloading)
		_asm jmp UpdateShaders_func
	_asm
	{
		sub     esp, 0xC
		push    esi
		push    edi
		mov     edi, ShaderToReloadPointerInt
		lea     ebx, [ebx + 0]

	loc_73E750:
		mov     esi, [edi]
		mov     eax, [esi + 0x44]
		test    eax, eax
		jnz     loc_73E760
		mov     ecx, esi
		call    sub_72B660

	loc_73E760 :
		mov     eax, [esi + 0x10]
		test    eax, eax
		jz      loc_73E784
		mov     edx, [esi + 0xC]
		mov     eax, [esi + 0x44]
		mov     ecx, [eax]
		push    edx
		push    eax
		call    dword ptr[ecx + 0x30]
		mov     ecx, [esi + 0x44]
		push    eax
		mov[esi + 0x10], eax
		mov     edx, [ecx]
		push    ecx
		call    dword ptr[edx + 0x0E8]

	loc_73E784:
		mov     eax, [esi + 0x44]
		mov     ecx, [eax]
		lea     edx, [esp + 0x14 - 0xC]
		push    edx
		mov     edx, [esi + 0x10]
		push    edx
		push    eax
		call    dword ptr[ecx + 0x14]
		mov     eax, [esp + 0x14 - 0x8]
		add     edi, 4
		//cmp     edi, ShaderToReloadPointerInt
		mov[esi + 0x14], eax
		//jle     loc_73E750
		pop     edi
		pop     esi
		add     esp, 0xC
		retn
	}
}

void __declspec(naked) UnloadSingleShaderFunc()
{
	if (!bFastReloading)
		_asm jmp UnloadShaders_func
	_asm
	{
		push    esi
		push    edi
		mov     edi, ShaderToReloadPointerInt
		
	loc_73E707:
		mov     esi, [edi]
		mov     eax, [esi+0x44]
		test    eax, eax
		jz      loc_73E729
		mov     ecx, [eax]
		push    eax
		call    dword ptr [ecx+0x114]
		mov     eax, [esi+0x44]
		mov     edx, [eax]
		push    eax
		call    dword ptr [edx+8]
		mov     dword ptr [esi+0x44], 0
		
	loc_73E729:
		//add     edi, 4
		//cmp     edi, ShaderToReloadPointerIntEnd2Def
		//jl      loc_73E707
		pop     edi
		pop     esi
		retn
	}
}

int DeStutterCaveExit = 0x731241;
void __declspec(naked) DeStutterCave()
{
	if (!bFastReloading)
		_asm call uDisplayFrameThing4_func
	_asm add esp, 0xC
	if (!bFastReloading)
		_asm call uDisplayFrameThing5_func
	*(int*)0x00AB0B25 = 0;
	bFastReloading = false;
	_asm jmp DeStutterCaveExit
}

int TriggerReload(int ShaderIndex)
{
	printf("Reloading [%s] @ %X\n", ShaderNames[ShaderIndex], &ShaderObjects[ShaderIndex]);
	bFastReloading = true;
	ShaderToReloadPointerInt = (int)&ShaderObjects[ShaderIndex];
	*(unsigned int*)0x00AB0B25 = 1;
	return 0;
}

DWORD WINAPI HotkeyThread()
{
	CIniReader inireader("");
	while (1)
	{
		if ((GetAsyncKeyState(hkReload) & 1))
		{
			ShaderToReload = inireader.ReadInteger("ReloadShader", "Index", 0);
			ShaderToReload %= NFSC_SHADEROBJ_COUNT;
			TriggerReload(ShaderToReload);
			while ((GetAsyncKeyState(hkReload) & 0x8000) > 0) { Sleep(0); }
		}
	}
	return 0;
}

int InitThread()
{
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&HotkeyThread, NULL, 0, NULL);
	return 0;
}

int Init()
{
	InitThread();
	injector::MakeJMP(0x0073122E, DeStutterCave, true);
	injector::MakeCALL(0x007311FD, UpdateSingleShaderFunc, true);
	injector::MakeCALL(0x007311E6, UnloadSingleShaderFunc, true);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		freopen("CON", "w", stdout);
		freopen("CON", "w", stderr);
		Init();
	}
	return TRUE;
}
