// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(FlagMesh);
	GetAreaSphere()->SetupAttachment(FlagMesh);
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();

	OriginalTransform = GetActorTransform();
}

void AFlag::Dropped()
{
	
	SetWeaponState(EWeaponState::EWS_Dropped);
	//GetWeaponMesh()->SetVisibility(true);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	PSOwnerCharacter = nullptr;
	PSOwnerController = nullptr;
	GetWorldTimerManager().SetTimer(
		DroppedTimer,
		this,
		&AFlag::DroppedTimerFinished,
		DroppedDelay
	);
}

void AFlag::DroppedTimerFinished()
{
	SetActorTransform(OriginalTransform);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}

void AFlag::OnEquipped()
{
	
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	//FlagMesh->SetVisibility(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	

	GetWorldTimerManager().ClearTimer(DroppedTimer);

	
}

void AFlag::OnDropped()
{
	
	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	//FlagMesh->SetVisibility(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

}