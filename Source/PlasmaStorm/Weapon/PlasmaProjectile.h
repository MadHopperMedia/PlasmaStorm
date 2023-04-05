// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileBullet.h"
#include "PlasmaProjectile.generated.h"

/**
 * 
 */
UCLASS()
class PLASMASTORM_API APlasmaProjectile : public AProjectileBullet
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY(EditAnywhere)
	float TraceRadious = 10;

	UPROPERTY(EditAnywhere)
	float DistanceToStart = 100;

	UPROPERTY(EditAnywhere)
	bool ShowTrace = false;
	
};
