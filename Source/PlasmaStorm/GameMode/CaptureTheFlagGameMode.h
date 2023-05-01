// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsPSGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PLASMASTORM_API ACaptureTheFlagGameMode : public ATeamsPSGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class APSCharacter* ElimmedCharacter, class APSPlayerController* VictimController, class APSPlayerController* AttackerController) override;
	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);
};
