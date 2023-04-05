// Fill out your copyright notice in the Description page of Project Settings.


#include "PSCharacterAnimInstance.h"
#include "PSCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlasmaStorm/Weapon/weapon.h"
#include "PlasmaStorm/PSTypes/CombatState.h"


void UPSCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PSCharacter = Cast<APSCharacter>(TryGetPawnOwner());
}

void UPSCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
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
	//bIsAccelerating = PSCharacter->ForwardVal > 0.f ? true : false;
	bIsAccelerating = PSCharacter->GetbIsAccelerating();
	bWeaponEquipped = PSCharacter->IsWeaponEquipped();
	EquippedWeapon = PSCharacter->GetEquippedWeapon();
	bIsCrouched = PSCharacter->GetIsCrouched();
	bAiming = PSCharacter->IsAiming();
	bIsHovering = PSCharacter->GetIsFlying();
	bIsFlying = PSCharacter->GetIsFlyingForward();
	bIsIdoling = PSCharacter->GetIsIdoling();
	bIsBoosting = (PSCharacter->GetIsBoosting() && Speed > 100);
	TurningInPlace = PSCharacter->GetTurningInPlace();
	FlyingRotation = PSCharacter->GetFlyingRotation();

	// Offset Yaw for Strafing
	FRotator AimRotation = PSCharacter->GetActorRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PSCharacter->Speed);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = PSCharacter->GetBaseAimRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
	Ao_Pitch = FMath::FInterpTo(Ao_Pitch, PSCharacter->GetAo_Pitch(), DeltaTime, 20.f);
	Ao_Yaw = PSCharacter->GetAo_Yaw();
	Flying_PitchOffset = PSCharacter->GetFlying_PitchOffset();
	Flying_RollOffset = PSCharacter->GetFlying_RollOffset();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && PSCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		PSCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), LeftHandTransform.GetRotation().Rotator(), OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		
		if (PSCharacter->IsLocallyControlled() && PSCharacter->GetHitTarget() != FVector::ZeroVector)
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = PSCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - PSCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
			
		}
	}

	bUseFABRIK = PSCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
}