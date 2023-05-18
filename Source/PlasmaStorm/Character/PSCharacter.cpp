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
#include "PlasmaStorm/PSComponents/LagCompensationComponent.h"
#include "PlasmaStorm/PlayerStart/TeamPlayerStart.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"


APSCharacter::APSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	FPSMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSMesh"));
	FPSMesh->SetupAttachment(GetFollowCamera());
	FPSMesh->bOnlyOwnerSee = true;
	FPSMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FPSMesh->SetVisibility(false);

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));
	

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/**
	* Hit boxes for server side rewind
	*/

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	spine_05 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_05"));
	spine_05->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_05"), spine_05);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("hethigh_lad"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));	
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);
	
	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
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
	PlayerInputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &APSCharacter::SwitchWeaponButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APSCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APSCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APSCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APSCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APSCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("Reload", IE_Released, this, &APSCharacter::ReloadButtonReleased);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &APSCharacter::GrenadeButtonPressed);
	PlayerInputComponent->BindAction("Boost", IE_Pressed, this, &APSCharacter::BoostButtonPressed);
	PlayerInputComponent->BindAction("Boost", IE_Released, this, &APSCharacter::BoostButtonReleased);
	PlayerInputComponent->BindAction("Melee", IE_Pressed, this, &APSCharacter::MeleeButtonPressed);
	PlayerInputComponent->BindAction("DodgeRight", IE_Pressed, this, &APSCharacter::DodgeRightButtonPressed);
	PlayerInputComponent->BindAction("DodgeLeft", IE_Pressed, this, &APSCharacter::DodgeLeftButtonPressed);
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
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<APSPlayerController>(Controller);
		}
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
	DOREPLIFETIME(APSCharacter, Stamina);
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
		bCanBoostJump = false;
		
	}
	else
	{
		CTFComponent->bCanFly = true;
		bCanBoostJump = true;
	}
	if (Combat && Combat->MountedWeapon)
	{
		if (!bIsGrounded)
		{
			Combat->MountedWeapon->bCanRecharge = false;
		}
		else
		{
			Combat->MountedWeapon->bCanRecharge = true;
		}
	}
	if (Combat && Combat->EquippedWeapon && IsHoldingThFlag())
	{
		Combat->EquippedWeapon->SetActorHiddenInGame(true);
	}
	else if (Combat && Combat->EquippedWeapon && !GetIsFlying())
	{
		Combat->EquippedWeapon->SetActorHiddenInGame(false);
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
			bUseFirstPerson = PSPlayerController->bUseFPS;			
		}
	}
	

	if (PSPlayerState == nullptr)
	{
		PSPlayerState = GetPlayerState<APSPlayerState>();
		if (PSPlayerState)
		{
			OnPlayerStateInitialized();
		}
	}
}

void APSCharacter::OnPlayerStateInitialized()
{
	PSPlayerState->AddToScore(0.f);
	PSPlayerState->AddToDefeats(0);
	SetTeamColor(PSPlayerState->GetTeam());
	SetSpawnPoint();
}

void APSCharacter::SetSpawnPoint()
{
	if (HasAuthority() && PSPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for (auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if (TeamStart && TeamStart->Team == PSPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		if (TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(
				ChosenPlayerStart->GetActorLocation(),
				ChosenPlayerStart->GetActorRotation()
			);
		}
		
	}
}

void APSCharacter::HideCharacterIfCharacterClose()
{
	bFPSView = (FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < FPSCameraThreshold;
	if ( Combat == nullptr) return;
	if (bFPSView && IsLocallyControlled())
	{
		GetMesh()->SetVisibility(false);
		GetFPSMesh()->SetVisibility(true);
		if (Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh() && Combat->EquippedWeapon->GetWeaponState() != EWeaponState::EWS_Dropped)
		{
			const USkeletalMeshSocket* FPS_Socket = GetFPSMesh()->GetSocketByName(FName("FPS_Socket"));
			bool bUseFirstPersonMesh = IsLocallyControlled();
			if (!IsElimmed())
			{
				if (FPS_Socket && bUseFirstPersonMesh && Combat->CombatState == ECombatState::ECS_Unoccupied)
				{
					FPS_Socket->AttachActor(Combat->EquippedWeapon, GetFPSMesh());
				}
			}
			
		}
		if (Combat->MountedWeapon && Combat->MountedWeapon->GetWeaponMesh() && Combat->MountedWeapon->GetWeaponMesh()->bOwnerNoSee == false)
		{			
			Combat->MountedWeapon->GetWeaponMesh()->bOwnerNoSee = true;			
		}		
	}
	else 
	{		
		GetFPSMesh()->SetVisibility(false);
		if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() > CameraThreshold)
		{
			GetMesh()->SetVisibility(true);
			if (Combat && Combat->MountedWeapon && Combat->MountedWeapon->GetWeaponMesh())
			{
				Combat->MountedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
				
			}
		}
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh() && Combat->EquippedWeapon->GetWeaponState() != EWeaponState::EWS_Dropped)
		{			
			const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
			bool bUseFirstPersonMesh = IsLocallyControlled();
			if (!IsElimmed())
			{
				if (HandSocket && bUseFirstPersonMesh || !bUseFirstPersonMesh)
				{
					if (Combat->CombatState != ECombatState::ECS_Unoccupied) return;
					HandSocket->AttachActor(Combat->EquippedWeapon, GetMesh());
				}
			}
			
		}		
	}
	if (bIsFlying || IsHoldingThFlag())
	{
		GetFPSMesh()->SetVisibility(false);
	}	
}

void APSCharacter::ForwardMovement(float Val)
{
	if (IsHoldingThFlag())
	{
		if (GetIsFlying())
		{
			Val = Val * .1f;
		}
		else
		{
			Val = Val * .5f;
		}
		
	}
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

void APSCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr || Material == nullptr || Material1 == nullptr || RedMaterial == nullptr || RedMaterial1 == nullptr || BlueMaterial == nullptr || BlueMaterial1 == nullptr) return;
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, Material);
		GetMesh()->SetMaterial(1, Material1);
		GetFPSMesh()->SetMaterial(0, Material);
		GetFPSMesh()->SetMaterial(1, Material1);
		break;
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		GetMesh()->SetMaterial(1, RedMaterial1);
		GetFPSMesh()->SetMaterial(0, RedMaterial);
		GetFPSMesh()->SetMaterial(1, RedMaterial1);
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		GetMesh()->SetMaterial(1, BlueMaterial1);
		GetFPSMesh()->SetMaterial(0, BlueMaterial);
		GetFPSMesh()->SetMaterial(1, BlueMaterial1);
		break;
	}
}

void APSCharacter::PlayerYaw(float Val)
{
	if (IsLocallyControlled())
	{
		Val = FMath::Clamp(Val, -2, 2);
		if (bIsBoosting)
		{
			Val = FMath::Clamp(Val, -.6, .6);
		}
		if (bIsFlying && bTransitioningfromFlight)
		{
			Val = FMath::Clamp(Val, -1, 1);
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
	if (IsHoldingThFlag()) return;
	Crouch();
	StopBoosting();
	bIsBoosting = false;
}

void APSCharacter::JumpButtonPressed()
{
	if (Combat && Combat->bAiming) return;
	
	Jump();
}

AWeapon* APSCharacter::GetEquippedWeapons()
{
	if (Combat && Combat->EquippedWeapon)
	{
		return Combat->EquippedWeapon;
	}
	else
	{
		return nullptr;
	}
}

AWeapon* APSCharacter::GetSecondaryWeapons()
{
	if (Combat && Combat->SecondaryWeapon)
	{
		return Combat->SecondaryWeapon;
	}
	else
	{
		return nullptr;
	}
}

AWeapon* APSCharacter::GetMountedWeapons()
{
	if (Combat && Combat->MountedWeapon)
	{
		return Combat->MountedWeapon;
	}
	else
	{
		return nullptr;
	}
}

void APSCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	OverlappingWeapon = Weapon;	
}

void APSCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	
}

void APSCharacter::SwitchWeaponButtonPressed()
{
	if (bIsFlying || Combat == nullptr || !Combat->EquippedWeapon|| IsHoldingThFlag()) return;
		ServerSwapWeaponsButtonPressed();
		if (!HasAuthority() && Combat->CombatState == ECombatState::ECS_Unoccupied)
		{
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}	

		if (Combat->bAiming)
		{
			Combat->SetAiming(false);
		}		
}

void APSCharacter::ServerSwapWeaponsButtonPressed_Implementation()
{
	if (Combat && HasAuthority())
	{
		Combat->SwapWeapons();
	}
}

AWeapon* APSCharacter::GetMountedWeapon()
{
	if (Combat && Combat->MountedWeapon)
	{
		return Combat->MountedWeapon;
	}
	return nullptr;
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
	if (Combat && IsHoldingThFlag())
	{
		DropFlag();
		return;
	}
	if (Combat && !bIsFlying && !IsHoldingThFlag() && Combat->EquippedWeapon && Combat->EquippedWeapon->GetCanZoom())
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
	if (IsHoldingThFlag()) return;
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
	if (PSGameMode && World && DefaultWeaponClass && !bElimmed)	
	{		

		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
			//StartingWeapon->GetWeaponMesh()->SetVisibility(false);
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
	if (Combat && IsHoldingThFlag())
	{
		MeleeButtonPressed();
		return;
	}
	if (bIsBoosting || bIsFlying && !bTransitioningfromFlight || IsHoldingThFlag()) return;
	if (Combat && !bElimmed)
	{
		Combat->FireButtonPressed(true);
	}
}

void APSCharacter::DropFlag()
{
	if (Combat->TheFlag)
	{
		Combat->DropFlag();
		Combat->bHoldingTheFlag = false;
	}
	ServerDropFlag();
}

void APSCharacter::ServerDropFlag_Implementation()
{
	
	if (Combat)
	{
		Combat->DropFlag();
		Combat->bHoldingTheFlag = false;
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
	UAnimInstance* FPSAnimInstance = GetFPSMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
	if (FPSAnimInstance && FireWeaponMontage)
	{
		//FPSAnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		FPSAnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APSCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimInstance* FPSAnimInstance = GetFPSMesh()->GetAnimInstance();
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
	if (FPSAnimInstance && ReloadMontage)
	{
		FPSAnimInstance->Montage_Play(ReloadMontage);
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
		FPSAnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APSCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);		
	}
	UAnimInstance* FPSAnimInstance = GetFPSMesh()->GetAnimInstance();
	if (FPSAnimInstance && ThrowGrenadeMontage)
	{
		FPSAnimInstance->Montage_Play(ThrowGrenadeMontage);
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
	if (IsLocallyControlled())
	{
		AimButtonReleased();
	}
}

void APSCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimInstance* FPSAnimInstance = GetFPSMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
	if (FPSAnimInstance && SwapMontage)
	{
		FPSAnimInstance->Montage_Play(SwapMontage);
		
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
			if (AttackerController && AttackerController->PSCharacter)
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

void APSCharacter::Elim(bool bPlayerLeftGame)
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
		if (Combat->TheFlag)
		{
			Combat->TheFlag->Dropped();
		}
		
	}
	MulticastElim(bPlayerLeftGame);
	WasElimmed();
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
	if (PSGameMode && !bLeftGame)
	{
		PSGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void APSCharacter::ServerLeaveGame_Implementation()
{
	APSGameMode* PSGameMode = GetWorld()->GetAuthGameMode<APSGameMode>();
	PSPlayerState = PSPlayerState == nullptr ? GetPlayerState<APSPlayerState>() : PSPlayerState;
	if (PSGameMode && PSPlayerState)
	{
		PSGameMode->PlayerLeftGame(PSPlayerState);
	}
}

void APSCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{		
	bLeftGame = bPlayerLeftGame;
	WasElimmed();
	if (Material)
	{		
		//DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(0), this);
		

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), DissolveGlowAmount);
	}

	if (Material1)
	{
		//DynamicDissolveMaterialInstance1 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance1, this);
		DynamicDissolveMaterialInstance1 = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(1), this);

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
	if (!bLeftGame)
	{
		SetCollisionsAfterElimmed();
		GetMesh()->AddImpulse(Impulse);
	}
	
	if (PSPlayerController)
	{
		DisableInput(PSPlayerController);
	}	
	bool bHideSniperScope = IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&APSCharacter::ElimTimerFinished,
		ElimDelay
	);	
	
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
	if (Combat && Combat->CombatState == ECombatState::ECS_Unoccupied && !bIsFlying)
	{
		if (Combat->bHoldingTheFlag) return;
		if (OverlappingWeapon)
		{
			EquippingWeapon = true;
			GetWorldTimerManager().SetTimer(
				EquipTimer,
				this,
				&APSCharacter::EquipTimerFinished,
				EquipDelay);
			return;

		}
		else
		{
			Combat->Reload();
		}
	}	
}

void APSCharacter::EquipTimerFinished()
{
	if (Combat && Combat->CombatState == ECombatState::ECS_Unoccupied && !bIsFlying)
	{		
		EquippingWeapon = false;
		ServerEquipButtonPressed(true);
	}
}

void APSCharacter::ServerEquipButtonPressed_Implementation(bool IsEquipingWeapon)
{
	EquippingWeapon = IsEquipingWeapon;
	if (Combat && HasAuthority())
	{
		if (OverlappingWeapon && EquippingWeapon == true)
		{
			EquippingWeapon = false;
			Combat->EquipWeapon(OverlappingWeapon);
		}
	}
}

void APSCharacter::ReloadButtonReleased()
{
	if (EquippingWeapon && Combat && Combat->CombatState == ECombatState::ECS_Unoccupied && !bIsFlying)
	{
		GetWorldTimerManager().ClearTimer(EquipTimer);
		ServerEquipButtonPressed(false);
		EquippingWeapon = false;
	}
}

void APSCharacter::DodgeRightButtonPressed()
{	
	if (CTFComponent == nullptr || !bIsFlying || Stamina <= 40 || !GetIsFlyingForward()) return;
	bPrimedToDodgeLeft = false;
	
	if (bPrimedToDodgeRight)
	{
		
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && FireWeaponMontage && IsLocallyControlled())
		{
			AnimInstance->Montage_Play(DodgeMontage);
			FName SectionName;
			SectionName = FName("DodgeRight");
			AnimInstance->Montage_JumpToSection(SectionName);		
		}
		bPrimedToDodgeRight = false;
		Stamina = Stamina - 40;
		StopBoosting();
		if (HasAuthority())
		{
			MulticastDodge("DodgeRight");
		}
		else
		{
			ServerDodge("DodgeRight");
		}
	}
	else
	{
		bPrimedToDodgeRight = true;
		StartDodgeTimer();
	}
}

void APSCharacter::DodgeLeftButtonPressed()
{
	if (CTFComponent == nullptr || !bIsFlying || Stamina <= 40 || !GetIsFlyingForward()) return;
	bPrimedToDodgeRight = false;

	if (bPrimedToDodgeLeft)
	{			
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && FireWeaponMontage && IsLocallyControlled())
		{
			AnimInstance->Montage_Play(DodgeMontage);
			FName SectionName;
			SectionName = FName("DodgeLeft");
			AnimInstance->Montage_JumpToSection(SectionName);
		}
		bPrimedToDodgeRight = false;
		Stamina = Stamina - 40;
		StopBoosting();
		if (HasAuthority())
		{
			MulticastDodge("DodgeLeft");
		}
		else
		{
			ServerDodge("DodgeLeft");
		}
	}
	else
	{
		bPrimedToDodgeLeft = true;
		StartDodgeTimer();
	}
}

void APSCharacter::StartDodgeTimer()
{
	// ToDo Clear Timer
	GetWorldTimerManager().SetTimer(
		DodgeButtonTimer,
		this,
		&APSCharacter::DodgeButtonTimerFinished,
		.2f
	);
}

void APSCharacter::DodgeButtonTimerFinished()
{
	bPrimedToDodgeRight = false;
	bPrimedToDodgeLeft = false;
}

void APSCharacter::PlayerDodge(FVector DodgeDirection)
{
	if (IsLocallyControlled())
	{
		Dodge(DodgeDirection);
	}	
}

void APSCharacter::ServerDodge_Implementation(FName Section)
{
	MulticastDodge(Section);
}

void APSCharacter::MulticastDodge_Implementation(FName Section)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage && !IsLocallyControlled())
	{
		AnimInstance->Montage_Play(DodgeMontage);
		FName SectionName;
		SectionName = FName("Section");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

ECombatState APSCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

void APSCharacter::GrenadeButtonPressed()
{
	if (Combat && !bIsFlying && !IsHoldingThFlag())
	{
		Combat->ThrowGrenade();
	}
}

void APSCharacter::MeleeButtonPressed()
{
	if (bIsFlying)
	{
		JumpButtonPressed();
		return;
	}
	if (Combat == nullptr || Combat->CombatState != ECombatState::ECS_Unoccupied) return;
	PlayMeleeMontage();
	
	if (Combat->TargetCharacter != nullptr)
	{
		FVector DistanceToTarget = GetActorLocation() - Combat->TargetCharacter->GetActorLocation();
		if (DistanceToTarget.Size() < 200 && DistanceToTarget.Size() > 50)
		{			
			SetActorLocation(GetActorLocation() + GetActorForwardVector() * (DistanceToTarget.Size() - 50));
		}
	}
}



void APSCharacter::PlayMeleeMontage()
{
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();	
	UAnimInstance* FPSAnimInstance = GetFPSMesh()->GetAnimInstance();
	if (MeleeMontage && AnimInstance && Combat && Combat->EquippedWeapon)
	{	
		Combat->CombatState = ECombatState::ECS_Melee;
		AnimInstance->Montage_Play(MeleeMontage);
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
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Rifle");
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
		if (IsHoldingThFlag())
		{
			SectionName = FName("Flag");
		}
		AnimInstance->Montage_JumpToSection(SectionName);
		if (HasAuthority())
		{
			MultiPlayMeleeMontage();
		}
		else
		{
			ServerPlayMeleeMontage();
		}
		if (FPSAnimInstance && IsLocallyControlled())
		{
			FPSAnimInstance->Montage_Play(MeleeMontage);
			FPSAnimInstance->Montage_JumpToSection(SectionName);
		}
	}
	

}

void APSCharacter::ServerPlayMeleeMontage_Implementation()
{
	if (IsLocallyControlled() || Combat == nullptr || !Combat->EquippedWeapon) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimInstance* FPSAnimInstance = GetFPSMesh()->GetAnimInstance();
	if (AnimInstance && MeleeMontage)
	{
		
		AnimInstance->Montage_Play(MeleeMontage);
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
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Rifle");
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
		if (IsHoldingThFlag())
		{
			SectionName = FName("Flag");
		}
		AnimInstance->Montage_JumpToSection(SectionName);
		MultiPlayMeleeMontage();
		if (FPSAnimInstance && IsLocallyControlled())
		{
			FPSAnimInstance->Montage_Play(MeleeMontage);
			FPSAnimInstance->Montage_JumpToSection(SectionName);
		}		
	}
}

void APSCharacter::MultiPlayMeleeMontage_Implementation()
{
	if (IsLocallyControlled() || Combat == nullptr || !Combat->EquippedWeapon) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && MeleeMontage)
	{
		
		AnimInstance->Montage_Play(MeleeMontage);
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
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Rifle");
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
		if (IsHoldingThFlag())
		{
			SectionName = FName("Flag");
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}	
}

void APSCharacter::EnableWeaponMeleeHitbox()
{
	if (IsHoldingThFlag() && Combat && Combat->TheFlag)
	{
		Combat->TheFlag->EnableHitBox();
	}
	else if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->EnableHitBox();
	}
}

void APSCharacter::DisableWeaponMeleeHitbox()
{
	if (IsHoldingThFlag() && Combat && Combat->TheFlag)
	{
		Combat->TheFlag->DisableHitBox();		
	}
	else if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->DisableHitBox();		
	}
	Combat->CombatState = ECombatState::ECS_Unoccupied;	
}

bool APSCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

bool APSCharacter::IsHoldingThFlag() const
{
	if (Combat == nullptr) return false;
	return Combat->bHoldingTheFlag;
}

void APSCharacter::SetHoldingTheFalg(bool bHolding)
{
	if (Combat == nullptr) return;

	Combat->bHoldingTheFlag = bHolding;
}	

ETeam APSCharacter::GetTeam()
{
	PSPlayerState = PSPlayerState == nullptr ? GetPlayerState<APSPlayerState>() : PSPlayerState;
	if (PSPlayerState == nullptr) return ETeam::ET_NoTeam;
	{
		return PSPlayerState->GetTeam();
	}

}