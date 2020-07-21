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
bool bFastReloading = false;
int uDisplayFrameThing5_func = 0x0072B370;

int DeStutterCaveExit = 0x731241;
void __declspec(naked) DeStutterCave()
{
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
	injector::WriteMemory<int*>(0x73E746, &ShaderObjects[ShaderIndex], true);
	injector::WriteMemory<int*>(0x73E79F, &ShaderObjects[ShaderIndex], true);

	*(unsigned int*)0x00AB0B25 = 1;

	injector::WriteMemory<int*>(0x73E79F, &ShaderObjects[28], true);
	injector::WriteMemory<int*>(0x73E746, &ShaderObjects[0], true);
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
	injector::MakeJMP(0x00731236, DeStutterCave, true);

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
