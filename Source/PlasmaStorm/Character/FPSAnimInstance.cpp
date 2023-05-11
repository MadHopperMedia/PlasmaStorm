// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSAnimInstance.h"
#include "PSCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlasmaStorm/Weapon/weapon.h"
#include "PlasmaStorm/PSTypes/CombatState.h"

void UFPSAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PSCharacter = Cast<APSCharacter>(TryGetPawnOwner());
}

void UFPSAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (PSCharacter == nullptr)
	{
		PSCharacter = Cast<APSCharacter>(TryGetPawnOwner());
	}

	if (PSCharacter == nullptr) return;
	FVector Velocity = PSCharacter->Speed;
	Speed = Velocity.Size();
	bIsGrounded = PSCharacter->bIsGrounded;	
	bIsAccelerating = PSCharacter->GetbIsAccelerating();
	bWeaponEquipped = PSCharacter->IsWeaponEquipped();
	EquippedWeapon = PSCharacter->GetEquippedWeapon();
	bIsCrouched = PSCharacter->GetIsCrouched();
	bAiming = PSCharacter->IsAiming();
	bIsHovering = PSCharacter->GetIsFlying();
	bIsFlying = PSCharacter->GetIsFlyingForward();
	bIsIdoling = PSCharacter->GetIsIdoling();
	bHoldingTheFlag = PSCharacter->IsHoldingThFlag();
	bIsBoosting = (PSCharacter->GetIsBoosting() && Speed > 100);
	bIsSliding = PSCharacter->GetIsSliding();
	
	if (EquippedWeapon && (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun))
	{
		bUsePistolStance = true;
	}
	else
	{
		bUsePistolStance = false;
	}


	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && PSCharacter->GetFPSMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		PSCharacter->GetFPSMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), LeftHandTransform.GetRotation().Rotator(), OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (PSCharacter->IsLocallyControlled() && PSCharacter->GetHitTarget() != FVector::ZeroVector)
		{
			FTransform RightHandTransform = PSCharacter->GetFPSMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - PSCharacter->GetHitTarget()));
			RightHandRotation = LookAtRotation;   //FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 100.f);
		}
	}
	
	bUseFABRIK = PSCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
}