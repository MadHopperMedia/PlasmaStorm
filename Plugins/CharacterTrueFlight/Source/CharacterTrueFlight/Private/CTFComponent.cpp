// Fill out your copyright notice in the Description page of Project Settings.


#include "CTFComponent.h"
#include "CTFPawn.h"
#include "GameFramework/Actor.h"
#include "CollisionShape.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UCTFComponent::UCTFComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;	
	
}

void UCTFComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	
	//DOREPLIFETIME_CONDITION(UCTFComponent, MovementState, COND_SkipOwner);
	DOREPLIFETIME(UCTFComponent, MovementState);
	

}

// Called when the game starts
void UCTFComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (PlayerPawn)
	{
		OriginialOrientation = PlayerPawn->GetActorRotation();		
	}
}

// Called every frame
void UCTFComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bCanMove)
	{
		AddGravity(DeltaTime);
		CalculateVelocity(DeltaTime);
		TraceForGround(DeltaTime);
		AddYawRotation(DeltaTime);
		AddPitchRotation(DeltaTime);
		AddRollRotation(DeltaTime);
		PlayerJump(DeltaTime);
		MoveCharacter(DeltaTime);
		AutoCorrectOrientation(DeltaTime);
		CalculateAnimationVelocity(DeltaTime);

		if (PlayerPawn)
		{

			PlayerCharacter = PlayerCharacter == nullptr ? Cast<ACTFPawn>(PlayerPawn) : PlayerCharacter;
			if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
			{
				if (ForwardInput > .2)
				{
					if (MovementState == EMovementState::EMS_Hovering)
					{
						SetMovementState(EMovementState::EMS_Flying);
					}
				}
				else if (ForwardInput <= 0.2f)
				{
					if (MovementState == EMovementState::EMS_Flying)
					{
						SetMovementState(EMovementState::EMS_Hovering);
					}
				}
			}
		}
	}	
	
	
}

void UCTFComponent::CalculateAnimationVelocity(float DeltaTime)
{
	if (PlayerPawn)
	{
		FTransform currenttransform;
		
		// Get the current time
		float CurrentTime = GetWorld()->GetTimeSeconds();

		// Get the current position of the object
		FVector CurrentPosition = PlayerPawn->GetActorLocation();

		// Calculate the change in position since the last update
		FVector DeltaPosition = CurrentPosition - LastPosition;

		// Calculate the time that has elapsed since the last update
		float DeltaTimeSinceLastUpdate = CurrentTime - LastUpdateTime;

		// Calculate the velocity
		AnimationVelocity = DeltaPosition / DeltaTimeSinceLastUpdate;

		// Optionally, you can use the length of the velocity vector to get the speed of the object
		float Speed = AnimationVelocity.Size();

		// Store the current position and time for the next update
		LastPosition = CurrentPosition;
		LastUpdateTime = CurrentTime;

		if (AnimationVelocity.Size() <= 50.0f)
		{
			AnimationVelocity = FVector::ZeroVector;
		}
		if (bIsGrounded)
		{
			AnimationVelocity.Z = 0.f;
		}
	}
}

void UCTFComponent::SetMovementState(EMovementState State)
{	

	if (PlayerPawn)
	{		
		
		MovementState = State;
		switch (MovementState)
		{
		case EMovementState::EMS_Walking:
			bIsGrounded = true;
			bIsIdeling = false;
			bIsCrouching = false;
			bAffectedByGravity = false;			
			MovementTypeMultiplyer = 1.0f;
			bCanTransitionFlight = false;		
			break;		
		case EMovementState::EMS_Falling:
			bIsIdeling = false;
			bIsGrounded = false;
			bAffectedByGravity = true;
			bIsFlying = false;
			bIsCrouching = false;
			MovementTypeMultiplyer = AirControlWhileJumpingMultiplier;
			bCanTransitionFlight = false;
			break;
		case EMovementState::EMS_Hovering:
			bIsIdeling = false;
			bAffectedByGravity = false;
			bCanTransitionFlight = false;
			bIsGrounded = false;
			bIsJumping = false;
			bIsFlying = true;
			bIsCrouching = false;			
			MovementTypeMultiplyer = AirControlWhileFlyingMultiplier;
			GetWorld()->GetTimerManager().ClearTimer(JumpTimerHandle);			
			break;
		case EMovementState::EMS_Flying:
			bIsIdeling = false;
			bCanTransitionFlight = true;			
			break;
		case EMovementState::EMS_Crouching:
			bIsIdeling = false;
			bIsCrouching = true;
			bIsGrounded = true;
			bAffectedByGravity = false;
			bIsFlying = false;			
			bCanTransitionFlight = false;
			bSliding = false;
			break;
		case EMovementState::EMS_Sliding:
			bSliding = true;
			bIsBoosting = false;
			break;
		
		}
		if (!PlayerPawn->HasAuthority() && PlayerPawn->IsLocallyControlled())
		{
			ServerSetMovementState(State);
		}
		else
		{
			Multi_SetMovementState(State);
		}
	}
	
}

void UCTFComponent::ServerSetMovementState_Implementation(EMovementState State)
{	
	SetMovementState(State);	
}

void UCTFComponent::Multi_SetMovementState_Implementation(EMovementState State)
{
	MovementState = State;
	switch (MovementState)
	{
	case EMovementState::EMS_Walking:
		bIsGrounded = true;
		bIsIdeling = false;
		bIsCrouching = false;
		bAffectedByGravity = false;		
		MovementTypeMultiplyer = 1.0f;
		bCanTransitionFlight = false;		
		break;	
	case EMovementState::EMS_Falling:
		bIsIdeling = false;
		bIsGrounded = false;
		bAffectedByGravity = true;
		bIsFlying = false;
		bIsCrouching = false;
		MovementTypeMultiplyer = AirControlWhileJumpingMultiplier;
		bCanTransitionFlight = false;
		break;
	case EMovementState::EMS_Hovering:
		bIsIdeling = false;
		bAffectedByGravity = false;
		bCanTransitionFlight = false;
		bIsGrounded = false;
		bIsJumping = false;
		bIsFlying = true;
		bIsCrouching = false;		
		MovementTypeMultiplyer = AirControlWhileFlyingMultiplier;
		GetWorld()->GetTimerManager().ClearTimer(JumpTimerHandle);	
		break;
	case EMovementState::EMS_Flying:
		bIsIdeling = false;
		bCanTransitionFlight = true;		
		break;
	case EMovementState::EMS_Crouching:
		bIsIdeling = false;
		bIsCrouching = true;
		bIsGrounded = true;
		bAffectedByGravity = false;
		bIsFlying = false;		
		bCanTransitionFlight = false;
		bSliding = false;
		break;
	case EMovementState::EMS_Sliding:
		bSliding = true;
		bIsBoosting = false;
		break;
	
	}
}

void UCTFComponent::OnRep_MovementState(EMovementState LastMovementState)
{
	
	SetMovementState(MovementState);
	
}

void UCTFComponent::AutoCorrectOrientation(float DeltaTime)
{
	FRotator CurrentRotation = PlayerPawn->GetActorRotation();
	OriginialOrientation = FRotator(OriginialOrientation.Pitch, PlayerPawn->GetActorRotation().Yaw, OriginialOrientation.Roll);	
	if(!bCanTransitionFlight || !bIsFlying && CurrentRotation != OriginialOrientation)
	{
		CurrentRotation = FMath::RInterpTo(CurrentRotation, OriginialOrientation, DeltaTime, 5.0f);
		PlayerPawn->SetActorRotation(CurrentRotation, ETeleportType::TeleportPhysics);
	}
}

void UCTFComponent::AddYawRotation(float DeltaTime)
{
	float MDPS;
	if (bIsFlying)
	{
		if (bCanTransitionFlight)
		{
			MDPS = 140;
		}
		else
		{
			MDPS = 260;
		}
		YawInputRunup = FMath::FInterpTo(YawInputRunup, YawInput, DeltaTime, 4.0f);
		
	}
	else
	{
		
		YawInputRunup = FMath::FInterpConstantTo(YawInputRunup, YawInput, DeltaTime, 2.0f);
		MDPS = MaxDegreesPerSecond;
	}
	if (PlayerPawn)
	{		
		float RotationAngle = MDPS * YawInputRunup;
		FQuat RotationDelta(GetOwnerUpVector(), FMath::DegreesToRadians(RotationAngle));
		if (bIsGrounded)
		{			
			//PlayerVelocity = RotationDelta.RotateVector(PlayerVelocity);
		}		
		
		PlayerPawn->AddActorWorldRotation(RotationDelta, false, nullptr, ETeleportType::TeleportPhysics);
		

		
		
	}
}

void UCTFComponent::AddPitchRotation(float DeltaTime)
{
	float MDPS;
	if (bCanTransitionFlight)
	{
		PitchInputRunup = FMath::FInterpConstantTo(PitchInputRunup, PitchInput, DeltaTime, 2.f);
		MDPS = MaxDegreesPitchPerSecondFlying;
	}
	else
	{
		
		PitchInputRunup = 0.0f;
		MDPS = MaxDegreesPerSecond;
	}
	float RotationAngle = MDPS * DeltaTime * PitchInputRunup;
	FQuat RotationDelta(PlayerPawn->GetActorRightVector(), FMath::DegreesToRadians(RotationAngle));	
	PlayerPawn->AddActorWorldRotation(RotationDelta, true, nullptr,  ETeleportType::TeleportPhysics);
	
}

void UCTFComponent::AddRollRotation(float DeltaTime)
{
	if (bCanTransitionFlight)
	{
		RollInputRunup = FMath::FInterpConstantTo(RollInputRunup, RollInput, DeltaTime, 2.0f);
	}
	else
	{
		RollInputRunup = 0.0f;
	}
	float RotationAngle = MaxDegreesRollPerSecond * DeltaTime * RollInputRunup;
	FQuat RotationDelta(PlayerPawn->GetActorForwardVector(), FMath::DegreesToRadians(RotationAngle));
	PlayerPawn->AddActorWorldRotation(RotationDelta, true, nullptr, ETeleportType::TeleportPhysics);
}

void UCTFComponent::Turn(float Val)
{	
	
	
	if (bIsFlying)
	{
		
		
		YawInput = FMath::FInterpTo(YawInput, Val, GetWorld()->GetDeltaSeconds(), 2.5f);
		
		
	}
	else
	{
		YawInput = Val;
	}
	
}

void UCTFComponent::Pitch(float Val)
{
	
	PitchInput = Val;
}

void UCTFComponent::Roll(float Val)
{
	
	RollInput = Val;
}

FVector UCTFComponent::GetOwnerUpVector()
{
	if (PlayerPawn)
	{
		return PlayerPawn->GetActorUpVector();
	}
	else
	{
		return FVector(0.0, 0.0, 1.0);
	}	
}

void UCTFComponent::AddGravity(float DeltaTime)
{
	

	FVector Gravity;
	if (!bIsGrounded && !bIsFlying && !bIsJumping)
	{
		
		Gravity = GravityDirection * -GravityForce * GravityMultiplier;
	}
	else
	{
		Gravity = FVector::ZeroVector;
	}
	if (bIsFlying)
	{
		UpVelocity = FMath::VInterpTo(UpVelocity, FVector::ZeroVector, DeltaTime, 1.5);
	}

	UpVelocity  = UpVelocity + Gravity / 100;
	UpVelocity.Z = FMath::Clamp(UpVelocity.Z, -TerminalVelocity, 100);

	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(-1, .01f, FColor::Blue, FString(UpVelocity.ToString()));
	}
}

void UCTFComponent::TraceForGround(float DeltaTime)
{
	if (PlayerPawn)
	{
		
		FHitResult Hit;
		FVector Start;
		FVector End;
		float Radious;
		if (MovementState == EMovementState::EMS_Flying && bCanTransitionFlight)
		{
			Radious = TraceCapsuleRadious * .2;
			 Start = PlayerPawn->GetActorLocation();
			 End = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 110;
		}
		else
		{
			Radious = TraceCapsuleRadious;
			 Start = PlayerPawn->GetActorLocation() + GetOwnerUpVector() * 10;
			 End = PlayerPawn->GetActorLocation() + GetOwnerUpVector() * -TraceForGroundRange;
		}
		
		

				
		TArray<AActor*> ActorsToIgnore;
		UKismetSystemLibrary::SphereTraceSingle(this, Start, End, Radious, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
		true, ActorsToIgnore, EDrawDebugTrace::None, Hit, true);
		if (Hit.IsValidBlockingHit())
		{

			
			float Slope;			
			FVector UpVector = PlayerPawn->GetActorUpVector();
			Slope = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(UpVector, Hit.ImpactNormal)));
			
		
			if (Slope <= 46 && MovementState != EMovementState::EMS_Flying && MovementState != EMovementState::EMS_Hovering)
			{
				if (bIsCrouching)
				{
					SetMovementState(EMovementState::EMS_Crouching);
				}
				else
				{
					SetMovementState(EMovementState::EMS_Walking);				
				}
				bIsSlidingDownSlope = false;
				SlideMultiplier = 1;				
			}
			else if(MovementState != EMovementState::EMS_Flying && MovementState != EMovementState::EMS_Hovering)
			{
				SetMovementState(EMovementState::EMS_Falling);
				SlideMultiplier = 5;
			}				
		}
		else 
		{
			bIsGrounded = false;			
		}

		if (!bIsJumping && !bCanTransitionFlight)
		{
			FVector GroundedLocation;
			if (bIsGrounded)
			{
				if (UpVelocity != FVector::ZeroVector)
				{
					UpVelocity = FVector::ZeroVector;
				}
				
				
				GroundedLocation = FVector(PlayerPawn->GetActorLocation().X, PlayerPawn->GetActorLocation().Y, Hit.ImpactPoint.Z + TraceForGroundRange);
				PlayerPawn->SetActorLocation(GroundedLocation, true);
			}
			else if(bIsFlying && PlayerPawn->GetActorLocation().Z < Hit.ImpactPoint.Z + TraceForGroundRange && Hit.IsValidBlockingHit())
			{
				GetWorld()->GetTimerManager().ClearTimer(JumpTimerHandle);
				GroundedLocation = FVector(PlayerPawn->GetActorLocation().X, PlayerPawn->GetActorLocation().Y, Hit.ImpactPoint.Z + TraceForGroundRange);
				PlayerPawn->SetActorLocation(GroundedLocation, true);
			}			
		}				
	}		
}

void UCTFComponent::MoveCharacter(float DeltaTime)
{
	FHitResult Hit;
	
	PlayerVelocity = PlayerVelocity * DeltaTime * 100;
	PlayerPawn->AddActorWorldOffset(PlayerVelocity, true, &Hit, ETeleportType::TeleportPhysics);
	if (Hit.bBlockingHit)
	{	
		
		BounceOffWallVector = FVector::VectorPlaneProject(PlayerVelocity, Hit.ImpactNormal * 1.001);
		PlayerPawn->AddActorWorldOffset(BounceOffWallVector, true, nullptr, ETeleportType::TeleportPhysics);

		ReflectionVector = FVector::VectorPlaneProject(PlayerVelocity, Hit.ImpactNormal * 1.001);
		if (ReflectionVector.Z > 0)
		{
			ReflectionVector.Z = 0;
		}
	}
	else
	{
		ReflectionVector = FVector::ZeroVector;
	}
}

void UCTFComponent::CalculateVelocity(float DeltaTime)
{
	FVector Force = (ForwardVelocity + RightVelocity + ReflectionVector);
	Force += GetForwardAirResistance();
	Force += GetGroundResistance();
	FVector Acceleration = Force / Mass;
	GroundMovementVelocity = GroundMovementVelocity + Acceleration;
	
	if (bIsFlying)
	{
		if (bIsBoosting)
		{
			MaxSpeed = FlyingSpeedBoosting;
		}
		else
		{
			MaxSpeed = FlyingSpeed;
		}
		
	}
	else
	{
		if (bIsCrouching)
		{
			MaxSpeed = CrouchingSpeed;
		}
		else
		{
			if (bIsBoosting && ForwardInput > 0.f)
			{
				MaxSpeed = WalkSpeedBoosting;
			}
			else
			{
				if (bAiming)
				{
					MaxSpeed = WalkSpeedAiming;
				}
				else
				{
					MaxSpeed = WalkSpeed;
				}
				
			}
			
		}
	}
	if (bBoostJumping)
	{
		MaxSpeed = WalkSpeed;
	}
	
	

	GroundMovementVelocity = UKismetMathLibrary::ClampVectorSize(GroundMovementVelocity, -MaxSpeed, MaxSpeed);
	if (bSliding)
	{
		GroundMovementVelocity = GroundMovementVelocity * 1.5f;
	}
	
	PlayerVelocity = GroundMovementVelocity + UpVelocity;

	if (PlayerVelocity.Size() < 0.01f)
	{
		PlayerVelocity = FVector::ZeroVector;
	}	
}

void UCTFComponent::MoveForward(FVector ForwardVector, float Val)
{		
	ForwardInputVector = ForwardVector;
	

	if (!bIsGrounded && !bIsFlying && !bBoostJumping)
	{
		ForwardInput = Val * AirControlWhileJumpingMultiplier;
	}
	else if(bIsFlying)
	{
		ForwardInput = Val;
	}
	else if (bIsGrounded)
	{
		ForwardInput = Val;		
	}
	else if (bBoostJumping)
	{
		ForwardInput = Val;
	}
	
	
	float ModifiedForce;

	if (bIsFlying && Val < 0.0f)
	{
		ModifiedForce = MaxRightForce;
		ForwardInput = Val * 0.075f;
	}
	else
	{
		ModifiedForce = MaxForwardForce;
	}
	ForwardVelocity = ForwardInputVector * ForwardInput * ModifiedForce;	

}

FVector UCTFComponent::GetForwardAirResistance()
{
	if (bIsGrounded)
	{
		return -GroundMovementVelocity.GetSafeNormal() * GroundMovementVelocity.SizeSquared() * DragCoefficient;
	}
	else
	{
		return -GroundMovementVelocity.GetSafeNormal() * GroundMovementVelocity.SizeSquared() * (AirDragCoefficient);
	}
	
}

FVector UCTFComponent::GetRightAirResistance()
{
	return -RightVelocity.GetSafeNormal() * RightVelocity.SizeSquared() * DragCoefficient;
}

FVector UCTFComponent::GetGroundResistance()
{
	float AccelerationDueToGravity = GravityForce / 100;
	float NormalForce = Mass * AccelerationDueToGravity;
	if (bIsGrounded)
	{
		return -GroundMovementVelocity.GetSafeNormal() * GroundResistanceCoefficient * NormalForce;
	}
	else
	{
		return -GroundMovementVelocity.GetSafeNormal() * FlyingResistanceCoefficient * NormalForce;
	}
	
}

void UCTFComponent::MoveRight(FVector RightVector, float Val)
{

	
	RightVectorInput = RightVector;
	if (bIsFlying)
	{
		RightInput = Val *0.075f;
	}
	else
	{
		if (!bIsGrounded)
		{
			RightInput = Val * AirControlWhileJumpingMultiplier;
		}
		else
		{
			RightInput = Val;
		}
		
	}
	
	RightVelocity = RightVectorInput * RightInput * MaxRightForce;

}

void UCTFComponent::Server_EnterFlight_Implementation()
{
	if (bServerCanDamageFromThrusters)
	{
		bServerCanDamageFromThrusters = false;
		UGameplayStatics::ApplyDamage(GetOwner(), 30, nullptr, GetOwner(), UDamageType::StaticClass());
		GetWorld()->GetTimerManager().SetTimer(
			BoostJumpStopTimerHandle,
			this,
			&UCTFComponent::StopBoostJump,
			3.0f
		);
	}
}

void UCTFComponent::JumpButtonPressed()
{	
	if (bIsGrounded)
	{
		
		if (bIsCrouching)
		{
			Crouch(); return;
		}
		UpVelocity = FVector::ZeroVector;
		bWantsToJump = true;		
		JumpTimer();
		if (bIsBoosting && bCanBoostJump)
		{
			BoostJump();
			//GetWorld()->GetTimerManager().ClearTimer(JumpTimerHandle);
		}
	}
	else if(MovementState == EMovementState::EMS_Hovering || MovementState == EMovementState::EMS_Flying)
	{
		SetMovementState(EMovementState::EMS_Falling);
		
	}
	else if(bCanFly)
	{
		SetMovementState(EMovementState::EMS_Hovering);
		GetWorld()->GetTimerManager().ClearTimer(JumpTimerHandle);
		Server_EnterFlight();
		
		
	}	
}

void UCTFComponent::JumpButtonReleased()
{
	JumpEnded();
}

void UCTFComponent::PlayerJump(float DeltaTime)
{

	
	if (PlayerPawn == nullptr || bIsFlying) return;
	
	if (!bIsJumping && !bBoostJumping)
	{
		JumpVelocity = FVector::ZeroVector;		
	}
	else
	{
		bIsGrounded = false;
		JumpVelocity = PlayerPawn->GetActorUpVector() * JumpForce;
	}
	
	if (bBoostJumping)
	{		
		
		JumpVelocity = PlayerPawn->GetActorUpVector() * 400  + (LookAtVector * 200 * ForwardInput); 
		
	}	
	
	UpVelocity = UKismetMathLibrary::Vector_ClampSizeMax(UpVelocity + JumpVelocity / 100, 50);
		
}

void UCTFComponent::PlayerStopJump()
{
	
	
	JumpEnded();	
}

void UCTFComponent::JumpEnded()
{
	bWantsToJump = false;
	bIsJumping = false;
	bBoostJumping = false;

	GetWorld()->GetTimerManager().ClearTimer(EndJumpTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(JumpTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(BoostJumpTimerHandle);
	
}

void UCTFComponent::StopBoostJump()
{
	bServerCanDamageFromThrusters = true;
	JumpEnded();
}

void UCTFComponent::JumpTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(EndJumpTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(JumpTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(BoostJumpTimerHandle);
	bIsJumping = true;	
	GetWorld()->GetTimerManager().SetTimer(
		JumpTimerHandle,
		this,
		&UCTFComponent::PlayerStopJump,
		EndJumpTime
	);
}

void UCTFComponent::BoostJump()
{
	if (!bWantsToJump) return;
	Server_EnterFlight();
	bBoostJumping = true;	
	GetWorld()->GetTimerManager().SetTimer(
		BoostJumpTimerHandle,
		this,
		&UCTFComponent::StopBoostJump,
		JumpHoldTime
	);
	bServerCanDamageFromThrusters = false;
}

void UCTFComponent::Crouch()
{
	if (bIsFlying) return;
	if (!bIsCrouching)
	{
		
		if (bIsBoosting)
		{	
			bIsBoosting = false;
			SetMovementState(EMovementState::EMS_Sliding);
			GetWorld()->GetTimerManager().SetTimer(
				SlidingTimerHandle,
				this,
				&UCTFComponent::StopSliding,
				SlideTime
			);
		}
		else
		{
			SetMovementState(EMovementState::EMS_Crouching);
		}
		
	}
	else
	{
		FHitResult Hit;
		FVector Start = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorUpVector();
		FVector End = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorUpVector() * TraceForCelingRange;



		TArray<AActor*> ActorsToIgnore;
		UKismetSystemLibrary::SphereTraceSingle(this, Start, End, TraceCapsuleRadious, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
			false, ActorsToIgnore, EDrawDebugTrace::None, Hit, true);
		if (Hit.IsValidBlockingHit()) return;
		SetMovementState(EMovementState::EMS_Walking);
		
	}
	
}

void UCTFComponent::StopSliding()
{	
	SetMovementState(EMovementState::EMS_Crouching);
}
