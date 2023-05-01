// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlasmaStorm/PSTypes/Team.h"
#include "FlagZone.generated.h"

UCLASS()
class PLASMASTORM_API AFlagZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlagZone();

	UPROPERTY(EditAnywhere)
	ETeam Team; 

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);

	UPROPERTY(EditAnywhere)
	class USoundCue* FlagCapturedQue;

private:

	UPROPERTY(EditAnywhere)
	class USphereComponent* ZoneSphere;
	UFUNCTION(Server, Reliable)
	void ServerReturnFlag(APSCharacter* Character, AFlag* Flag);
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AFlag> FlagClass;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayFlagCaptured();
	UFUNCTION(Server, Reliable)
	void ServerPlayFlagCaptured();

public:	


};
