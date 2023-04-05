// Fill out your copyright notice in the Description page of Project Settings.


#include "PlasmaProjectile.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"

void APlasmaProjectile::BeginPlay()
{
	Super::BeginPlay();


}

void APlasmaProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Start = GetActorLocation() + GetActorForwardVector() * DistanceToStart;
	FVector End = GetActorLocation() + GetActorForwardVector() * 10000.0f;
	FHitResult Hit;
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
	UKismetSystemLibrary::SphereTraceSingle(
		this, Start, End, TraceRadious, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
		true, ActorsToIgnore, DrawTrace, Hit, true
	);

	if (Hit.bBlockingHit)
	{
		APSCharacter* HitPlayer = Cast<APSCharacter>(Hit.GetActor());
		if (HitPlayer && HitPlayer->GetTarget())
		{
			ProjectileMovementComponent->HomingTargetComponent = HitPlayer->GetTarget();
		}
	}

}