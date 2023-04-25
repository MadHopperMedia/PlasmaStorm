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

};
