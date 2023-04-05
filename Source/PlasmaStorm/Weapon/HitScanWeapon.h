// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class PLASMASTORM_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:

	virtual void Fire(const FVector& HitTarget) override;

protected:

	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;
	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;
	UPROPERTY(EditAnywhere)
	float ImpulsePower = 2000;

	

private:
	

	

	/**
	* Trace end with scatter
	*/
	UPROPERTY(EditAnywhere, Category = "WeaponScatter")
	float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere, Category = "WeaponScatter")
	float SphereRadious = 800.f;
	UPROPERTY(EditAnywhere, Category = "WeaponScatter")
	bool bUseScatter = false;


public:

	
};
