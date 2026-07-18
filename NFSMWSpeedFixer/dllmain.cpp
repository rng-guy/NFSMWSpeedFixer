
// Compatibility checks -----------------------------------------------------------------------------------------------------------------------------

#ifndef _MSC_VER
#error "SpeedFixer requires MSVC."

#elif (_MSC_VER < 1930)
#error "SpeedFixer requires Visual Studio 2022 or newer."

#elif ((not defined(_WIN32)) or defined(_WIN64))
#error "SpeedFixer requires 32-bit Windows."

#elif ((not defined(_MSVC_LANG)) or (_MSVC_LANG < 202002L))
#error "SpeedFixer requires C++20 or newer."

#endif





// Project includes ---------------------------------------------------------------------------------------------------------------------------------

#include <Windows.h>

#ifdef _DEBUG
#include <debugapi.h>
#endif

#include <fstream>
#include <optional>
#include <filesystem>
#include <string_view>

#include "MemoryTools.h"
#include "StreamParser.h"





// Aliases ------------------------------------------------------------------------------------------------------------------------------------------

using byte    = MemoryTools::byte;
using address = MemoryTools::address;

using Parser = StreamParser::Parser<>;





// Conversion constants -----------------------------------------------------------------------------------------------------------------------------

constexpr float kph2mph = .6214f; // mph / kph
constexpr float mps2kph = 3.6f;   // kph / mps
constexpr float deg2rad = .0175f; // rad / deg
constexpr float gravity = -9.81f; // mps / second





// Speedbreaker parameters --------------------------------------------------------------------------------------------------------------------------

// Activation
float minSpeedToActivate = 30.f / kph2mph; // kph
float maxDuration        = 10.f;           // seconds

// Recharging
float minSpeedToRecharge = 100.f / kph2mph; // kph
float rechargeTime       = 25.f;            // seconds

float activeScale   = .5f;
float minDriftSpeed = 35.f / kph2mph; // kph
float minDriftSlip  = 30.f;           // degrees

// Physics
float timeScale    = 4.f;
float carMassScale = 2.f;
float gravityScale = 3.f;

float frictionBoost    = 75.f; // percent
float maxSteeringAngle = 60.f; // degrees

float aerodynamicDrag = 25.f; // percent
float steeringDrag    =  0.f; // percent





// Derived parameters -------------------------------------------------------------------------------------------------------------------------------

// Activation
bool  durationEnabled;
float maxDurationScale;

// Recharging
bool passiveEnabled;

float minDriftBase; // mps
float minSlipRad;   // rad

// Physics
float gravityBoost; // mps / second

float frictionScale;

float aerodynamicScale;
float steeringScale;





// Parsing structs and functions --------------------------------------------------------------------------------------------------------------------

struct Bounds
{
	std::optional<float> lower;
	std::optional<float> upper;


	void Enforce(float& value) const
	{
		if (this->lower and (value < *(this->lower)))
			value = *(this->lower);

		if (this->upper and (value > *(this->upper)))
			value = *(this->upper);
	}
};



static bool ParseFloat
(
	const auto&            section,
	const std::string_view key,
	float&                 value,
	const Bounds           limits = {}
) {
	const bool isValid = Parser::GetValues<float>(section, key, value);

	limits.Enforce(value);

	return isValid;
}



static bool ParseParameters()
{
	std::ifstream fileStream(std::filesystem::path("scripts/NFSMWSpeedFixerSettings.ini"));
	if (not fileStream.is_open()) return false; // missing file; disable mod

	constexpr size_t sectionCapacity        = 3; // sections
	constexpr size_t pairCapacityPerSection = 7; // pairs

	const Parser parser(fileStream, sectionCapacity, pairCapacityPerSection);

	const auto& sections = parser.GetSections();

	// Activation parameters
	auto foundSection = sections.find("Speedbreaker:Activation");

	if (foundSection != sections.end())
	{
		const auto& section = foundSection->second;

		ParseFloat(section, "minCarSpeed", minSpeedToActivate, {0.f});

		durationEnabled = ParseFloat(section, "maxDuration", maxDuration, {.001f});
	}
	else durationEnabled = false;

	// Recharging parameters
	foundSection = sections.find("Speedbreaker:Recharging");
	
	if (foundSection != sections.end())
	{
		const auto& section = foundSection->second;

		const bool speedDefined = ParseFloat(section, "minCarSpeed",  minSpeedToRecharge, {0.f});
		const bool timeDefined  = ParseFloat(section, "rechargeTime", rechargeTime,       {.001f});

		passiveEnabled = (speedDefined or timeDefined);

		ParseFloat(section, "activeScale",   activeScale,   {0.f});
		ParseFloat(section, "minDriftSpeed", minDriftSpeed, {0.f});
		ParseFloat(section, "minDriftSlip",  minDriftSlip,  {0.f, 90.f});
	}
	else passiveEnabled = false;

	// Physics parameters
	foundSection = sections.find("Speedbreaker:Physics");

	if (foundSection != sections.end())
	{
		const auto& section = foundSection->second;

		ParseFloat(section, "timeScale",        timeScale,        {1.f});
		ParseFloat(section, "carMassScale",     carMassScale,     {0.f});
		ParseFloat(section, "gravityScale",     gravityScale);
		ParseFloat(section, "frictionBoost",    frictionBoost,    {0.f});
		ParseFloat(section, "maxSteeringAngle", maxSteeringAngle, {0.f, 90.f});
		ParseFloat(section, "aerodynamicDrag",  aerodynamicDrag,  {0.f, 100.f});
		ParseFloat(section, "steeringDrag",     steeringDrag,     {0.f, 85.f});
	}

	return true;
}





// Initialisation and injection ---------------------------------------------------------------------------------------------------------------------

address InitialiseSpeedFixerOriginal = 0x0;

static void __cdecl InitialiseSpeedFixer
(
	const size_t  numArgs,
	const address argArray
) {
	const auto OriginalFunction = reinterpret_cast<void (__cdecl*)(size_t, address)>(InitialiseSpeedFixerOriginal);

	// Call original function first
	OriginalFunction(numArgs, argArray);

	// Halt until debugger is attached
	#ifdef _DEBUG
	while (not IsDebuggerPresent());
	#endif

	// Global mod parameters
	if (not ParseParameters()) return; // missing file; disable mod

	// Code modifications (activation)
	maxDurationScale = 1.f / maxDuration;

	MemoryTools::Write<float> (minSpeedToActivate * kph2mph, {0x8B01C0});
	MemoryTools::Write<float*>(&maxDurationScale,            {0x6EDDC3});

	if (not durationEnabled)
		MemoryTools::Write<byte>(0xEB, {0x6EDDB9}); // jump near, relative

	// Code modifications (recharging)
	minDriftBase = minDriftSpeed / mps2kph;
	minSlipRad   = minDriftSlip * deg2rad;

	MemoryTools::Write<float> (minSpeedToRecharge * kph2mph, {0x901AE8});
	MemoryTools::Write<float> (rechargeTime,                 {0x901AE4});
	MemoryTools::Write<float*>(&activeScale,                 {0x6A99F8});
	MemoryTools::Write<float*>(&minDriftBase,                {0x6A99B8});
	MemoryTools::Write<float*>(&minSlipRad,                  {0x6A99CB});

	if (not passiveEnabled)
		MemoryTools::Write<byte>(0xEB, {0x6EDDE3}); // jump near, relative

	// Code modifications (physics)
	gravityBoost     = gravity * (gravityScale - 1.f);
	frictionScale    = frictionBoost / 100.f;
	aerodynamicScale = (100.f - aerodynamicDrag) / 100.f;
	steeringScale    = (85.f - steeringDrag) / 100.f;

	MemoryTools::Write<float*>(&timeScale,        {0x472C53});
	MemoryTools::Write<float> (1.f / timeScale,   {0x6F4DD4});
	MemoryTools::Write<float> (carMassScale,      {0x901AEC});
	MemoryTools::Write<float*>(&gravityBoost,     {0x6B1F17});
	MemoryTools::Write<float*>(&frictionScale,    {0x6A9E37});
	MemoryTools::Write<float*>(&maxSteeringAngle, {0x69E990});
	MemoryTools::Write<float*>(&aerodynamicScale, {0x6B201E});
	MemoryTools::Write<float*>(&steeringScale,    {0x6B1FA3});
}





// DLL hook boilerplate -----------------------------------------------------------------------------------------------------------------------------

BOOL WINAPI DllMain
(
	const HINSTANCE hinstDLL,
	const DWORD     fdwReason,
	const LPVOID    lpvReserved
) {
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (MemoryTools::GetEntryPoint() != 0x3C4040) // .exe-dependent entry point
		{
			MessageBoxA(NULL, "This .exe isn't compatible with SpeedFixer.\nSee SpeedFixer's README for help.", "NFSMW SpeedFixer", MB_ICONERROR);
			return FALSE; // should never happen (assuming the user has actually read the README, which... yeah...)
		}

		InitialiseSpeedFixerOriginal = MemoryTools::MakeCallHook(0x6665B4, InitialiseSpeedFixer); // InitializeEverything (0x665FC0)
	}

	return TRUE;
}