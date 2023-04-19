// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class PLASMASTORM_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	UFUNCTION(BlueprintImplementableEvent)
	void AddImpulse();

	/**
	* Used with server side rewind
	*/
	UPROPERTY(EditAnywhere)
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;
	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;
	
	float Damage = 20.f;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void ExplodeDamage();

	UPROPERTY(EditAnywhere, Category = "Explosion")
	float InnerRadious = 200.f;
	UPROPERTY(EditAnywhere, Category = "Explosion")
	float OuterRadious = 800.f;
	UPROPERTY(EditAnywhere, Category = "Explosion")
	float DamageFalloff = 1.f;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	float ImpulsePower = 2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, Category = "Trail")
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	void SpawnTrailSystem();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(BlueprintReadWrite)
	class APSCharacter* CurrentTarget;

private:
	
	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere, Category = "Trail")
	float DestroyTime = 3.f;


	UPROPERTY(EditAnywhere)
	 UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TraceComponent;

	

	

public:	
	
	void SetTarget(APSCharacter* Target);

};
