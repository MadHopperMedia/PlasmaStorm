// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "PlasmaStorm/PlayerController/PSPlayerController.h"
#include "Animation/AnimationAsset.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Casing.h"

// Sets default values
AWeapon::AWeapon()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));	
	SetRootComponent(WeaponMesh);
	WeaponMesh->bOwnerNoSee = true;
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

	
}

 void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	 Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	 DOREPLIFETIME(AWeapon, WeaponState);
	 DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);	 
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	AddedAmmo = Ammo;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RecharageAmmo(DeltaTime);
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();	
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_EquippedMountedWeapon:
		OnEquippedMountedWeapon();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{	
	OnWeaponStateSet();
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetVisibility(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	/*if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->WakeAllRigidBodies();
	}*/

	GetWorldTimerManager().ClearTimer(DroppedTimer);

	PSOwnerCharacter = PSOwnerCharacter == nullptr ? Cast<APSCharacter>(GetOwner()) : PSOwnerCharacter;
	if (PSOwnerCharacter && bUseServerSideRewind)
	{
		PSOwnerController = PSOwnerController == nullptr ? Cast<APSPlayerController>(PSOwnerCharacter->Controller) : PSOwnerController;
		if (PSOwnerController && HasAuthority() && !PSOwnerController->HighPingDelegate.IsBound())
		{
			PSOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetVisibility(false);
	/*if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->WakeAllRigidBodies();
	}*/
	
	GetWorldTimerManager().ClearTimer(DroppedTimer);

	/*PSOwnerCharacter = PSOwnerCharacter == nullptr ? Cast<APSCharacter>(GetOwner()) : PSOwnerCharacter;
	if (PSOwnerCharacter && bUseServerSideRewind)
	{
		PSOwnerController = PSOwnerController == nullptr ? Cast<APSPlayerController>(PSOwnerCharacter->Controller) : PSOwnerController;
		if (PSOwnerController && HasAuthority() && PSOwnerController->HighPingDelegate.IsBound())
		{
			PSOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}*/
}

void AWeapon::OnEquippedMountedWeapon()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}

void AWeapon::OnDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetVisibility(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APSCharacter* PSCharacter = Cast<APSCharacter>(OtherActor);
	if (PSCharacter)
	{	
		if (PSCharacter->IsHoldingThFlag()) return;
		if (PSCharacter->GetEquippedWeapons() && PSCharacter->GetEquippedWeapons()->GetWeaponName() != WeaponName && PSCharacter->GetSecondaryWeapons() && PSCharacter->GetSecondaryWeapons()->GetWeaponName() != WeaponName && PSCharacter->GetMountedWeapons() && PSCharacter->GetMountedWeapons()->GetWeaponName() != WeaponName)
		{
			if (WeaponType == EWeaponType::EWT_Flag && PSCharacter->GetTeam() == Team) return;
			PSCharacter->SetOverlappingWeapon(this);
		}		
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APSCharacter* PSCharacter = Cast<APSCharacter>(OtherActor);
	if (PSCharacter && GetOwner() == false)
	{
		if (PSCharacter->IsHoldingThFlag()) return;
		if (WeaponType == EWeaponType::EWT_Flag && PSCharacter->GetTeam() == Team) return;
		PSCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		//PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = GetWeaponMesh()->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh());
			
						
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()					
					);
			}			
		}
	}
	SpendRound();
}

void AWeapon::Dropped()
{

	SetWeaponState(EWeaponState::EWS_Dropped);
	GetWeaponMesh()->SetVisibility(true);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);	
	SetOwner(nullptr);
	PSOwnerCharacter = nullptr;
	PSOwnerController = nullptr;
	GetWorldTimerManager().SetTimer(
		DroppedTimer,
		this,
		&AWeapon::DroppedTimerFinished,
		DroppedDelay
	);
}

void AWeapon::DroppedTimerFinished()
{
	Destroy();
}

void AWeapon::RecharageAmmo(float DeltaTime)
{	
	PSOwnerCharacter = PSOwnerCharacter == nullptr ? Cast<APSCharacter>(GetOwner()) : PSOwnerCharacter;
	
	if (bIsMountedWeapon && bCanRecharge && Ammo < MagCapacity)
	{	
		if (PSOwnerCharacter)
		{
			AddedAmmo = AddedAmmo + 1 * DeltaTime;
			if (AddedAmmo >= RechargeTime)
			{
				//AddAmmo(1);
				Ammo = Ammo + 1;
				AddedAmmo = 0;
			}
		}
	}
}

void AWeapon::SetHUDAmmo()
{
	if (bCanRecharge) return;
	PSOwnerCharacter = PSOwnerCharacter == nullptr ? Cast<APSCharacter>(GetOwner()) : PSOwnerCharacter;
	if (PSOwnerCharacter)
	{
		PSOwnerController = PSOwnerController == nullptr ? Cast<APSPlayerController>(PSOwnerCharacter->Controller) : PSOwnerController;
		if (PSOwnerController)
		{
			PSOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SpendRound()
{	
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);	
	SetHUDAmmo();
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	PSOwnerCharacter = PSOwnerCharacter == nullptr ? Cast<APSCharacter>(GetOwner()) : PSOwnerCharacter;
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		PSOwnerCharacter = nullptr;
		PSOwnerController = nullptr;
	}
	else
	{
		PSOwnerCharacter = PSOwnerCharacter == nullptr ? Cast<APSCharacter>(Owner) : PSOwnerCharacter;
		if (PSOwnerCharacter && PSOwnerCharacter->GetEquippedWeapon() && PSOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}		
	}
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	float ScatterRadious = SphereRadious;
	if (bIsAiming)
	{
		ScatterRadious = SphereRadious / 2;
	}

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, ScatterRadious);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	/*
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadious, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLoc * WeaponRange / ToEndLoc.Size()), FColor::Cyan, true);
	*/
	return FVector(TraceStart + ToEndLoc * WeaponRange / ToEndLoc.Size());

}
