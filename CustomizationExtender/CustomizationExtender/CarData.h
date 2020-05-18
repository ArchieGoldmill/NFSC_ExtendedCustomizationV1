#pragma once


struct CarData
{
	char* Name;

	int PopUpHeadLights = 0; // disabled, popupsm customizible

	bool ForceLodA = false;

	bool IneriorHI = false;

	int Roof = 0; // default, sun roof, roof menu option

	bool AutosculptExhaustFx = false; // default, enabled

	int ChopTop = 0; // default, enabled, disabled

	bool FrontBadging = false;

	bool RearBadging = false;

	int CustomAftermarketSpoilers = false;
	int CustomAutosculptSpoilers = false;
};