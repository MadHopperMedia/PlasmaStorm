// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "PlasmaStorm/PSComponents/BuffComponent.h"


AHealthPickup::AHealthPickup()
{
	bReplicates = true;
	
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);	

	APSCharacter* PSCharacter = Cast<APSCharacter>(OtherActor);
	if (PSCharacter && PSCharacter->GetHealth() < PSCharacter->GetMaxHealth())
	{
		UBuffComponent* Buff = PSCharacter->GetBuff();
		if (Buff)
		{
			Buff->Heal(HealAmount, HealingTime);
		}
	}
	else
	{
		return;
	}
	Destroy();
}

