// NFSC Dynamic shader reload/update code

#include "stdafx.h"
#include "stdio.h"
#include <windows.h>
#include "includes\injector\injector.hpp"
#include "includes\IniReader.h"
#define NFSC_SHADEROBJ_COUNT 29

enum ShaderTypes
{
	WorldShader = 0,
	WorldReflectShader,
	WorldBoneShader,
	WorldNormalMap,
	CarShader,
	CARNORMALMAP,
	WorldMinShader,
	FEShader,
	FEMaskShader,
	FilterShader,
	ScreenFilterShader,
	RainDropShader,
	VisualTreatmentShader,
	WorldPrelitShader,
	ParticlesShader,
	skyshader,
	shadow_map_mesh,
	CarShadowMapShader,
	WorldDepthShader,
	shadow_map_mesh_depth,
	NormalMapNoFog,
	InstanceMesh,
	ScreenEffectShader,
	HDRShader,
	UCAP,
	GLASS_REFLECT,
	WATER,
	RVMPIP,
	GHOSTCAR
};

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
bool UpdateFlags[NFSC_SHADEROBJ_COUNT];
void(__stdcall*UnloadShaders_func)() = (void(__stdcall*)())0x0073E700;
void(__stdcall*UpdateShaders_func)() = (void(__stdcall*)())0x0073E740;
void(__thiscall*sub_72B660)(unsigned int dis) = (void(__thiscall*)(unsigned int))0x72B660;

typedef struct SomeDXStruct
{
	int unk1;
	int unk2;
	int unk3;
};

void __stdcall UnloadShaders_Custom()
{
	if (!bFastReloading)
		return UnloadShaders_func();

	int v1;

	int(__stdcall*DXFunc1)(unsigned int var1);
	int(__stdcall*DXFunc2)(unsigned int var1);

	for (int i = 0; i < NFSC_SHADEROBJ_COUNT; i++)
	{
		if (UpdateFlags[i])
		{
			v1 = *(int*)(int)&ShaderObjects[i];
			if (*(int*)(v1 + 0x44))
			{
				DXFunc1 = (int(__stdcall*)(unsigned int))*(int*)(*(int*)*(int*)(v1 + 0x44) + 0x114);
				DXFunc1(*(int*)(v1 + 0x44));

				DXFunc2 = (int(__stdcall*)(unsigned int))*(int*)(*(int*)*(int*)(v1 + 0x44) + 8);
				DXFunc2(*(int*)(v1 + 0x44));

				*(int*)(v1 + 0x44) = 0;
			}
		}
	}
}

void __stdcall UpdateShaders_Custom()
{
	if (!bFastReloading)
		return UpdateShaders_func();

	int v1;
	int v2;
	SomeDXStruct DXStruct;

	int thing3;

	int(__stdcall*DXFunc1)(unsigned int var1, unsigned int var2);
	int(__stdcall*DXFunc2)(unsigned int var1, unsigned int var2);
	int(__stdcall*DXFunc3)(unsigned int var1, unsigned int var2, unsigned int var3);

	for (int i = 0; i < NFSC_SHADEROBJ_COUNT; i++)
	{
		if (UpdateFlags[i])
		{
			v1 = *(int*)(int)&ShaderObjects[i];

			if (!*(int*)(v1 + 0x44))
			{
				sub_72B660((int)v1);
			}

			if (*(int*)(v1 + 16))
			{
				thing3 = *(int*)*(int*)(v1 + 0x44);
				DXFunc1 = (int(__stdcall*)(unsigned int, unsigned int))*(int*)(thing3 + 0x30);
				v2 = DXFunc1(*(int*)(v1 + 0x44), *(int*)(v1 + 0xC));
				*(int*)(v1 + 16) = v2;

				DXFunc2 = (int(__stdcall*)(unsigned int, unsigned int))*(int*)(thing3 + 0xE8);
				DXFunc2(*(int*)(v1 + 0x44), v2);
			}
			thing3 = *(int*)*(int*)(v1 + 0x44);
			DXFunc3 = (int(__stdcall*)(unsigned int, unsigned int, unsigned int))*(int*)(thing3 + 0x14);
			DXFunc3(*(int*)(v1 + 0x44), *(int*)(v1 + 16), (unsigned int)&DXStruct);
			*(int*)(v1 + 20) = DXStruct.unk2;
			UpdateFlags[i] = false;
		}
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
	UpdateFlags[ShaderIndex] = true;
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
	injector::MakeCALL(0x007311FD, UpdateShaders_Custom, true);
	injector::MakeCALL(0x007311E6, UnloadShaders_Custom, true);

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
