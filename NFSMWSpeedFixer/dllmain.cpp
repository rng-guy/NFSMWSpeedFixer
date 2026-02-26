#include <fstream>
#include <Windows.h>
#include <filesystem>
#include <string_view>

#include "inipp.h"
#include "MemoryTools.h"





// Parameters ---------------------------------------------------------------------------------------------------------------------------------------

// Conversions
constexpr float kph2mph = .6214f; // mph / kph
constexpr float mps2kph = 3.6f;   // kph / mps
constexpr float deg2rad = .0175f; // rad / deg
constexpr float gravity = -9.81f; // mps / second

// Activation parameters
float minSpeedToActivate = 30.f / kph2mph; // kph
float fullCapacity       = 10.f;           // seconds

// Recharging parameters
float minSpeedToRecharge = 100.f / kph2mph; // kph
float fullRecharge       = 25.f;            // seconds

float rechargeScale = .5f;
float minDriftSpeed = 35.f / kph2mph; // kph
float minDriftSlip  = 30.f;           // degrees

// Physics parameters
float timeScale    = 4.f;
float massScale    = 2.f;
float gravityScale = 3.f;

float frictionBoost    = 75.f; // percent
float maxSteeringAngle = 60.f; // degrees

float aerodynamicDrag = 25.f; // percent
float steeringDrag    =  0.f; // percent

// Derived parameters
float fullCapacityScale;

bool passiveEnabled;

float minDriftBase; // mps
float minSlipRad;   // rad

float gravityBoost; // mps / second

float frictionScale;

float aerodynamicScale;
float steeringScale;





// Parsing functions --------------------------------------------------------------------------------------------------------------------------------

template <typename T>
struct Bounds
{
	std::optional<T> lower;
	std::optional<T> upper;


	void Enforce(T& value) const
	{
		if (this->lower and (value < *(this->lower)))
			value = *(this->lower);

		if (this->upper and (value > *(this->upper)))
			value = *(this->upper);
	}
};



template <typename T>
bool ParseFromFile
(
	const inipp::Ini&      parser,
	const std::string_view section,
	const std::string_view key,
	T&                     value,
	const Bounds<T>&       limits = {}
) {
	const bool isValid = parser.ExtractFromSection<T>(section, key, value);

	limits.Enforce(value);

	return isValid;
}



static bool ParseParameters()
{
	std::ifstream fileStream(std::filesystem::path("scripts/NFSMWSpeedFixerSettings.ini"));
	if (not fileStream.is_open()) return false; // missing file; disable feature

	const inipp::Ini parser(fileStream);

	// Activation parameters
	std::string_view section = "Speedbreaker:Activation";

	ParseFromFile<float>(parser, section, "minSpeed", minSpeedToActivate, {0.f});
	ParseFromFile<float>(parser, section, "duration", fullCapacity,       {.001f});

	// Recharging parameters
	section = "Speedbreaker:Recharging";

	const bool speedDefined = ParseFromFile<float>(parser, section, "minSpeed", minSpeedToRecharge, {0.f});
	const bool timeDefined  = ParseFromFile<float>(parser, section, "recharge", fullRecharge,       {.001f});

	passiveEnabled = (speedDefined or timeDefined);

	ParseFromFile<float>(parser, section, "rechargeScale", rechargeScale, {0.f});
	ParseFromFile<float>(parser, section, "minDriftSpeed", minDriftSpeed, {0.f});
	ParseFromFile<float>(parser, section, "minDriftSlip",  minDriftSlip,  {0.f, 90.f});

	// Physics parameters
	section = "Speedbreaker:Physics";

	ParseFromFile<float>(parser, section, "timeScale",        timeScale,        {1.f});
	ParseFromFile<float>(parser, section, "massScale",        massScale,        {0.f});
	ParseFromFile<float>(parser, section, "gravityScale",     gravityScale);
	ParseFromFile<float>(parser, section, "frictionBoost",    frictionBoost,    {0.f});
	ParseFromFile<float>(parser, section, "maxSteeringAngle", maxSteeringAngle, {0.f, 90.f});
	ParseFromFile<float>(parser, section, "aerodynamicDrag",  aerodynamicDrag,  {0.f, 100.f});
	ParseFromFile<float>(parser, section, "steeringDrag",     steeringDrag,     {0.f, 85.f});

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

	// Apply hooked logic last
	if (not ParseParameters()) return; // missing file; disable feature

	// Code modifications (activation)
	fullCapacityScale = 1.f / fullCapacity;

	MemoryTools::Write<float> (minSpeedToActivate * kph2mph, {0x8B01C0});
	MemoryTools::Write<float*>(&fullCapacityScale,           {0x6EDDC3});

	// Code modifications (recharging)
	minDriftBase = minDriftSpeed / mps2kph;
	minSlipRad   = minDriftSlip * deg2rad;

	MemoryTools::Write<float> (minSpeedToRecharge * kph2mph, {0x901AE8});
	MemoryTools::Write<float> (fullRecharge,                 {0x901AE4});
	MemoryTools::Write<float*>(&rechargeScale,               {0x6A99F8});
	MemoryTools::Write<float*>(&minDriftBase,                {0x6A99B8});
	MemoryTools::Write<float*>(&minSlipRad,                  {0x6A99CB});

	if (not passiveEnabled)
		MemoryTools::Write<byte>(0xEB, {0x6EDDE3}); // unconditional jump

	// Code modifications (physics)
	gravityBoost     = gravity * (gravityScale - 1.f);
	frictionScale    = frictionBoost / 100.f;
	aerodynamicScale = (100.f - aerodynamicDrag) / 100.f;
	steeringScale    = (85.f - steeringDrag) / 100.f;

	MemoryTools::Write<float*>(&timeScale,        {0x472C53});
	MemoryTools::Write<float> (1.f / timeScale,   {0x6F4DD4});
	MemoryTools::Write<float> (massScale,         {0x901AEC});
	MemoryTools::Write<float*>(&gravityBoost,     {0x6B1F17});
	MemoryTools::Write<float*>(&frictionScale,    {0x6A9E37});
	MemoryTools::Write<float*>(&maxSteeringAngle, {0x69E990});
	MemoryTools::Write<float*>(&aerodynamicScale, {0x6B201E});
	MemoryTools::Write<float*>(&steeringScale,    {0x6B1FA3});
}



static bool IsExecutableCompatible()
{
	// Credit: thelink2012 and MWisBest
	const auto base = reinterpret_cast<uintptr_t>(GetModuleHandleA(NULL));

	const auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
	const auto nt  = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);

	return (nt->OptionalHeader.AddressOfEntryPoint == 0x3C4040);
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
		if (not IsExecutableCompatible())
		{
			MessageBoxA(NULL, "This .exe isn't compatible with SpeedFixer.\nSee SpeedFixer's README for help.", "NFSMW SpeedFixer", MB_ICONERROR);

			return FALSE;
		}

		InitialiseSpeedFixerOriginal = MemoryTools::MakeCallHook(InitialiseSpeedFixer, 0x6665B4); // InitializeEverything (0x665FC0)
	}

	return TRUE;
}