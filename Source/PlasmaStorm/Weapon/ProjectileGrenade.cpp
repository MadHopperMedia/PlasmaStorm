// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"


AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();

	StartDestroyTimer();
	SpawnTrailSystem();

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound)
	{
		APSCharacter* HitPlayer = Cast<APSCharacter>(ImpactResult.GetActor());
		if (HitPlayer && HitPlayer != GetOwner())
		{
			APawn* OwnerPawn = Cast<APawn>(GetOwner());
			if (OwnerPawn)
			{
				AController* OwnerController = OwnerPawn->Controller;
				if (OwnerController)
				{
					UGameplayStatics::ApplyDamage(ImpactResult.GetActor(), 60.f, OwnerController, this, UDamageType::StaticClass());
				}
			}
		}
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BounceSound,
			GetActorLocation()
		);
	}
}

void AProjectileGrenade::Destroyed()
{
	
	ExplodeDamage();
	Super::Destroyed();
}