// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "PlasmaStorm/Weapon/Weapon.h"
#include "PlasmaStorm/PSComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	APSCharacter* PSCharacter = Cast<APSCharacter>(OtherActor);
	if (PSCharacter)
	{
		UCombatComponent* Combat = PSCharacter->GetCombat();

		if (Combat)
		{
			bool bHasWeapon = Combat->GetEquippedWeapon() && Combat->GetEquippedWeapon()->GetWeaponType() == WeaponType || Combat->GetSecondaryWeapon() && Combat->GetSecondaryWeapon()->GetWeaponType() == WeaponType;
			if (bHasWeapon && Combat->GetCurrentAmmoAmountForWeapon()[WeaponType] < Combat->GetMaxAmmoForWeapon(WeaponType))
			{
				Combat->PickupAmmo(WeaponType, AmmoAmount);
				Destroy();
			}
			else
			{
				return;
			}
		}		
	}	
}