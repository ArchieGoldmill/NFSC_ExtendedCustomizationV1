#pragma once

enum IniOption
{
	_PopUpHeadLights,
	_ForceLodA,
	_IneriorHI,
	_Roof,
	_ChopTop,
	_FrontBadging,
	_RearBadging,
	_HeadlightsOnOffTexture
};

struct CarData
{
	char* Name;

	int PopUpHeadLights = 0; // disabled, popups, customizible

	bool ForceLodA = false;

	bool IneriorHI = false;

	int Roof = 0; // default, sun roof, roof menu option

	int ChopTop = 0; // default, enabled, disabled

	bool FrontBadging = false;

	bool RearBadging = false;

	bool HeadlightsOnOffTexture = false;

	int CustomAftermarketSpoilers = false;
	int CustomAutosculptSpoilers = false;
};

struct Vector
{
	float x;
	float y;
	float z;
	float w;
};

struct RotationMatrix
{
	Vector x;
	Vector y;
	Vector z;
};

struct MountPoint
{
	int hash;
	int blank[3];
	RotationMatrix rotationMatrix;
	Vector position;
};

bool operator==(const Vector& a, const Vector& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

struct MenuPartItem
{
	MenuPartItem* next;
	MenuPartItem* prev;
	int v1;
	int* part;
	char* type;
	int partNum;
	int v2;
	int v3;
	int v4;
	int v5;
};