// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileBullet.h"
#include "MinigunProjectile.generated.h"

/**
 * 
 */
UCLASS()
class PLASMASTORM_API AMinigunProjectile : public AProjectileBullet
{
	GENERATED_BODY()
	
public:

protected:

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

private:


};
