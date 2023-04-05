// Fill out your copyright notice in the Description page of Project Settings.


#include "PSCharacter.h"
#include "CTFPawn.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"
#include "PlasmaStorm/Weapon/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "PlasmaStorm/PlasmaStorm.h"
#include "PlasmaStorm/Weapon/WeaponTypes.h"
#include "PlasmaStorm/GameMode/PSGameMode.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlasmaStorm/PlayerState/PSPlayerState.h"
#include "PlasmaStorm/PSComponents/CombatComponent.h"
#include "PlasmaStorm/PSComponents/BuffComponent.h"
#include "PlasmaStorm/Character/PSCharacterAnimInstance.h"
#include "PlasmaStorm/PlayerController/PSPlayerController.h"
#include "Components/SceneComponent.h"


APSCharacter::APSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	Target = CreateDefaultSubobject<USceneComponent>(TEXT("TargetPoint"));
	Target->SetupAttachment(GetMesh());

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	
}

void APSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APSCharacter::ForwardMovement);
	PlayerInputComponent->BindAxis("MoveRight", this, &APSCharacter::RightMovement);
	PlayerInputComponent->BindAxis("LookUp", this, &APSCharacter::PlayerPitch);
	PlayerInputComponent->BindAxis("Turn", this, &APSCharacter::PlayerYaw);	
	PlayerInputComponent->BindAxis("Roll", this, &APSCharacter::PlayerRoll);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APSCharacter::JumpButtonPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACTFPawn::StopJump);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ACTFPawn::Crouch);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &APSCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APSCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APSCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APSCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APSCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APSCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &APSCharacter::GrenadeButtonPressed);
	PlayerInputComponent->BindAction("Boost", IE_Pressed, this, &APSCharacter::BoostButtonPressed);
	PlayerInputComponent->BindAction("Boost", IE_Released, this, &APSCharacter::BoostButtonReleased);
	PlayerInputComponent->BindAction("Melee", IE_Pressed, this, &APSCharacter::MeleeButtonPressed);

}

void APSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
	}
	
}

void APSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APSCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(APSCharacter, Ao_Pitch);
	DOREPLIFETIME(APSCharacter, Health);
	DOREPLIFETIME(APSCharacter, Shield);
	DOREPLIFETIME(APSCharacter, Impulse); 
	DOREPLIFETIME(APSCharacter, bDisableGameplay);
	DOREPLIFETIME(APSCharacter, bIsBoosting);
}

void APSCharacter::BeginPlay()
{
	Super::BeginPlay();
	

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &APSCharacter::RecieveDamage);
		//FVector NewLocation = GetActorLocation()->Point
		FVector result = FMath::VRand();
		result *= FMath::RandRange(100.f, 300.f);

		SetActorLocation(FVector(GetActorLocation().X + result.X, GetActorLocation().Y + result.Y, GetActorLocation().Z + 20));
	}	

	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
	
}

void APSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
	HideCharacterIfCharacterClose();
	PollInit();	
	float dotProduct = FVector::DotProduct(Speed, GetActorForwardVector());
	if (dotProduct > 0)
	{
		bIsAccelerating = true;
	}
	else
	{
		bIsAccelerating = false;
	}
	if (Shield < ShieldRequiredToFly)
	{
		CTFComponent->bCanFly = false;
	}
	else
	{
		CTFComponent->bCanFly = true;
	}
}

void APSCharacter::PollInit()
{	
	if (PSPlayerController == nullptr)
	{
		PSPlayerController = PSPlayerController == nullptr ? Cast<APSPlayerController>(Controller) : PSPlayerController;
		if (PSPlayerController)
		{
			SpawnDefaultWeapon();
			UpdateHUDHealth();
			UpdateHUDShield();
			UpdateHUDAmmo();
			UpdateHUDStamina();
			bInverted = PSPlayerController->bInverted;
			bToggleBoost = PSPlayerController->bToggleBoost;
			
			
		}
	}
	

	if (PSPlayerState == nullptr)
	{
		PSPlayerState = GetPlayerState<APSPlayerState>();
		if (PSPlayerState)
		{
			PSPlayerState->AddToScore(0.f);
			PSPlayerState->AddToDefeats(0);
			
		}
	}
}

void APSCharacter::HideCharacterIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (Combat && Combat->MountedWeapon && Combat->MountedWeapon->GetWeaponMesh())
		{
			Combat->MountedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (Combat && Combat->MountedWeapon && Combat->MountedWeapon->GetWeaponMesh())
		{
			Combat->MountedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void APSCharacter::ForwardMovement(float Val)
{
	
	MoveForward(Val);
}

void APSCharacter::RightMovement(float Val)
{
	MoveRight(Val);
}

void APSCharacter::PlayerPitch(float Val)
{
	Val = FMath::Clamp(Val, -2, 2);
	if (bIsBoosting)
	{
		Val = FMath::Clamp(Val, -.5, .5);
	}
	if (bInverted)
	{
		Val = Val * -1.f;
	}
	if (Combat && Combat->bAiming)
	{
		if (Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
		{
			Val = Val * SniperZoomYawSpeed;
		}
		else
		{
			Val = Val * ZoomYawSpeed;
		}
	}	
	LookUp((Val * OverlappingCharacterMultiplier));
}

void APSCharacter::PlayerYaw(float Val)
{
	if (IsLocallyControlled())
	{
		Val = FMath::Clamp(Val, -2, 2);
		if (bIsBoosting)
		{
			Val = FMath::Clamp(Val, -.5, .5);
		}

		if (Combat && Combat->bAiming)
		{
			if (Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
			{
				Val = Val * SniperZoomYawSpeed;
			}
			else
			{
				Val = Val * ZoomYawSpeed;
			}

		}
		Turn(Val * OverlappingCharacterMultiplier);
	}	
}

void APSCharacter::PlayerRoll(float Val)
{
	Roll(Val);
}

void APSCharacter::PlayerCrouch()
{
	Crouch();
}

void APSCharacter::JumpButtonPressed()
{
	if (Combat && Combat->bAiming) return;
	Jump();
}

void APSCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void APSCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}	
}

void APSCharacter::EquipButtonPressed()
{	
	if (Combat && Combat->CombatState == ECombatState::ECS_Unoccupied && !bIsFlying)
	{
		if (Combat->bAiming)
		{
			Combat->SetAiming(false);
		}
		ServerEquipButtonPressed();
	}	
}

void APSCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if(Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}		
	}
}

bool APSCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool APSCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* APSCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

void APSCharacter::AimButtonPressed()
{
	if (Combat && !bIsFlying)
	{
		Combat->SetAiming(true);
		Combat->PlayEquippedWeaponZoomSound();
	}
}

void APSCharacter::AimButtonReleased()
{
	if (Combat && Combat->bAiming)
	{
		Combat->SetAiming(false);
		Combat->PlayEquippedWeaponUnZoomSound();
	}
}

void APSCharacter::SetAimWalkSpeed(bool Aiming)
{
	SetAimingWalkSpeed(Aiming);
}

void APSCharacter::AimOffset(float DeltaTime)
{
	/*
	//*/

	

	if (IsLocallyControlled())
	{
		Ao_Pitch = GetBaseAimRotation().Pitch;		
		NewAo_Pitch = Ao_Pitch;
		if (!HasAuthority())
		{
			ServerAimOffset(Ao_Pitch);
		}
	}
	
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->SetActorHiddenInGame(bIsFlying);
	}

	//StartingAimRotation = GetActorRotation();
	
	FVector Velocity = Speed;
	Velocity.Z = 0.f;
	float PlayerSpeed = Velocity.Size();
	bool bIsInAir = !bIsGrounded;
	bool bIsHovering = bIsFlying;
	
	if (PlayerSpeed == 0.f && bIsGrounded|| bIsHovering)
	{

		//FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.0f);
		FRotator CurrentAimRotation =  GetBaseAimRotation();
		//FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		FRotator DeltaAimRotation = CurrentAimRotation - LastRotation;
		float DeltaTimeSinceLastUpdate = DeltaTime - LastUpdateTime;
		
		Ao_Yaw = DeltaAimRotation.Yaw;
		
		if (IsLocallyControlled())
		{
			Flying_PitchOffset = PitchVal * 1.5f;
			Flying_RollOffset = RollVal;
		}
		else
		{
			Flying_PitchOffset = DeltaAimRotation.Pitch;
			Flying_RollOffset = DeltaAimRotation.Roll;
			if (GetActorUpVector().Z < 0)
			{
				Flying_PitchOffset = Flying_PitchOffset * -1.0f;
				
			}
		}
		
		
		LastUpdateTime = DeltaTime;
		LastRotation = CurrentAimRotation;
		
		
	}
	if (PlayerSpeed > 0.f && !bIsHovering)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		Ao_Yaw = 0.f;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	TurnInPlace(DeltaTime);
	
}

void APSCharacter::TurnInPlace(float DeltaTime)
{
	if (!bIsFlying)
	{
		if (Ao_Yaw < -1)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else if (Ao_Yaw > 1)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
	}
	else
	{
		if (Ao_Yaw < -0.3f)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else if (Ao_Yaw > 0.3f)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
	}	
}

void APSCharacter::ServerAimOffset_Implementation(float Val)
{
	Ao_Pitch = Val;
	NewAo_Pitch = Ao_Pitch;
}

void APSCharacter::OnRep_AoPitch()
{
	//Ao_Pitch = NewAo_Pitch;
}

void APSCharacter::BoostButtonPressed()
{
	if (bToggleBoost && bIsBoosting) 
	{
		StopBoosting();
	}
	else
	{
		StartBoosting();
	}	
}

void APSCharacter::StartBoosting()
{
	if (IsLocallyControlled() && Buff && Stamina > 10.f)
	{
		Buff->DrainingStamina(100, StaminaDrainSpeed);
		SetBoosting(true);
		FireButtonReleased();
		bIsBoosting = true;
		Server_IsBoosting(true);
	}
}

void APSCharacter::BoostButtonReleased()
{
	if (!bToggleBoost)
	{
		StopBoosting();
	}
}

void APSCharacter::StopBoosting()
{
	if (Buff)
	{
		Buff->ReplenishStamina(100, StaminaReplenishSpeed);
		SetBoosting(false);
		bIsBoosting = false;
		Server_IsBoosting(false);
	}
}

void APSCharacter::Server_IsBoosting_Implementation(bool Boosting)
{
	bIsBoosting = Boosting;
}

void APSCharacter::SpawnDefaultWeapon()
{
	APSGameMode* PSGameMode = Cast<APSGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (PSGameMode && World && DefaultWeaponClass && !bElimmed)	{		

		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
		
		if (DefaultSecondaryWeaponClass)
		{
			AWeapon* StartingSecondaryWeapon = World->SpawnActor<AWeapon>(DefaultSecondaryWeaponClass);
			Combat->EquipWeapon(StartingSecondaryWeapon);
		}

		if (MountedWeaponClass)
		{
			AWeapon* MountedWeapon = World->SpawnActor<AWeapon>(MountedWeaponClass);
			Combat->EquipMountedWeapon(MountedWeapon);
		}
	}	
}

void APSCharacter::FireButtonPressed()
{
	if (bIsBoosting) return;
	if (bIsFlying && !bTransitioningfromFlight) return;
	if (Combat && !bElimmed)
	{
		Combat->FireButtonPressed(true);
	}
}

void APSCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void APSCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APSCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
				break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rifle");
				break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;

		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APSCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
		
	}
}

void APSCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr || Combat->CombatState != ECombatState::ECS_Unoccupied) return;	

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);		
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
	
	
}

FVector APSCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->TraceTargetForWeaponRotation;
}

void APSCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}	
}

void APSCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();		
	}
}

void APSCharacter::OnRep_Stamina(float LastStamina)
{
	UpdateHUDStamina();
	
}

void APSCharacter::UpdateHUDHealth()
{
	PSPlayerController = PSPlayerController == nullptr ? Cast<APSPlayerController>(Controller) : PSPlayerController;
	if (PSPlayerController)
	{
		PSPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void APSCharacter::UpdateHUDShield()
{
	
	PSPlayerController = PSPlayerController == nullptr ? Cast<APSPlayerController>(Controller) : PSPlayerController;
	if (PSPlayerController)
	{
		PSPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void APSCharacter::UpdateHUDStamina()
{
	PSPlayerController = PSPlayerController == nullptr ? Cast<APSPlayerController>(Controller) : PSPlayerController;
	if (PSPlayerController)
	{
		PSPlayerController->SetHUDStamina(Stamina, MaxStamina);
	}
}

void APSCharacter::UpdateHUDAmmo()
{
	PSPlayerController = PSPlayerController == nullptr ? Cast<APSPlayerController>(Controller) : PSPlayerController;
	if (PSPlayerController && Combat && Combat->EquippedWeapon)
	{
		PSPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		PSPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void APSCharacter::RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{	
	if (bElimmed) return;

	float DamageToHealth = Damage;
	if (Shield > 0)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0, Damage);
			Shield = 0.f;
		}
	}
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	GetWorldTimerManager().ClearTimer(HitTimer);
	GetWorldTimerManager().SetTimer(
		HitTimer,
		this,
		&APSCharacter::HitTimerFinished,
		HitDelay);
	
	if (Buff) Buff->StopReplenishingShield();
	
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();
	AimButtonReleased();
	if (Health == 0.f)
	{		
		
		APSGameMode* PSGameMode = GetWorld()->GetAuthGameMode<APSGameMode>();
		if (PSGameMode)
		{
			PSPlayerController = PSPlayerController == nullptr ? Cast<APSPlayerController>(Controller) : PSPlayerController;
			AttackerController = Cast<APSPlayerController>(InstigatorController);
			PSGameMode->PlayerEliminated(this, PSPlayerController, AttackerController);	
			if (AttackerController->PSCharacter)
			{
				AttackerController->PSCharacter->PlayKillSound();
			}
		}
	}
	
}

void APSCharacter::HitTimerFinished()
{
	Buff->ReplenishShield(100, 5);
}

void APSCharacter::Elim()
{
	if (Combat )
	{
		if (Combat->EquippedWeapon)
		{
			Combat->EquippedWeapon->Dropped();
		}
		if (Combat->SecondaryWeapon)
		{
			Combat->SecondaryWeapon->Dropped();
		}
		if (Combat && Combat->MountedWeapon)
		{
			Combat->MountedWeapon->Destroy();
		}
		
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&APSCharacter::ElimTimerFinished,
		ElimDelay
	);	
}

void APSCharacter::PlayKillSound_Implementation()
{
	if (KillSound && IsLocallyControlled())
	{
		UGameplayStatics::PlaySound2D(this, KillSound);
	}
	
}

void APSCharacter::Destroyed()
{
	Super::Destroyed();

	
}

void APSCharacter::ElimTimerFinished()
{
	APSGameMode* PSGameMode = GetWorld()->GetAuthGameMode<APSGameMode>();
	if (PSGameMode)
	{
		PSGameMode->RequestRespawn(this, Controller);
	}
}

void APSCharacter::MulticastElim_Implementation()
{		
	
	if (DissolveMaterialInstance)
	{		
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), DissolveGlowAmount);
	}

	if (DissolveMaterialInstance1)
	{
		DynamicDissolveMaterialInstance1 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance1, this);

		GetMesh()->SetMaterial(1, DynamicDissolveMaterialInstance1);
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Glow"), DissolveGlowAmount);
	}
	if (PSPlayerController)
	{
		PSPlayerController->SetHUDWeaponAmmo(0);
	}
	StartDissolve();
	bElimmed = true;
	FireButtonReleased();
	SetCanMove(false);
	SetCollisionsAfterElimmed();
	if (PSPlayerController)
	{
		DisableInput(PSPlayerController);
	}	
	bool bHideSniperScope = IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	GetMesh()->AddImpulse(Impulse);
	
}

void APSCharacter::MulticastShieldRecharge_Implementation()
{
	if (RechargeShieldSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			RechargeShieldSound,
			GetActorLocation()
		);
	}
}

void APSCharacter::SetCollisionsAfterElimmed()
{
	GetCameraBoom()->bInheritPitch = false;
	GetCameraBoom()->bInheritRoll = false;
	GetCameraBoom()->bInheritYaw = false;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
	//GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetSimulatePhysics(true);	
	GetBox()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APSCharacter::UpdateDisolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
	if (DynamicDissolveMaterialInstance1)
	{
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void APSCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &APSCharacter::UpdateDisolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void APSCharacter::ReloadButtonPressed()
{
	if (Combat)
	{
		Combat->Reload();
	}
}

ECombatState APSCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

void APSCharacter::GrenadeButtonPressed()
{
	if (Combat && !bIsFlying)
	{
		Combat->ThrowGrenade();
	}
}

void APSCharacter::EnterFlight()
{
	
}

void APSCharacter::MeleeButtonPressed()
{
	if (bIsFlying)
	{
		JumpButtonPressed();
	}
	else if (Combat && Combat->EquippedWeapon)
	{
		Combat->Melee();
	}
}