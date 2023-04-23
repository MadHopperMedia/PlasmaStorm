// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileRocket.h"
#include "ProximityProjectile.generated.h"

/**
 * 
 */
UCLASS()
class PLASMASTORM_API AProximityProjectile : public AProjectileRocket
{
	GENERATED_BODY()


public:
	virtual void Tick(float DeltaTime) override;

protected:

	void TraceForPlayer();

private:
	UPROPERTY(EditAnywhere)
	float Radious = 200;
	UPROPERTY()
	class APSCharacter* HitPlayer;
};
