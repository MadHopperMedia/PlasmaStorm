// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PSGameMode.h"
#include "TeamsPSGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PLASMASTORM_API ATeamsPSGameMode : public APSGameMode
{
	GENERATED_BODY()
public:
	ATeamsPSGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void PlayerEliminated(class APSCharacter* ElimmedCharacter, class APSPlayerController* VictimController, class APSPlayerController* AttackerController) override;

protected:
	virtual void HandleMatchHasStarted() override;

};
