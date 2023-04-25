// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"
#include "PlasmaStorm/Weapon/flag.h"
#include "Components/SphereComponent.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "PlasmaStorm/GameMode/CaptureTheFlagGameMode.h"


AFlagZone::AFlagZone()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	SetRootComponent(ZoneSphere);

}


void AFlagZone::BeginPlay()
{
	Super::BeginPlay();	
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	if (OverlappingFlag)
	{
		APSCharacter* FlagBearer = Cast<APSCharacter>(OverlappingFlag->GetOwner());
		if (OverlappingFlag->GetTeam() != Team && FlagBearer->GetTeam() == Team)
		{
			ACaptureTheFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
			if (GameMode)
			{
				GameMode->FlagCaptured(OverlappingFlag, this);

				if (FlagBearer)
				{
					OverlappingFlag->SetReturnTimer();
					FlagBearer->DropFlag();

				}
				OverlappingFlag->SetReturnTimer();
				//OverlappingFlag->ReturnFlag();
				//ServerReturnFlag(FlagBearer, OverlappingFlag);
			}
		}
	}
	
}

void AFlagZone::ServerReturnFlag_Implementation(APSCharacter* Character, AFlag* Flag)
{
	if (Flag)
	{		
		Flag->ReturnFlag();
	}
}



