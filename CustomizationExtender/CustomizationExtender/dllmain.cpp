// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "injector/injector.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "ini/IniReader.h"
#include <stdlib.h>
#include "CarData.h"

using namespace std;
namespace fs = filesystem;

#define SAVE_REGS __asm\
{\
	__asm push eax\
	__asm push ebx\
	__asm push ecx\
	__asm push edx\
	__asm push edi\
	__asm push esi\
}\

#define RESTORE_REGS __asm\
{\
	__asm pop esi\
	__asm pop edi\
	__asm pop edx\
	__asm pop ecx\
	__asm pop ebx\
	__asm pop eax\
}\

vector<CarData*> carList;
bool forceLodA = false;
bool fixExhaustFx = false;

auto GetCarTypeName = (char* (__cdecl*)(int))0x007B0290;
auto FeCustomizeParts_AddMenuOption = (int(__thiscall*)(DWORD*, DWORD, int, bool, BYTE))0x85FE30;
auto StringHashModel = (int(__cdecl*)(char* a1, unsigned int a2))0x471080;
auto StringHash = (int(__cdecl*)(char* a1))0x471050;
auto bSNPrintf = (int(__cdecl*)(char* buffer, int size, const char* str, ...))0x475C30;
auto GetPart = (void* (__thiscall*)(int* carNum, int partNum))0x7B06F0;
auto GetPartNameHash = (int(__thiscall*)(void* _this))0x7CD930;
auto InstallPart = (int(__cdecl*)(void* _this, int* carId, int a3, int partNum, int a5, const char* str, ...))0x84F040;
auto IsNotAutosculpt = (bool(__thiscall*)(void* _this))0x7CA040;
int* GameState = (int*)0xA99BBC;

int GetBit(int n, int k)
{
	return (n & (1 << k)) >> k;
}

void AddDefaultCars()
{
	CarData* car_240SX = new CarData();
	car_240SX->Name = (char*)"240SX";
	car_240SX->PopUpHeadLights = 1;

	CarData* car_RX7 = new CarData();
	car_RX7->Name = (char*)"RX7";
	car_RX7->PopUpHeadLights = 1;

	CarData* car_MR2 = new CarData();
	car_MR2->Name = (char*)"MR2";
	car_MR2->PopUpHeadLights = 1;

	CarData* car_CAMARO = new CarData();
	car_CAMARO->Name = (char*)"CAMARO";
	car_CAMARO->PopUpHeadLights = 1;

	CarData* car_COROLLA = new CarData();
	car_COROLLA->Name = (char*)"COROLLA";
	car_COROLLA->PopUpHeadLights = 1;

	CarData* car_CHARGER69 = new CarData();
	car_CHARGER69->Name = (char*)"CHARGER69";
	car_CHARGER69->PopUpHeadLights = 1;

	CarData* car_BRERA = new CarData();
	car_BRERA->Name = (char*)"BRERA";
	car_BRERA->Roof = 1;

	carList.push_back(car_240SX);
	carList.push_back(car_RX7);
	carList.push_back(car_MR2);
	carList.push_back(car_CAMARO);
	carList.push_back(car_COROLLA);
	carList.push_back(car_CHARGER69);
	carList.push_back(car_BRERA);
}

CarData* CheckIfExists(char* name)
{
	for (CarData* i : carList)
	{
		if (strcmp(name, i->Name) == 0)
		{
			return i;
		}
	}

	return NULL;
}

void LoadCarData()
{
	std::string path = "ExtendedCustomiationCars";
	fs::directory_iterator iterator = fs::directory_iterator(path);
	for (const auto& entry : iterator)
	{
		auto path = entry.path();
		auto carNameStr = path.filename().string();
		auto carName = carNameStr.c_str();

		CIniReader iniReader(path.string().c_str());

		CarData* carData = new CarData();
		carData->PopUpHeadLights = iniReader.ReadInteger((char*)"GENERAL", (char*)"PopUpHeadLights", 0);
		carData->Roof = iniReader.ReadInteger((char*)"GENERAL", (char*)"Roof", 0);
		carData->ChopTop = iniReader.ReadInteger((char*)"GENERAL", (char*)"ChopTop", 0);
		carData->IneriorHI = iniReader.ReadInteger((char*)"GENERAL", (char*)"IneriorHI", 0);
		carData->ForceLodA = iniReader.ReadInteger((char*)"GENERAL", (char*)"ForceLodA", 0);
		//carData->CustomAftermarketSpoilers = iniReader.ReadInteger((char*)"CUSTOM_PARTS", (char*)"AftermarketSpoilers", 0);
		//carData->CustomAutosculptSpoilers = iniReader.ReadInteger((char*)"CUSTOM_PARTS", (char*)"AutosculptSpoilers", 0);
		carData->FrontBadging = iniReader.ReadInteger((char*)"GENERAL", (char*)"FrontBadging", 0);
		carData->RearBadging = iniReader.ReadInteger((char*)"GENERAL", (char*)"RearBadging", 0);

		int size = carNameStr.length();
		carData->Name = new char[size + 1];
		strcpy_s(carData->Name, size + 1, carName);

		CarData* exising = CheckIfExists(carData->Name);

		if (exising != NULL)
		{
			*exising = *carData;
			delete carData;
		}
		else
		{
			carList.push_back(carData);
		}
	}
}

int __stdcall CheckCarData(IniOption type, int carId)
{
	char* carName = GetCarTypeName(carId);

	for (CarData* i : carList)
	{
		if (strcmp(carName, i->Name) != 0)
		{
			continue;
		}

		if (type == IniOption::_PopUpHeadLights)
		{
			return i->PopUpHeadLights;
		}

		if (type == IniOption::_Roof)
		{
			return i->Roof;
		}

		if (type == IniOption::_ChopTop)
		{
			return i->ChopTop;
		}

		if (type == IniOption::_IneriorHI)
		{
			return i->IneriorHI;
		}

		//if (type == 5)
		//{
		//	return i->CustomAftermarketSpoilers;
		//}

		//if (type == 6)
		//{
		//	return i->CustomAutosculptSpoilers;
		//}

		if (type == IniOption::_ForceLodA)
		{
			return i->ForceLodA;
		}

		if (type == IniOption::_FrontBadging)
		{
			return i->FrontBadging;
		}

		if (type == IniOption::_RearBadging)
		{
			return i->RearBadging;
		}
	}

	return 0;
}

void SetHeadlightsOn(int* carId, void* _this, int edi, char* carName)
{
	InstallPart(_this, carId, edi, 0x20, 1, "%s_LEFT_HEADLIGHT", carName);
	InstallPart(_this, carId, edi, 0x2A, 1, "%s_RIGHT_HEADLIGHT", carName);
	InstallPart(_this, carId, edi, 0x2B, 1, "%s_RIGHT_HEADLIGHT_GLASS", carName);
	InstallPart(_this, carId, edi, 0x21, 1, "%s_LEFT_HEADLIGHT_GLASS", carName);
}

void SetHeadlightsOff(int* carId, void* _this, int edi, char* carName)
{
	InstallPart(_this, carId, edi, 0x2A, 1, "%s_RIGHT_HEADLIGHT_OFF", carName);
	InstallPart(_this, carId, edi, 0x2B, 1, "%s_RIGHT_HEADLIGHT_GLASS_OFF", carName);
	InstallPart(_this, carId, edi, 0x20, 1, "%s_LEFT_HEADLIGHT_OFF", carName);
	InstallPart(_this, carId, edi, 0x21, 1, "%s_LEFT_HEADLIGHT_GLASS_OFF", carName);
}

bool __stdcall SetHeadlights(int* carId, void* _this, int edi)
{
	int res = CheckCarData(IniOption::_PopUpHeadLights, *carId);
	char* carName = GetCarTypeName(*carId);
	if (res == 2)
	{
		void* partPtr = GetPart(carId, 0x20);
		if (partPtr != NULL)
		{
			int hash = GetPartNameHash(partPtr);
			char buffer[128];

			bSNPrintf(buffer, 128, "%s_LEFT_HEADLIGHT", carName);
			if (hash == StringHash(buffer))
			{
				SetHeadlightsOn(carId, _this, edi, carName);
			}

			bSNPrintf(buffer, 128, "%s_LEFT_HEADLIGHT_OFF", carName);
			if (hash == StringHash(buffer))
			{
				SetHeadlightsOff(carId, _this, edi, carName);
			}
		}

		return 0;
	}
	else if (res == 0)
	{
		SetHeadlightsOn(carId, _this, edi, carName);
	}
	else
	{
		return res;
	}
}

DWORD PopUpHeadlights1 = 0x00859836;
DWORD PopUpHeadlights2 = 0x008598ED;
void __declspec(naked) PopUpHeadlightsCave()
{
	// Some original code
	__asm
	{
		push ebx
		push edi
		mov edi, [esp + 0x0000021C]
	}

	// Save registers as function call in not expected
	SAVE_REGS;

	__asm
	{
		// Make call
		push edi;
		push ebp;
		push esi;
		call SetHeadlights;
		cmp eax, 1
	}

	RESTORE_REGS;

	__asm
	{
		je resultTrue
		jmp PopUpHeadlights2

		resultTrue :
		jmp PopUpHeadlights1
	}
}

DWORD PopupHeadlightsOn1 = 0x008652C2;
DWORD PopupHeadlightsOn2 = 0x00865455;
void __declspec(naked) PopupHeadlightsOnCave()
{
	SAVE_REGS;

	__asm
	{
		push eax
		push _PopUpHeadLights
		call CheckCarData
		cmp eax, 2
	}

	RESTORE_REGS;

	__asm
	{
		jl resultTrue
		jmp PopupHeadlightsOn2

		resultTrue :
		jmp PopupHeadlightsOn1
	}
}

DWORD EnableChopTopMenuItem1 = 0x00866186;
DWORD EnableChopTopMenuItem2 = 0x00866235;
int* CurrentCarPtr = (int*)0x00B74320;
void __declspec(naked) EnableChopTopMenuItemCave()
{
	SAVE_REGS;

	__asm
	{
		mov eax, CurrentCarPtr;
		mov eax, [eax];
		add eax, 0xBF0;
		push[eax];
		push _ChopTop
			call CheckCarData;

		cmp eax, 1;
		je enableChopTop;

		cmp eax, 2;
		je disableChopTop;

		jmp defaultChopTop;

	enableChopTop:
		RESTORE_REGS;
		jmp EnableChopTopMenuItem1;

	disableChopTop:
		RESTORE_REGS;
		jmp EnableChopTopMenuItem2;

	defaultChopTop:
		RESTORE_REGS;
		cmp dword ptr[eax], 02;
		je enableChopTop1;
		jmp EnableChopTopMenuItem2;
	enableChopTop1:;
		jmp EnableChopTopMenuItem1;
	}
}

DWORD HandleRoof1 = 0x008598F2;
DWORD HandleRoof2 = 0x00859947;
void __declspec(naked) HandleRoofCave()
{
	SAVE_REGS;

	__asm
	{
		push[esi]
		push _Roof
		call CheckCarData
		cmp eax, 1
	}

	RESTORE_REGS;

	__asm
	{
		je resultTrue
		jmp HandleRoof2

		resultTrue :
		jmp HandleRoof1
	}
}

const char* CarSunRoofStr = "%s_ROOF_SUN";
DWORD FixRoofCarName1 = 0x00859931;
void __declspec(naked) FixRoofCarName1Cave()
{
	__asm
	{
		push[esi]
		call GetCarTypeName

		push eax
		push CarSunRoofStr

		jmp FixRoofCarName1
	}
}

const char* CarRoofStr = "%s_ROOF";
DWORD FixRoofCarName2 = 0x00859938;
void __declspec(naked) FixRoofCarName2Cave()
{
	__asm
	{
		push[esi]
		call GetCarTypeName

		push eax
		push CarRoofStr

		jmp FixRoofCarName2
	}
}

DWORD AftermarketTuning1 = 0x00866215;
void __declspec(naked) AftermarketTuningCave()
{
	_asm
	{
		call FeCustomizeParts_AddMenuOption;

		SAVE_REGS;
		mov eax, CurrentCarPtr;
		mov eax, [eax];
		add eax, 0xBF0;
		push[eax];
		push _IneriorHI;
		call CheckCarData;
		cmp eax, 1;
		RESTORE_REGS;
		jne skipInerior;

		xor edx, edx;
		mov dl, [esi + 0x000002D4];
		push 00;
		mov ecx, esi;
		push edx;
		push 0x1D; // Interior
		push 0x3691263B; // item name
		call FeCustomizeParts_AddMenuOption;

	skipInerior:
		SAVE_REGS;
		mov eax, CurrentCarPtr;
		mov eax, [eax];
		add eax, 0xBF0;
		push[eax];
		push _Roof;
		call CheckCarData;
		cmp eax, 2;
		RESTORE_REGS;
		jne skipRoof;

		xor edx, edx;
		mov dl, [esi + 0x000002D4];
		push 00;
		mov ecx, esi;
		push edx;
		push 0x4c; // Roof
		push 0x301B0BF9; // item name
		call FeCustomizeParts_AddMenuOption;

	skipRoof:
		SAVE_REGS;
		mov eax, CurrentCarPtr;
		mov eax, [eax];
		add eax, 0xBF0;
		push[eax];
		push _PopUpHeadLights
			call CheckCarData;;
		cmp eax, 2;
		RESTORE_REGS;
		jne skipLights;

		xor edx, edx;
		mov dl, [esi + 0x000002D4];
		push 00;
		mov ecx, esi;
		push edx;
		push 0x20; // Headlight left
		push 0x3691263B; // item name
		call FeCustomizeParts_AddMenuOption;

	skipLights:
		SAVE_REGS;
		mov eax, CurrentCarPtr;
		mov eax, [eax];
		add eax, 0xBF0;
		push[eax];
		push _FrontBadging;
		call CheckCarData;
		cmp eax, 1;
		RESTORE_REGS;
		jne skipFrontBadging;

		xor edx, edx;
		mov dl, [esi + 0x000002D4];
		push 00;
		mov ecx, esi;
		push edx;
		push 0x49; // Front badging
		push 0x3691263B; // item name
		call FeCustomizeParts_AddMenuOption;

	skipFrontBadging:
		SAVE_REGS;
		mov eax, CurrentCarPtr;
		mov eax, [eax];
		add eax, 0xBF0;
		push[eax];
		push _RearBadging;
		call CheckCarData;
		cmp eax, 1;
		RESTORE_REGS;
		jne skipRearBadging;

		xor edx, edx;
		mov dl, [esi + 0x000002D4];
		push 00;
		mov ecx, esi;
		push edx;
		push 0x4B; // Rear badging
		push 0x3691263B; // item name
		call FeCustomizeParts_AddMenuOption;

	skipRearBadging:
		jmp AftermarketTuning1;
	}
}

DWORD FixInteriorPartLoad1 = 0x0085FA25;
void __declspec(naked) FixInteriorPartLoadCave()
{
	__asm
	{
		cmp ebp, 0x1D; // Interoir
		jne defaultPartLoad;
		cmp ebp, eax;
		jmp interoirPartLoad;

	defaultPartLoad:
		cmp al, bl;
	interoirPartLoad:
		lea eax, [esp + 0x24];

		jmp FixInteriorPartLoad1;
	}
}

void __stdcall AddEmptyPartToList(void* firstItem, int part, void* emptyItem)
{
	if (part == 0x30)// spoiler
	{

	}
}

DWORD AddEmptyPart = 0x0085FA4D;
DWORD PartsPtr = 0x00B76860;
void __declspec(naked) AddEmptyPartCave()
{
	__asm
	{
		// original code
		mov[esp + 14], ebx;
		mov[esp + 24], ebx;

		SAVE_REGS;
		mov eax, [PartsPtr];
		mov eax, [eax];
		add eax, 0x188;
		mov eax, [eax];
		push eax;
		push ebp;
		mov eax, [esp + 0x3c];// +1C originally
		push eax;
		call AddEmptyPartToList;
		RESTORE_REGS;

		jmp AddEmptyPart;
	}
}

void MakeLod(char* str, char lod)
{
	if (strstr(str, "DAMAGE") == NULL)
	{
		int len = strlen(str);
		str[len - 1] = 'A';
	}
}

int GetCustomSpoilerHash(bool type, char* str, int carId)
{
	// For menu we can just get current car
	if (*GameState == 3)
	{
		int* temp = CurrentCarPtr;
		temp = (int*)* temp;
		temp += 0xBF0 / 4;
		carId = *temp;
	}

	int spoilers = 0;
	if (type)
	{
		//spoilers = CheckCarData(6, carId);
	}
	else
	{
		//spoilers = CheckCarData(5, carId);
	}

	char style[3];
	memcpy(style, str + 6, 2);
	style[2] = '\0';
	int styleNum = atoi(style);

	if (GetBit(spoilers, styleNum - 1))
	{
		char buffer[64]; // Hope it is enough

		strcpy(buffer, str);
		strcpy(buffer + 9, "SPOILER_");
		strcpy(buffer + 17, str + 9);

		//if (CheckCarData(7, carId))
		{
			MakeLod(buffer, 'A');
		}

		char* carName = GetCarTypeName(carId);
		unsigned int carHash = StringHash(carName);
		return StringHashModel(buffer, carHash);
	}

	return 0;
}

int __cdecl ChangePartString(int carId, char* str, int modelHash)
{
	if (forceLodA)
	{
		MakeLod(str, 'A');
	}
	else
	{
		for (CarData* i : carList)
		{
			if (StringHash(i->Name) != modelHash)
			{
				continue;
			}

			if (i->ForceLodA)
			{
				MakeLod(str, 'A');
			}
		}
	}

	//if (strstr(str, "ROTOR") != NULL)
	//{
	//	return StringHash((char*)"BRAKE_STYLE03_A");
	//}

	//if (modelHash == 0xC93B73FD) // SPOILER
	//{
	//	int hash = GetCustomSpoilerHash(false, str, carId);
	//	if (hash != 0)
	//	{
	//		return hash;
	//	}
	//}

	return StringHashModel(str, modelHash);
}

DWORD HookPartLoad1 = 0x007CDC8D;
void __declspec(naked) HookPartLoadCave()
{
	__asm
	{
		mov eax, esp;
		add eax, 0x13C;
		push[eax];
		call ChangePartString;
		add esp, 0xC;
		jmp HookPartLoad1;
	}
}

DWORD PassCurrentCarLoading1 = 0x007CFD5D;
void __declspec(naked) PassCurrentCarLoading1Cave()
{
	__asm
	{
		mov eax, esp
		mov eax, [eax]
		add eax, 0x8
		push[eax];
		push edi;
		push edx;
		push 00;
		push ebx;

		jmp PassCurrentCarLoading1;
	}
}

DWORD PassCurrentCarLoading2 = 0x007CFC4C;
void __declspec(naked) PassCurrentCarLoading2Cave()
{
	__asm
	{
		push[ebp];
		push 00;
		push edi;
		push 00;
		push ebx;

		jmp PassCurrentCarLoading2;
	}
}

DWORD PassCurrentCarLoading3 = 0x007CFF96;
void __declspec(naked) PassCurrentCarLoading3Cave()
{
	__asm
	{
		mov eax, [esp + 4];//TODO check if eax is ok
		add eax, 8;
		push[eax];
		push 00;
		push edx;
		push 00;
		push esi;

		jmp PassCurrentCarLoading3;
	}
}

DWORD PassCurrentCarLoading4 = 0x007CFEED;
void __declspec(naked) PassCurrentCarLoading4Cave()
{
	__asm
	{
		mov eax, [esp + 4];//TODO check if eax is ok
		add eax, 8;
		push[eax];
		push esi;
		push edx;
		push 00;
		push edi;

		jmp PassCurrentCarLoading4;
	}
}

DWORD PassCurrentCarLoading5 = 0x007D55C9;
void __declspec(naked) PassCurrentCarLoading5Cave()
{
	__asm
	{
		mov ecx, [esp + 8];
		push[ecx];
		push 00;
		push esi;
		push 00;
		push eax;

		jmp PassCurrentCarLoading5;
	}
}

DWORD CarShadow1 = 0x007E5A6A;
DWORD CarShadow2 = 0x007C9F10;
const char* NeonBlue = "CARSHADOW1";
const char* NeonGreen = "CARSHADOW2";
void __declspec(naked) CarShadowCave()
{
	__asm
	{
		mov ecx, [ebp + 8];
		mov ecx, [ecx + 0x000001EC];
		call CarShadow2;

		cmp eax, 0x89F23A14; // green
		jne neonBlue;
		push NeonGreen;
		jmp neonExit;


	neonBlue:
		cmp eax, 0x880C43AB; // blue
		jne neonOriginal;
		push NeonBlue;
		jmp neonExit;

	neonOriginal:
		push 0x009F26B4;
	neonExit:
		jmp CarShadow1;
	}
}

DWORD FrontBadge1 = 0x00859D02;
DWORD FrontBadge2 = 0x00859C8B;
void __declspec(naked) FrontBadgeCave()
{
	__asm
	{
		mov ebx, eax;
		SAVE_REGS;
		push[esi];
		push _FrontBadging;
		call CheckCarData;
		cmp eax, 1;
		RESTORE_REGS;
		jne FrontBadgeDefault;
		jmp FrontBadge1;

	FrontBadgeDefault:
		test ebx, ebx;
		jne FrontBadgeDefault2;
		jmp FrontBadge1;

	FrontBadgeDefault2:
		jmp FrontBadge2;
	}
}

DWORD RearBadge1 = 0x00859988;
DWORD RearBadge2 = 0x0085996B;
void __declspec(naked) RearBadge1Cave()
{
	__asm
	{
		call GetPart;

		SAVE_REGS;
		push[esi];
		push _RearBadging;
		call CheckCarData;
		cmp eax, 1;
		RESTORE_REGS;
		jne RearBadgeDefault;
		jmp RearBadge1;

	RearBadgeDefault:
		test eax, eax;
		jne RearBadgeDefault2;
		jmp RearBadge1;

	RearBadgeDefault2:
		jmp RearBadge2;
	}
}

DWORD RearBadge21 = 0x008599D2;
void __declspec(naked) RearBadge2Cave()
{
	__asm
	{
		SAVE_REGS;
		push[esi];
		push _RearBadging;
		call CheckCarData;
		cmp eax, 1;
		RESTORE_REGS;
		jne RearBadgeDefault;
		jmp RearBadge21;

	RearBadgeDefault:
		call InstallPart;
		jmp RearBadge21;
	}
}

int exhaustFxHash = StringHash((char*)"EXHAUST_FX");
int leftExhaustHash = StringHash((char*)"LEFT_EXHAUST");
int rightExhaustHash = StringHash((char*)"RIGHT_EXHAUST");
int centerExhaustHash = StringHash((char*)"CENTER_EXHAUST");
bool __stdcall CheckEmmiter(void* rearBumper, int hash1, int hash2)
{
	if (rearBumper != NULL) {
		if (!IsNotAutosculpt(rearBumper))
		{
			if (hash1 == exhaustFxHash)
			{
				if (hash2 == hash1)
				{
					return false; // Disable exhaust fx from body
				}

				if (hash2 == leftExhaustHash || hash2 == rightExhaustHash || hash2 == centerExhaustHash)
				{
					return true;
				}
			}
		}
	}

	return hash1 == hash2;
}

DWORD SkipEmmiter = 0x007BEC35;
DWORD ApplyEmmiter = 0x007BEBFB;
void __declspec(naked) FixExhaustCave1()
{
	__asm
	{
		SAVE_REGS;
		mov ecx, [esp + 0x30]; // +0x18 originally but shifted due to save regs
		mov ecx, [ecx + 0x000003F0];
		mov ecx, [ecx + 0x0000017C];
		push edx;
		push[eax + edi * 4];
		push ecx;
		call CheckEmmiter;
		test al, al;
		RESTORE_REGS;

		je skip;
		jmp ApplyEmmiter;
	skip:
		jmp SkipEmmiter;
	}
}

vector<MountPoint*> exhaustMpList;
MountPoint* GetExhaustMountPoint(Vector * position)
{
	for (MountPoint* i : exhaustMpList)
	{
		if (i->position == *position)
		{

			return i;
		}
	}

	MountPoint* exhaustMP = new MountPoint();
	exhaustMP->rotationMatrix.x.z = 1;
	exhaustMP->rotationMatrix.y.y = 1;
	exhaustMP->rotationMatrix.z.x = -1;
	exhaustMP->position = *position;
	exhaustMpList.push_back(exhaustMP);

	return exhaustMP;
}


void __stdcall CopyMountPointForExhaust(MountPoint * original, int nmem)
{
	Vector* newMem = (Vector*)(nmem + 4);
	*newMem = original->position;


	int hash = original->hash;
	int addr;
	if (hash == leftExhaustHash || hash == rightExhaustHash || hash == centerExhaustHash)
	{

		addr = (int)GetExhaustMountPoint(&original->position);
	}
	else
	{
		addr = (int)original;
	}

	int* ptr = (int*) & (newMem->w);
	*ptr = addr;
}

DWORD FixExhaust2 = 0x007BEC27;
void __declspec(naked) FixExhaustCave2()
{
	__asm
	{
		SAVE_REGS;
		push eax;
		push esi;
		call CopyMountPointForExhaust;
		RESTORE_REGS;
		jmp FixExhaust2;
	}
}

void InitPopupHeadLights()
{
	injector::MakeJMP(0x0085980B, PopUpHeadlightsCave, true);
	injector::MakeJMP(0x0086527D, PopupHeadlightsOnCave, true);
}

void InitCustomizationMenuItems()
{
	injector::MakeJMP(0x0086617D, EnableChopTopMenuItemCave, true);

	injector::MakeJMP(0x00866210, AftermarketTuningCave, true);

	injector::MakeJMP(0x0085FA1F, FixInteriorPartLoadCave, true);

	injector::MakeJMP(0x0085FA45, AddEmptyPartCave, true);

	injector::MakeJMP(0x00859C85, FrontBadgeCave, true);

	injector::MakeJMP(0x00859962, RearBadge1Cave, true);
	injector::MakeJMP(0x008599CD, RearBadge2Cave, true);
}

void InitSunRoof()
{
	injector::MakeJMP(0x008598ED, HandleRoofCave, true);
	injector::MakeJMP(0x0085992C, FixRoofCarName1Cave, true);
	injector::MakeJMP(0x00859933, FixRoofCarName2Cave, true);
	injector::WriteMemory<char>(0x00859946, 0x20, true);
}

void InitPartLoadHook()
{
	injector::MakeJMP(0x007CDC85, HookPartLoadCave, true);

	injector::MakeJMP(0x007CFD58, PassCurrentCarLoading1Cave, true);
	injector::WriteMemory<char>(0x007CFD66, 0x14, true);

	injector::MakeJMP(0x007CFC46, PassCurrentCarLoading2Cave, true);
	injector::WriteMemory<char>(0x007CFC55, 0x14, true);

	injector::MakeJMP(0x007CFF90, PassCurrentCarLoading3Cave, true);
	injector::WriteMemory<char>(0x007CFF9F, 0x14, true);

	injector::MakeJMP(0x007CFEE8, PassCurrentCarLoading4Cave, true);
	injector::WriteMemory<char>(0x007CFEF4, 0x14, true);

	injector::MakeJMP(0x007D55C3, PassCurrentCarLoading5Cave, true);
	injector::WriteMemory<char>(0x007D55D6, 0x14, true);
	injector::WriteMemory<char>(0x007D55D1, 0x24, true);
}

void InitExhaustFix()
{
	if (fixExhaustFx)
	{
		char disableRearBumperCheck[5] = { 0xB8, 0x01, 0x00, 0x00, 0x00 };
		injector::WriteMemoryRaw(0x007CC6BD, disableRearBumperCheck, 5, true);

		injector::MakeJMP(0x007BEBF6, FixExhaustCave1, true);
		injector::MakeJMP(0x007BEC12, FixExhaustCave2, true);
	}
}

void Init()
{
	CIniReader iniReader("ExtendedCustomization.ini");
	forceLodA = iniReader.ReadInteger((char*)"GENERAL", (char*)"ForceLodA", 0);
	fixExhaustFx = iniReader.ReadInteger((char*)"GENERAL", (char*)"FixExhaustFx", 1);

	AddDefaultCars();

	LoadCarData();

	InitPopupHeadLights();
	InitCustomizationMenuItems();
	InitSunRoof();
	InitPartLoadHook();
	InitExhaustFix();

	/*injector::MakeJMP(0x007E5A65, CarShadowCave, true);
	injector::WriteMemory<char>(0x007E5E4F, 0xEB, true);*/
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
		IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)(base);
		IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);

		if ((base + nt->OptionalHeader.AddressOfEntryPoint + (0x400000 - base)) == 0x87E926) // Check if .exe file is compatible - Thanks to thelink2012 and MWisBest
		{
			Init();
		}
		else
		{
			MessageBoxA(NULL, "This .exe is not supported.\nPlease use v1.4 English nfsc.exe (6,88 MB (7.217.152 bytes)).", "NFSC - Customization Extender", MB_ICONERROR);
			return FALSE;
		}
	}
	break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

