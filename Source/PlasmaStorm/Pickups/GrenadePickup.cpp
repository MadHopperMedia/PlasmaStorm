// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadePickup.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "PlasmaStorm/PSComponents/CombatComponent.h"

void AGrenadePickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	APSCharacter* PSCharacter = Cast<APSCharacter>(OtherActor);
	if (PSCharacter )
	{
		UCombatComponent* Combat = PSCharacter->GetCombat();
		if (Combat && Combat->GetGrenades() < Combat->GetMaxGrenades())
		{
			Combat->PickupGrenade(GrenadeAmount);

		}
		else
		{
			return;
		}
	}
	Destroy();
}