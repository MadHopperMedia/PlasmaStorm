#pragma once

UENUM(BlueprintType)
enum class EFlyingRotation : uint8
{
		
	EFR_NotPitching UMETA(DisplayName = "NotPitching"),
	EFR_PitchingUp UMETA(DisplayName = "Pitching Up"),
	EFR_PitchingDown UMETA(DisplayName = "Pitching Down"),

	EFR_Max UMETA(DisplayName = "DefaultMax")
};