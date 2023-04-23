// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "PlasmaStorm/PlayerController/PSPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "PlasmaStorm/PSComponents/LagCompensationComponent.h"


void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	APSCharacter* OwnerCharacter = Cast<APSCharacter>(OwnerPawn);
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		if (OwnerCharacter && !OwnerCharacter->HasAuthority())
		{
			Start = Start + OwnerCharacter->PlayerVelocity().GetSafeNormal() * 15;
		}		
		

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
		APSCharacter* PSCharacter = Cast<APSCharacter>(FireHit.GetActor());
		if (PSCharacter  && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;				

				UGameplayStatics::ApplyDamage(
					PSCharacter,
					DamageToCause,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			if (!HasAuthority() && bUseServerSideRewind)
			{
				PSOwnerCharacter = PSOwnerCharacter == nullptr ? Cast<APSCharacter>(OwnerPawn) : PSOwnerCharacter;
				PSOwnerController = PSOwnerController == nullptr ? Cast<APSPlayerController>(InstigatorController) : PSOwnerController;
				if (PSOwnerController && PSOwnerCharacter && PSOwnerCharacter->GetLagCompensation() && PSOwnerCharacter->IsLocallyControlled())
				{
					PSOwnerCharacter->GetLagCompensation()->ServerScoreRequest(PSCharacter,
						Start,
						HitTarget,
						PSOwnerController->GetServerTime() - PSOwnerController->SingleTripTime,
						this
					);
				}
			}
			
		}
		if (PSCharacter)
		{
			PSCharacter->Impulse = -FireHit.ImpactNormal * ImpulsePower;
		}
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}		
		
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
	
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;			
		}
		else
		{
			BeamEnd = End;
			OutHit.ImpactPoint = End;
		}
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

