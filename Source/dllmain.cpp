
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

#include "Headers/MemoryTools.hpp"
#include "Headers/StreamParser.hpp"





// Aliases ------------------------------------------------------------------------------------------------------------------------------------------

using MemoryTools::byte;
using MemoryTools::address;

using Parser = StreamParser::Parser<>;





// Mod data -----------------------------------------------------------------------------------------------------------------------------------------

constexpr float kph2mph = .6214f; // mph / kph
constexpr float mps2kph = 3.6f;   // kph / mps
constexpr float deg2rad = .0175f; // rad / deg
constexpr float gravity = -9.81f; // mps / second

// Activation
float maxDurationScale; // unity

// Recharging
float activeScale; // unity

float minDriftBase; // mps
float minSlipRad;   // rad

// Physics
float timeScale;    // unity
float gravityBoost; // mps / second

float activationScale;   // unity
float deactivationScale; // unity

float frictionScale;    // unity
float maxSteeringAngle; // degrees

float aeroDragScale;  // unity
float steerDragScale; // unity





// Extraction helpers -------------------------------------------------------------------------------------------------------------------------------

struct Bounds
{
// Members

	std::optional<float> lower;
	std::optional<float> upper;


// Methods

	void Enforce(float& value) const
	{
		if (this->lower and (value < *(this->lower))) value = *(this->lower);
		if (this->upper and (value > *(this->upper))) value = *(this->upper);
	}
};



static bool Extract
(
	const Parser::Section* const section,
	const std::string_view       key,
	float&                       value,
	const Bounds&                limits = {}
) {
	if (not Parser::ExtractValues<float>(section, key, value)) return false;

	limits.Enforce(value);

	return true;
}





// Initialisation helpers ---------------------------------------------------------------------------------------------------------------------------

static void InitialiseActivation(const Parser& parser)
{
	bool hasFiniteDuration = false;

	float minSpeedToActivate = 30.f / kph2mph; // kph
	float maxDuration        = 10.f;           // seconds

	// Extraction
	if (const auto* const section = parser.GetSection("Speedbreaker:Activation"))
	{
		Extract(section, "minCarSpeed", minSpeedToActivate, {0.f});
		hasFiniteDuration = Extract(section, "maxDuration", maxDuration, {.001f});
	}

	maxDurationScale = 1.f / maxDuration;

	// Code changes
	MemoryTools::Write<float> (minSpeedToActivate * kph2mph, {0x8B01C0});
	MemoryTools::Write<float*>(&maxDurationScale,            {0x6EDDC3});

	if (not hasFiniteDuration)
		MemoryTools::Write<byte>(0xEB, {0x6EDDB9}); // jump near, relative
}



static void InitialiseRecharging(const Parser& parser)
{
	bool canRechargePassively = false;

	float minSpeedToRecharge = 100.f / kph2mph; // kph
	float rechargeTime       = 25.f;            // seconds

	activeScale = .5f; // unity

	float minDriftSpeed = 35.f / kph2mph; // kph
	float minDriftSlip  = 30.f;           // degrees

	// Extraction
	if (const auto* const section = parser.GetSection("Speedbreaker:Recharging"))
	{
		const bool speedDefined = Extract(section, "minCarSpeed",  minSpeedToRecharge, {0.f});
		const bool timeDefined  = Extract(section, "rechargeTime", rechargeTime,       {.001f});

		canRechargePassively = (speedDefined or timeDefined);

		Extract(section, "activeScale",   activeScale,   {0.f});
		Extract(section, "minDriftSpeed", minDriftSpeed, {0.f});
		Extract(section, "minDriftSlip",  minDriftSlip,  {0.f, 90.f});
	}

	minDriftBase = minDriftSpeed / mps2kph;
	minSlipRad   = minDriftSlip * deg2rad;

	// Code changes
	MemoryTools::Write<float> (minSpeedToRecharge * kph2mph, {0x901AE8});
	MemoryTools::Write<float> (rechargeTime,                 {0x901AE4});
	MemoryTools::Write<float*>(&activeScale,                 {0x6A99F8});
	MemoryTools::Write<float*>(&minDriftBase,                {0x6A99B8});
	MemoryTools::Write<float*>(&minSlipRad,                  {0x6A99CB});

	if (not canRechargePassively)
		MemoryTools::Write<byte>(0xEB, {0x6EDDE3}); // jump near, relative
}



static void InitialisePhysics(const Parser& parser)
{
	timeScale = 4.f; // unity

	float carMassScale = 2.f; // unity
	float gravityScale = 3.f; // unity

	float toDilationScale   = 1.f; // unity
	float fromDilationScale = 1.f; // unity

	activationScale   = 2.f; // unity
	deactivationScale = .5f; // unity

	float frictionBoost = 75.f; // percent

	maxSteeringAngle = 60.f; // degrees

	float aeroDragReduction  = 75.f;  // percent
	float steerDragReduction = 100.f; // percent

	// Extraction
	if (const auto* const section = parser.GetSection("Speedbreaker:Physics"))
	{
		Extract(section, "timeScale",    timeScale,    {1.f});
		Extract(section, "carMassScale", carMassScale, {0.f});
		Extract(section, "gravityScale", gravityScale);

		Extract(section, "toDilationScale",   toDilationScale,   {.001f});
		Extract(section, "fromDilationScale", fromDilationScale, {.001f});

		Extract(section, "frictionBoost",    frictionBoost,    {0.f});
		Extract(section, "maxSteeringAngle", maxSteeringAngle, {0.f, 90.f});

		Extract(section, "aeroDragReduction",  aeroDragReduction,  {0.f,  100.f});
		Extract(section, "steerDragReduction", steerDragReduction, {15.f, 100.f});
	}

	const float timeRate = 1.f / timeScale; // unity

	gravityBoost = gravity * (gravityScale - 1.f);

	const float complementRatio = (1.f - timeRate) / .75f; // unity

	activationScale   *= complementRatio * toDilationScale;
	deactivationScale *= complementRatio * fromDilationScale;

	frictionScale  = frictionBoost / 100.f;

	aeroDragScale  = aeroDragReduction           / 100.f;
	steerDragScale = (steerDragReduction - 15.f) / 100.f;

	// Code changes
	MemoryTools::Write<float*>(&timeScale,         {0x472C53});
	MemoryTools::Write<float> (timeRate,           {0x6F4DD4});
	MemoryTools::Write<float*>(&activationScale,   {0x6F4DC8});
	MemoryTools::Write<float*>(&deactivationScale, {0x6F4DF4});
	MemoryTools::Write<float> (carMassScale,       {0x901AEC});
	MemoryTools::Write<float*>(&gravityBoost,      {0x6B1F17});
	MemoryTools::Write<float*>(&frictionScale,     {0x6A9E37});
	MemoryTools::Write<float*>(&maxSteeringAngle,  {0x69E990});
	MemoryTools::Write<float*>(&aeroDragScale,     {0x6B201E});
	MemoryTools::Write<float*>(&steerDragScale,    {0x6B1FA3});
}





// Hook functions -----------------------------------------------------------------------------------------------------------------------------------

HOOK_ORIGINAL(Initialise);

static void __cdecl Initialise
(
	const size_t  numArgs,
	const address argArray
) {
	// Call original function first
	CALL_HOOK_ORIGINAL(Initialise, numArgs, argArray);

	#ifdef _DEBUG
	while (not IsDebuggerPresent()); // halt until debugger is attached
	#endif

	// Parse configuration file
	const std::filesystem::path configFile = "scripts/NFSMWSpeedFixerSettings.ini";

	std::ifstream fileStream(configFile);
	if (not fileStream.is_open()) return;

	const Parser parser
	(
		fileStream, 
		/* sectionCapacity        = */ 3, 
		/* pairCapacityPerSection = */ 9
	);

	// Initialise features
	InitialiseActivation(parser);
	InitialiseRecharging(parser);
	InitialisePhysics   (parser);
}





// DLL hook boilerplate -----------------------------------------------------------------------------------------------------------------------------

BOOL WINAPI DllMain
(
	const HINSTANCE hinstDLL,
	const DWORD     fdwReason,
	const LPVOID    lpvReserved
) {
	if (fdwReason != DLL_PROCESS_ATTACH) return TRUE;

	if (MemoryTools::GetEntryPoint() != 0x3C4040) // .exe-dependent entry point
	{
		MessageBoxA(NULL, "This .exe isn't compatible with SpeedFixer.\nSee SpeedFixer's README for help.", "NFSMW SpeedFixer", MB_ICONERROR);

		return FALSE; // should never happen (assuming the user has actually read the README, which... yeah...)
	}

	PATCH_HOOK_FUNCTION(Initialise, 0x6665B4); // InitializeEverything (0x665FC0)
	
	return TRUE;
}