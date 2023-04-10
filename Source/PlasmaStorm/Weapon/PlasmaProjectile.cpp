// Fill out your copyright notice in the Description page of Project Settings.


#include "PlasmaProjectile.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"

void APlasmaProjectile::BeginPlay()
{
	Super::BeginPlay();
	FVector Start = GetActorLocation() + GetActorForwardVector() * DistanceToStart;
	FVector End = GetActorLocation() + GetActorForwardVector() * 200000.0f;
	TArray<FHitResult> Hit;
	TArray<AActor*> ActorsToIgnore;
	EDrawDebugTrace::Type DrawTrace;
	if (ShowTrace)
	{
		DrawTrace = EDrawDebugTrace::ForOneFrame;
	}
	else
	{
		DrawTrace = EDrawDebugTrace::None;
	}
	if (ProjectileMovementComponent->HomingTargetComponent != nullptr) return;
	UKismetSystemLibrary::SphereTraceMulti(
		this, Start, End, TraceRadious, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
		true, ActorsToIgnore, DrawTrace, Hit, true
	);



	for (FHitResult HitActor : Hit)
	{
		if (HitActor.bBlockingHit)
		{
			APSCharacter* HitPlayer = Cast<APSCharacter>(HitActor.GetActor());
			if (HitPlayer && HitPlayer->GetTarget() && !HitPlayer->IsElimmed())
			{
				//ProjectileMovementComponent->SetVelocityInLocalSpace(FVector::ZeroVector);
				//ProjectileMovementComponent->HomingTargetComponent = HitPlayer->GetTarget();
				
			}
		}
	}
	

}

void APlasmaProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	/*FVector Start = GetActorLocation() + GetActorForwardVector() * DistanceToStart;
	FVector End = GetActorLocation() + GetActorForwardVector() * 5000.0f;
	TArray<FHitResult> Hit;
	TArray<AActor*> ActorsToIgnore;
	EDrawDebugTrace::Type DrawTrace;
	if (ShowTrace)
	{
		DrawTrace = EDrawDebugTrace::ForOneFrame;
	}
	else
	{
		DrawTrace = EDrawDebugTrace::None;
	}
	if (ProjectileMovementComponent->HomingTargetComponent != nullptr) return;
	UKismetSystemLibrary::SphereTraceMulti(
		this, Start, End, TraceRadious, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
		true, ActorsToIgnore, DrawTrace, Hit, true
	);

	

	for (FHitResult HitActor : Hit)
	{
		if (HitActor.bBlockingHit)
		{
			APSCharacter* HitPlayer = Cast<APSCharacter>(HitActor.GetActor());
			if (HitPlayer && HitPlayer->GetTarget())
			{
				ProjectileMovementComponent->HomingTargetComponent = HitPlayer->GetTarget();
				if (GEngine)
				{
					//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString(HitPlayer->GetName()));
				}
			}
		}
	}*/
	/*if (Hit.IsValidBlockingHit())
	{
		APSCharacter* HitPlayer = Cast<APSCharacter>(Hit.GetActor());
		if (HitPlayer && HitPlayer->GetTarget())
		{
			ProjectileMovementComponent->HomingTargetComponent = HitPlayer->GetTarget();
		}
	}*/

}