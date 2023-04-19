// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "PlasmaStorm/PlayerController/PSPlayerController.h"
#include "PlasmaStorm/PSComponents/LagCompensationComponent.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	FVector DistanceToTarget = GetActorLocation() - HitTarget;
	
	if (DistanceToTarget.Size() < 3000 && GetWeaponType() != EWeaponType::EWT_RocketLauncher && GetWeaponType() != EWeaponType::EWT_GrenadeLauncher)
	{
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
			if (PSCharacter && InstigatorController)
			{
				if (OwnerPawn->IsLocallyControlled() && HasAuthority())
				{
					UGameplayStatics::ApplyDamage(
						PSCharacter,
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
				if (!HasAuthority() && bUseServerSideRewind)
				{
					PSOwnerCharacter = PSOwnerCharacter == nullptr ? Cast<APSCharacter>(OwnerPawn) : PSOwnerCharacter;
					PSOwnerController = PSOwnerController == nullptr ? Cast<APSPlayerController>(InstigatorController) : PSOwnerController;
					if (PSOwnerController && PSOwnerCharacter && PSOwnerCharacter->GetLagCompensation())
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
	else
	{
		SpawnProjectile(HitTarget);
	}	
}

void AProjectileWeapon::SpawnProjectile(const FVector& HitTarget)
{
	

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	UWorld* World = GetWorld();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket && World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;
		if (bUseServerSideRewind && ServerSideRewindProjectileClass)
		{
			if (InstigatorPawn->HasAuthority()) // Server
			{
				if (InstigatorPawn->IsLocallyControlled()) // server, Host - Use replicated projectile
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
				}
				else // server, not locally controlled - spawn none replicated projectile
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
				}
			}
			else // Client, using SSR
			{
				if (InstigatorPawn->IsLocallyControlled()) // Client, locally controlled - spawn none replicated projectile use SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					SpawnedProjectile->Damage = Damage;
				}
				else // Client, none locally controlled - spawn none replicated projectile not useing SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		else // weapon not using SSR
		{
			if (InstigatorPawn->HasAuthority())
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;
			}
		}		
	}
}

void AProjectileWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
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