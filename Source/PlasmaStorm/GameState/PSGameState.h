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


	/**
	* Teams
	*/

	void RedTeamScores();
	void BlueTeamScores();

	UPROPERTY()
	TArray<APSPlayerState*> RedTeam;

	UPROPERTY()
	TArray<APSPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_BlueTeamScore();

protected:

	UPROPERTY(EditAnywhere)
	class USoundCue* TakenTheLead;

	UPROPERTY(EditAnywhere)
	USoundCue* TakenTheLeadFirstBlood;

	UPROPERTY(EditAnywhere)
	USoundCue* FirstBlood;

	UPROPERTY(EditAnywhere)
	USoundCue* TiedTheLead;

	UPROPERTY(EditAnywhere)
	USoundCue* LostTheLead;

	UPROPERTY(EditAnywhere)
	USoundCue* RedHasTakenTheLead;

	UPROPERTY(EditAnywhere)
	USoundCue* BlueHasTakenTheLead;

	UPROPERTY(EditAnywhere)
	float AnnouncmentDelay = .5f;

	bool bBlueInLead = false;
	bool bRedInLead = false;
	

private:

	float TopScore = 0.f;
	
};
