// Copyright Nathan Guerin

#pragma once

#define TRACE_LENGTH 100000.f

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),
};

UENUM(BlueprintType)
enum class EHighlightColor
{
	None = 0 UMETA(DisplayName = "None"),
	Purple = 250 UMETA(DisplayName = "Purple"),
	Blue = 251 UMETA(DisplayName = "Blue"),
	Tan = 252 UMETA(DisplayName = "Tan"),

	Max UMETA(DisplayName = "Max")
};

// Dirty way to get the actual value stored in the EHighlightColor enum
FORCEINLINE int32 operator * (EHighlightColor T) { return static_cast<int32>(T); }
