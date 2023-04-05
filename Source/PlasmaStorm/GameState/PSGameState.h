// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PSGameState.generated.h"

/**
 * 
 */
UCLASS()
class PLASMASTORM_API APSGameState : public AGameState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class APSPlayerState* ScoringPlayer);
	UPROPERTY(Replicated)
	TArray<APSPlayerState*> TopScoringPlayers;


private:

	float TopScore = 0.f;
	
};
