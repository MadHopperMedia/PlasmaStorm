// Fill out your copyright notice in the Description page of Project Settings.


#include "ProximityProjectile.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "RocketMovementComponent.h"


void AProximityProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TraceForPlayer();
}

void AProximityProjectile::TraceForPlayer()
{
	FVector Start = GetActorLocation();
	FVector End = Start;
	TArray<FHitResult> Hits;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Emplace(GetOwner());
	UKismetSystemLibrary::SphereTraceMulti(this, Start, End, Radious, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
		true, ActorsToIgnore, EDrawDebugTrace::None, Hits, true);
	for (FHitResult Hit : Hits)
	{
		if (Hit.bBlockingHit)
		{
			if (HitPlayer != nullptr) return;
			HitPlayer = Cast<APSCharacter>(Hit.GetActor());
			if (HitPlayer && HitPlayer->GetIsFlying())
			{
				
				ExplodeDamage();
				AddImpulse();
				StartDestroyTimer();

				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
				}
				if (ImpactSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
				}
				if (ProjectileMesh)
				{
					ProjectileMesh->SetVisibility(false);
				}
				if (CollisionBox)
				{
					CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
				if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
				{
					TrailSystemComponent->GetSystemInstanceController()->Deactivate();
				}
				if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
				{
					ProjectileLoopComponent->Stop();
				}
				Destroy();
			}
		}
	}
	
}