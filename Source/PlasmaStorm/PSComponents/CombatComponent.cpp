// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "PlasmaStorm/Weapon/weapon.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "PlasmaStorm/PlayerController/PSPlayerController.h"
#include "PlasmaStorm/HUD/PSHud.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "PlasmaStorm/Weapon/Projectile.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlasmaStorm/Weapon/Shotgun.h"


// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;
	FlyingCrosshairTransition = CrosshairOffset;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, MountedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();	
	if (Character)
	{
		//Character->SetAimWalkSpeed(BaseWalkSpeed);
		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}		
	}	
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled())
	{
		
		TraceUnderCrosshairs(DeltaTime);
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
		
	}	
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;	
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		//float NewSpeed = bAiming ? AimWalkSpeed : Character->GetMaxSpeed();
		Character->SetAimWalkSpeed(bAiming);
		if (EquippedWeapon)
		{
			EquippedWeapon->GetIsAiming(bAiming);
		}
		
	}
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		//Character->ShowSniperScopeWidget(bIsAiming);
		if (bIsAiming)
		{
			EquippedWeapon->GetWeaponMesh()->SetVisibility(false);
		}
		else
		{
			EquippedWeapon->GetWeaponMesh()->SetVisibility(true);
		}		
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		float NewSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;
		Character->SetAimWalkSpeed(bAiming);
		if (EquippedWeapon)
		{
			EquippedWeapon->GetIsAiming(bAiming);
		}
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryweapon(WeaponToEquip);
		bPlaySecondaryEquip = true;		
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);
		bPlaySecondaryEquip = false;
	}	
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(WeaponToEquip);
	ReloadEmptyWeapon();
	SetWeaponRange();
	SetRecoil();
}

void UCombatComponent::EquipSecondaryweapon(AWeapon* WeaponToEquip)
{	
	if (WeaponToEquip == nullptr) return;

	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorSecondarySocket(WeaponToEquip);
	SecondaryWeapon->SetOwner(Character);
	PlayEquipWeaponSound(WeaponToEquip);
	bPlaySecondaryEquip = bPlaySecondaryEquip;
}

void UCombatComponent::EquipMountedWeapon(class AWeapon* WeaponToEquip)
{
	MountedWeapon = WeaponToEquip;
	MountedWeapon->SetWeaponState(EWeaponState::EWS_EquippedMountedWeapon);
	AttachActorMountedSocket(WeaponToEquip);
	MountedWeapon->SetOwner(Character);
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		SetWeaponRange();
		SetRecoil();
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		bPlaySecondaryEquip = bPlaySecondaryEquip;
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorSecondarySocket(SecondaryWeapon);
		if (bPlaySecondaryEquip == true)
		{
			PlayEquipWeaponSound(SecondaryWeapon);
		}
		
	}
}

void UCombatComponent::OnRep_MountedWeapon()
{	
	MountedWeapon->SetWeaponState(EWeaponState::EWS_EquippedMountedWeapon);
	AttachActorMountedSocket(MountedWeapon);
	MountedWeapon->SetOwner(Character);
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{		
		EquippedWeapon->Dropped();
	}
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

void UCombatComponent::SwapWeapons()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);

	bPlaySecondaryEquip = false;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorSecondarySocket(SecondaryWeapon);
	ReloadEmptyWeapon();
	SetWeaponRange();
	SetRecoil();
}

void UCombatComponent::SwapToMountedWeapon()
{
	
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = MountedWeapon;
	MountedWeapon = TempWeapon;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedMountedWeapon);
	SetWeaponRange();
	SetRecoil();
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;

	bool bUsePistolSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());		
	}
}

void UCombatComponent::AttachActorSecondarySocket(AActor* ActorToAttach)
{
	if(Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr ) return;

	const USkeletalMeshSocket* SecondarySocket = Character->GetMesh()->GetSocketByName("SecondarySocket");
	if (SecondarySocket)
	{
		SecondarySocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorMountedSocket(AActor* ActorToAttach)
{
	if(Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;

	const USkeletalMeshSocket* MountedWeaponSocket = Character->GetMesh()->GetSocketByName("MountedWeaponSocket");
	if (MountedWeaponSocket)
	{
		MountedWeaponSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<APSPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr || WeaponToEquip->EquipSound == nullptr) return;
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation()
		);
	}
}

void UCombatComponent::SetWeaponRange()
{
	if (EquippedWeapon)
	{
		WeaponRange = EquippedWeapon->GetWeaponRange();
	}
}

void UCombatComponent::SetRecoil()
{
	RecoilAmount = EquippedWeapon->GetWeaponRecoil();
}

void UCombatComponent::ReloadEmptyWeapon()
{
	
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && EquippedWeapon->GetAmmo() != EquippedWeapon->GetMagCapacity())
	{
		SetAiming(false);
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	
	return 0;
	
	
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}	
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<APSPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);

	}
	EquippedWeapon->AddAmmo(-ReloadAmount);	
}

void UCombatComponent::HandleReload()
{
	
	Character->PlayReloadMontage();
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;	
	if (bFireButtonPressed)
	{
		Fire();
	}
	
}

void UCombatComponent::Fire()
{
	if (CanFire() && CombatState != ECombatState::ECS_ThrowingGrenade)
	{	
		if (HitTarget.Size() == 0)
		{
			StartFireTimer();
			return;
		}
		bCanFire = false;		
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = FMath::Clamp(CrosshairShootingFactor += .75f, .5f, 3.f);

			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
				break;
			}
		}
		Character->AddRecoilOnFire(RecoilAmount);
		StartFireTimer();		
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		ServerFire(HitTarget);
		if(!Character->HasAuthority()) LocalFire(HitTarget);
	}	
}

void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		ServerFire(HitTarget);
		if (!Character->HasAuthority()) LocalFire(HitTarget);
	}
}

void UCombatComponent::FireShotgun()
{
	
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun && Character)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		if (!Character->HasAuthority()) ShotgunLocalFire(HitTargets);
		ServerShotgunFire(HitTargets);
	}	
}

void UCombatComponent::StartFireTimer()
{
	float FireDelay;
	if (!Character->GetIsFlying())
	{
		FireDelay = EquippedWeapon->FireDelay;
	}
	else
	{
		FireDelay = MountedWeapon->FireDelay;
	}

	if (EquippedWeapon == nullptr  || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		FireDelay
	);
	Character->AddRecoilOnFire(0);
}

void UCombatComponent::FireTimerFinished()
{

	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (!Character->GetIsFlying())
	{
		if (bFireButtonPressed && EquippedWeapon->bAutomatic)
		{
			Fire();
		}
	}
	else
	{
		if (bFireButtonPressed && MountedWeapon->bAutomatic && Character->GetIsFlyingForward())
		{
			Fire();
		}
	}
	
	ReloadEmptyWeapon();
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	
	MulticastFire(TraceHitTarget);
	
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);	
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	ShotgunLocalFire(TraceHitTargets);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		if (!Character->GetIsFlying())
		{
			Character->PlayFireMontage(bAiming);
			EquippedWeapon->Fire(TraceHitTarget);
		}
		else
		{			
			MountedWeapon->Fire(TraceHitTarget);
		}
	}

	if (TargetCharacter)
	{
		Character->SetTargetCharacter(TargetCharacter);
	}
	else
	{
		Character->SetTargetCharacter(nullptr);
	}
}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun == nullptr || Character == nullptr) return;
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::TraceUnderCrosshairs(float DeltaTime)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	float YLocation;
	if (Character)
	{
		YLocation = Character->GetIsFlying() ? ViewportSize.Y / 2.f : (ViewportSize.Y / 2.f) + CrosshairOffset;
	}
	FVector2D CrosshairLocation(ViewportSize.X / 2, YLocation);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	FHitResult LineTraceHit;
	FHitResult TraceHitResult;
	if (bScreenToWorld && EquippedWeapon != nullptr)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		bool SphereTrace = false;
		FVector End = Start + CrosshairWorldDirection * WeaponRange;
		GetWorld()->LineTraceSingleByChannel(
			LineTraceHit,
			Start,
			End,
			ECollisionChannel::ECC_Visibility);
		if (LineTraceHit.GetActor() && LineTraceHit.GetActor() && LineTraceHit.GetActor() != GetOwner() && LineTraceHit.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			SphereTrace = false;
		}
		else
		{
			SphereTrace = true;
		}

			
		float TargetSpeed;
		float PitchInput = 0;		
		float Radious = 10;
		TArray<AActor*> ActorsToIgnore;		
		ActorsToIgnore.Emplace(Character);		
		if (SphereTrace)
		{
			UKismetSystemLibrary::SphereTraceSingle(this, Start, End, EquippedWeapon->GetTraceRadious(), UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
			true, ActorsToIgnore, EDrawDebugTrace::None, TraceHitResult, true);
		}
		
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor() != GetOwner() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()
			|| LineTraceHit.GetActor() && LineTraceHit.GetActor() && LineTraceHit.GetActor() != GetOwner() && LineTraceHit.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
			if (Character)
			{				 
				float Amount = bAiming ? StickyAssistAmountAiming : StickyAssistAmount;
				Character->SetOverlappingCharacterMultiplier(Amount);
				Character->GetWorldTimerManager().SetTimer(
					LostTargetTimer,
					this,
					&UCombatComponent::LostTargetTimerFinished,
					TimeBeforeLosingTarget
				);
				TargetCharacter = Cast<APSCharacter>(TraceHitResult.GetActor());
				TargetSpeed = TurnToEnemyInterpSpeed;
				if (TraceHitResult.bBlockingHit && TraceHitResult.GetActor() == TargetCharacter)
				{					
					
					AimAssistSpeedYaw = FMath::FInterpTo(AimAssistSpeedYaw, TurnToEnemyInterpSpeed, GetWorld()->GetDeltaSeconds(), 5);
				}				
			}			
		}
		else
		{				
			AimAssistSpeedYaw = FMath::FInterpTo(AimAssistSpeedYaw, 0, GetWorld()->GetDeltaSeconds(), 1);
			TargetSpeed = 0.f;
			HUDPackage.CrosshairsColor = FLinearColor::White;
			if (Character)
			{
				Character->SetOverlappingCharacterMultiplier(1.f);				
			}			
		}

		if (!TraceHitResult.bBlockingHit && !LineTraceHit.bBlockingHit)
		{
			LineTraceHit.ImpactPoint = End;

		}
		
		TraceTargetForWeaponRotation = LineTraceHit.ImpactPoint;
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor() != GetOwner() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>() && SphereTrace)
		{
			AimAssist(DeltaTime, TraceHitResult);
			HitTarget = TraceHitResult.ImpactPoint;
			
		}
		else
		{
			AimAssist(DeltaTime, LineTraceHit);
			HitTarget = LineTraceHit.ImpactPoint;
			
		}
		if (LineTraceHit.ImpactPoint.Size() == 0)
		{
			TraceTargetForWeaponRotation = End;
			HitTarget = End;
		}

		if (GEngine)
		{
			//GEngine->AddOnScreenDebugMessage(-1, .1, FColor::Red, FString::Printf(TEXT("Distance: %f"), TraceTargetForWeaponRotation.Size()));
		}
		if (EquippedWeapon && TargetCharacter != nullptr)
		{
			EquippedWeapon->SetTarget(TargetCharacter);
		}
	}	
}

void UCombatComponent::AimAssist(float DeltaTime, FHitResult& HitResult)
{
	if (TargetCharacter && !TargetCharacter->IsElimmed() && TargetCharacter->Speed.Size() > 10)
	{		
		float RangeMin = 1000.0f;
		float RangeMax = WeaponRange;
		float AdjustmentFactor = RangeMax;
		float RangeValue = TargetCharacter->GetDistanceTo(Character);
		float InverseValue = 1.0f - (RangeValue - RangeMin) / (RangeMax - RangeMin);
		float AdjustedValue = AdjustmentFactor * InverseValue;
		float AimRange = UKismetMathLibrary::NormalizeToRange(AdjustedValue, RangeMax, 0);
		AimRange = FMath::Clamp(AimRange, 0.2f, 1.f);
		
		FRotator AimTargetRotation = UKismetMathLibrary::FindLookAtRotation(Character->GetFollowCamera()->GetComponentLocation(), TargetCharacter->GetMesh()->GetSocketLocation(FName("spine_05")));
		FRotator RotationForPawn = FRotator(Character->GetActorRotation().Pitch, AimTargetRotation.Yaw, Character->GetActorRotation().Roll);
		FRotator RotationForPitch = FRotator(AimTargetRotation.Pitch, Character->GetCameraBoom()->GetComponentRotation().Yaw, Character->GetCameraBoom()->GetComponentRotation().Roll);
		AimAssistSpeedPitch = AimAssistSpeedPitch * AimRange;
		AimAssistSpeedYaw = AimAssistSpeedYaw * AimRange;
		FRotator PawnRotation = FMath::RInterpTo(Character->GetActorRotation(), RotationForPawn, DeltaTime, AimAssistSpeedYaw);
		FRotator PawnPitch;
		if (AimRange == 0) return;
		float DisticeForYaw = FVector(HitResult.ImpactPoint - TargetCharacter->GetMesh()->GetSocketLocation(FName("spine_05"))).Size();
		if (DisticeForYaw > 3)
		{
			Character->SetActorRotation(PawnRotation);
		}		
		
		float DisticeForPitch = FVector(HitResult.ImpactPoint - TargetCharacter->GetMesh()->GetSocketLocation(FName("spine_05"))).Size();
		float PitchDirection = PawnPitch.Pitch - RotationForPitch.Pitch;
		if (PitchDirection <= 0 && DisticeForPitch > 35 || PitchDirection > 0 && DisticeForPitch > 200)
		{
			AimAssistSpeedPitch = FMath::FInterpTo(AimAssistSpeedPitch, PitchToEnemyInterpSpeed, GetWorld()->GetDeltaSeconds(), 5);
			PawnPitch = FMath::RInterpTo(Character->GetCameraBoom()->GetComponentRotation(), RotationForPitch, DeltaTime, AimAssistSpeedPitch);
			Character->GetCameraBoom()->SetWorldRotation(PawnPitch);
			Character->PitchFloat = PawnPitch.Pitch;
		}
		else
		{
			AimAssistSpeedPitch = FMath::FInterpTo(AimAssistSpeedPitch, 0, GetWorld()->GetDeltaSeconds(), 5);
		}		
		
	
	}	
}

void UCombatComponent::LostTargetTimerFinished()
{
	TargetCharacter = nullptr;	
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<APSPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<APSHud>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			
			if (EquippedWeapon && !Character->GetIsFlying())
			{
				
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;				
			}
			else
			{
				if (!MountedWeapon)
				{
					HUDPackage.CrosshairsCenter = nullptr;
					HUDPackage.CrosshairsLeft = nullptr;
					HUDPackage.CrosshairsRight = nullptr;
					HUDPackage.CrosshairsTop = nullptr;
					HUDPackage.CrosshairsBottom = nullptr;
					return;
				}
				HUDPackage.CrosshairsCenter = MountedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = MountedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = MountedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = MountedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = MountedWeapon->CrosshairsBottom;
			}
			if (bAiming && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				Character->ShowSniperScopeWidget(false);
			}
			else
			{
				//Character->ShowSniperScopeWidget(bAiming);
			}

			// Calculate crosshair spread

			FVector2D WalkSpeedRange(0.f, 600);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (!Character->bIsGrounded && !Character->GetIsFlying())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.5f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, .58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 5.f);

			HUDPackage.CrosshairSpread =
				.5f +
				CrosshairVelocityFactor + 
				CrosshairInAirFactor - 
				CrosshairAimFactor +
				CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
			
			if (!Character->GetIsFlying())
			{
				FlyingCrosshairTransition = FMath::FInterpTo(FlyingCrosshairTransition, CrosshairOffset, GetWorld()->GetDeltaSeconds(), 6);
				HUD->SetCrosshairsOffset(FlyingCrosshairTransition);
			}
			else
			{
				FlyingCrosshairTransition = FMath::FInterpTo(FlyingCrosshairTransition, 0.0, GetWorld()->GetDeltaSeconds(), 6);
				HUD->SetCrosshairsOffset(FlyingCrosshairTransition);
			}			
		}
	}

}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

bool UCombatComponent::CanFire()
{	
	
	if (!Character->GetIsFlying())
	{
		if (EquippedWeapon == nullptr) return false;
		if(!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied) return true;
	}
	else if(Character->GetIsFlying())
	{
		if (MountedWeapon == nullptr) return false;
		if(!MountedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied) return true;
	}
	return false;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<APSPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
		
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo); 
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo); 
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo); 
	CarriedAmmoMap.Emplace(EWeaponType::EWT_MiniGun, StartingMiniGunAmmo);
}

void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	SetAiming(false);
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
	
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<APSPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
	
	
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize Target)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FRotator ThrowRotation = ToTarget.Rotation() + FRotator(GrenadeThrowAngle, 0.f, 0.f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ThrowRotation,
				SpawnParams
				);
		}
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

int32 UCombatComponent::GetMaxAmmoForWeapon(EWeaponType WeaponType)
{
	;
	switch (WeaponType)
	{
	case EWeaponType::EWT_AssaultRifle:
		MaxCarriedAmmo = MaxARAmmo;
		break;
	case EWeaponType::EWT_RocketLauncher:
		MaxCarriedAmmo = MaxRocketAmmo;
		break;
	case EWeaponType::EWT_Pistol:
		MaxCarriedAmmo = MaxPistolAmmo;
		break;
	case EWeaponType::EWT_SubmachineGun:
		MaxCarriedAmmo = MaxSMGAmmo;
		break;
	case EWeaponType::EWT_Shotgun:
		MaxCarriedAmmo = MaxShotgunAmmo;
		break;
	case EWeaponType::EWT_SniperRifle:
		MaxCarriedAmmo = MaxSniperAmmo;
		break;
	case EWeaponType::EWT_GrenadeLauncher:
		MaxCarriedAmmo = MaxGrenadeLauncherAmmo;
		break;
	}
	return MaxCarriedAmmo;
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{	
		
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0 , MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

void UCombatComponent::PickupGrenade(int32 GrenadeAmount)
{
	Grenades = FMath::Clamp(Grenades + GrenadeAmount, 0, MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::PlayEquippedWeaponZoomSound()
{
	if (EquippedWeapon && EquippedWeapon->ZoomSound)
	{
		UGameplayStatics::PlaySound2D(
			this,
			EquippedWeapon->ZoomSound);
	}
}

void UCombatComponent::PlayEquippedWeaponUnZoomSound()
{
	if (EquippedWeapon && EquippedWeapon->UnZoomSound)
	{
		UGameplayStatics::PlaySound2D(
			this,
			EquippedWeapon->UnZoomSound);
	}
}

void UCombatComponent::Melee()
{

}