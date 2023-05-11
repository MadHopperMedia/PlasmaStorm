// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"

AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(FlagMesh);
	GetAreaSphere()->SetupAttachment(FlagMesh);
	MeleeHitBox->SetupAttachment(RootComponent);
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();

	OriginalTransform = GetActorTransform();
	ReturnDelay = DroppedDelay;
}

void AFlag::Dropped()
{
	
	SetWeaponState(EWeaponState::EWS_Dropped);	
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	PSOwnerCharacter = nullptr;
	PSOwnerController = nullptr;
	GetWorldTimerManager().SetTimer(
		DroppedTimer,
		this,
		&AFlag::DroppedTimerFinished,
		ReturnDelay
	);
}

void AFlag::DroppedTimerFinished()
{	
	ReturnFlag();
}

void AFlag::SetReturnTimer()
{
	ReturnDelay = 0.001;

}

void AFlag::ReturnFlag()
{
	if (!HasAuthority()) return;
	FVector Location = GetOriginalTransform().GetLocation();
	FRotator Rotation = GetOriginalTransform().Rotator();
	FActorSpawnParameters SpawnParams;
	UWorld* World = GetWorld();
	if (World)
	{
		AFlag* NewFlag = World->SpawnActor<AFlag>(FlagClass, Location, Rotation, SpawnParams);
	}	
	Destroy();
}

void AFlag::ServerReturnFlag_Implementation()
{
	
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTransform(OriginalTransform);
}

void AFlag::OnEquipped()
{
	
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);	
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	

	GetWorldTimerManager().ClearTimer(DroppedTimer);

	
}

void AFlag::OnDropped()
{
	
	
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

}