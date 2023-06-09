// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlasmaStorm/PSTypes/Team.h"
#include "PSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PLASMASTORM_API APSPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);
	

private:
	UPROPERTY()
	class APSCharacter* Character;
	UPROPERTY()
	class APSPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	UFUNCTION()
	void OnRep_Team();
	
	bool bHasTheLead = false;

public:

	FORCEINLINE	ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam TeamToSet);

	void PlayAnnouncement(class USoundCue* Sound, float AnnouncmentDelay);

	FORCEINLINE	void SetHasTheLead(bool bLead) { bHasTheLead = bLead; }
	FORCEINLINE	bool GetHasTheLead() const { return bHasTheLead; }
	
};
