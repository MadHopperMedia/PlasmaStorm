// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PSGameMode.generated.h"


namespace MatchState
{
	extern PLASMASTORM_API const FName Cooldown; // Match duratin has been reached display winner and begin cooldown timer
}

/**
 * 
 */
UCLASS()
class PLASMASTORM_API APSGameMode : public AGameMode
{
	GENERATED_BODY()
public:

	APSGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class APSCharacter* ElimmedCharacter, class APSPlayerController* VictimController, class APSPlayerController* AttackerController);	
	virtual void RequestRespawn(APawn* ElimmedCharacter , AController* ElimmedController);

	void PlayerLeftGame(class APSPlayerState* PlayerLeaving);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 5.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 5.f;
	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;
	

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
