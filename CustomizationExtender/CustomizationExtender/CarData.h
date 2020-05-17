#pragma once


struct CarData
{
	char* Name;

	bool PopUpHeadLights = false;

	bool ForceLodA = false;

	bool IneriorHI = false;

	int Roof = 0; // default, sun roof, roof menu option

	bool AutosculptExhaustFx = false; // default, enabled

	int ChopTop = 0; // default, enabled, disabled

	int CustomAftermarketSpoilers = false;
	int CustomAutosculptSpoilers = false;
};