// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class PLASMASTORM_API AFlag : public AWeapon
{
	GENERATED_BODY()
public:

		AFlag();
		virtual void Dropped() override;
		void ReturnFlag();
		void SetReturnTimer();
protected:
	virtual void BeginPlay() override;
	virtual void OnEquipped() override;
	virtual void OnDropped() override;
	virtual void DroppedTimerFinished() override;

	UPROPERTY()
		FTransform OriginalTransform;
private:
		UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* FlagMesh;
		UFUNCTION(Server, Reliable)
		void ServerReturnFlag();
		float ReturnDelay = 30.f;
		UPROPERTY(EditAnywhere)
		TSubclassOf<class AFlag> FlagClass;

public:
	FORCEINLINE FTransform GetOriginalTransform() const { return OriginalTransform; }
	



};
