// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldPickup.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "PlasmaStorm/PSComponents/BuffComponent.h"

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{


	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	APSCharacter* PSCharacter = Cast<APSCharacter>(OtherActor);
	if (PSCharacter)
	{
		UBuffComponent* Buff = PSCharacter->GetBuff();
		if (Buff)
		{
			Buff->ReplenishShield(ShieldAmount, ShieldTime);
		}
	}
	else
	{
		return;
	}
	Destroy();
}