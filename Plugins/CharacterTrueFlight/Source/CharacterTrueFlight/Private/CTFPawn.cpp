// Fill out your copyright notice in the Description page of Project Settings.


#include "CTFPawn.h"
#include "CTFComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
ACTFPawn::ACTFPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetBoxExtent(BoxExtentWalking);
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxCollision->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	BoxCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	RootComponent = BoxCollision;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(Mesh);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	
	CTFComponent = CreateDefaultSubobject<UCTFComponent>(TEXT("CTFComponent"));
	CTFComponent->SetIsReplicated(true);

	SetReplicateMovement(false);
	CameraBoom->CameraLagSpeed = 50.f;

}
 void ACTFPawn::PostInitializeComponents()
{
	 Super::PostInitializeComponents();

	 if (CTFComponent)
	 {
		 CTFComponent->PlayerPawn = this;
	 }
}

// Called when the game starts or when spawned
void ACTFPawn::BeginPlay()
{
	Super::BeginPlay();	

	StartingBoomLocation.Y = CameraBoom->SocketOffset.Y;
}

// Called every frame
void ACTFPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CTFComponent != nullptr && CTFComponent->bCanMove)
	{
		TurnPawnTowardFlightDirection(DeltaTime);
		Speed = CTFComponent->AnimationVelocity;
		bIsGrounded = CTFComponent->bIsGrounded;
		bIsFlying = CTFComponent->bIsFlying;
		bTransitioningfromFlight = CTFComponent->bCanTransitionFlight;
		bIsIdeling = CTFComponent->bIsIdeling;
		
	}
	
}

void ACTFPawn::SetCanMove(bool bCanMove)
{
	CTFComponent->SetCanMove(false);
}

// Called to bind functionality to input
void ACTFPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACTFPawn::MoveForward(float Val)
{
	ForwardVal = Val;
	if (CTFComponent == nullptr) return;
	if (!bIsFlying)
	{
		CTFComponent->MoveForward(GetActorForwardVector(), Val);
	}
	 else if(bIsFlying)
	{
		CTFComponent->MoveForward(CameraBoom->GetForwardVector(), Val);
	}	
}

void ACTFPawn::MoveRight(float Val)
{
	if (CTFComponent == nullptr) return;
	CTFComponent->MoveRight(GetActorRightVector(), Val);
}

void ACTFPawn::Jump()
{
	if (CTFComponent == nullptr) return;	
	CTFComponent->JumpButtonPressed();
}

void ACTFPawn::StopJump()
{
	if (CTFComponent == nullptr) return;
	CTFComponent->JumpButtonReleased();
}

void ACTFPawn::TurnPawnTowardFlightDirection(float DeltaTime)
{
	if (CTFComponent == nullptr) return;
	
	

	if (bIsFlying && bTransitioningfromFlight)
	{
		CameraBoom->bEnableCameraRotationLag = true;
		CameraBoom->bEnableCameraLag = true;
		CameraBoom->CameraLagSpeed = FMath::FInterpConstantTo(CameraBoom->CameraLagSpeed, 5, DeltaTime, 20);
		float BoomYLocation = FMath::FInterpTo(CameraBoom->SocketOffset.Y, FlyingBoomLocation.Y, DeltaTime, 6);
		//CameraBoom->SetRelativeLocation(FVector(0.0, BoomYLocation, 50.0));
		CameraBoom->SocketOffset.Y = BoomYLocation;

		if (IsLocallyControlled())
		{
			SetRotationForFlight(DeltaTime);			

			if (CurrentBoxExtent != BoxExtentFlying)
			{
				CurrentBoxExtent = FMath::VInterpTo(CurrentBoxExtent, BoxExtentFlying, DeltaTime, 2.5);
				BoxCollision->SetBoxExtent(CurrentBoxExtent);
			}			
		}
		
	}
	else
	{
		CurrentBoomRotation = FMath::RInterpTo(FRotator(CameraBoom->GetComponentRotation().Pitch, GetActorRotation().Yaw, GetActorRotation().Roll), CurrentBoomRotation, DeltaTime, 2.5);
		CameraBoom->SetWorldRotation(CurrentBoomRotation);


		if (!CTFComponent->bIsCrouching && CurrentBoxExtent != BoxExtentWalking)
		{
			CurrentBoxExtent = FMath::VInterpTo(CurrentBoxExtent, BoxExtentWalking, DeltaTime, 10.0f);

		}
		else if (CurrentBoxExtent != BoxExtentCrouching && CTFComponent->bIsCrouching)
		{
			CurrentBoxExtent = FMath::VInterpTo(CurrentBoxExtent, BoxExtentCrouching, DeltaTime, 10.0f);

		}
		BoxCollision->SetBoxExtent(CurrentBoxExtent);
		if (CTFComponent->bIsFlying && Mesh->GetRelativeLocation() != FVector(0.f, 0.f, -84.f))
		{
			CurrentMeshLocation = FVector(0.f, 0.f, -84.f);
		}
		if (CTFComponent->bIsCrouching && Mesh->GetRelativeLocation() != CrouchingMeshLocation)
		{
			CurrentMeshLocation = CrouchingMeshLocation;
			
		}

		if (!CTFComponent->bIsCrouching && !CTFComponent->bIsFlying && Mesh->GetRelativeLocation() != WalkingMeshLocation)
		{
			CurrentMeshLocation = WalkingMeshLocation;			
		}
		Mesh->SetRelativeLocation(CurrentMeshLocation);
		CameraBoom->bEnableCameraRotationLag = false;			
		CameraBoom->CameraLagSpeed = FMath::FInterpConstantTo(CameraBoom->CameraLagSpeed, 50, DeltaTime, 10);
		float BoomYLocation = FMath::FInterpTo(CameraBoom->SocketOffset.Y, StartingBoomLocation.Y, DeltaTime, 6);
		//CameraBoom->SetRelativeLocation(FVector(0.0, BoomYLocation, 50.0));
		CameraBoom->SocketOffset.Y = BoomYLocation; //(FVector(0.0, BoomYLocation, 50.0));
		if (CameraBoom->CameraLagSpeed == 50.f && CameraBoom->bEnableCameraLag == true)
		{
			CameraBoom->bEnableCameraLag = false;
		}
	}
}

void ACTFPawn::SetRotationForFlight(float DeltaTime)
{
	FRotator CurrentRotation = GetActorRotation();
	AutoRotationDirection = CameraBoom->GetComponentRotation();
	CurrentRotation = FMath::RInterpConstantTo(CurrentRotation, FRotator(AutoRotationDirection.Pitch, GetActorRotation().Yaw, GetActorRotation().Roll), DeltaTime, 50);
	SetActorRotation(CurrentRotation);

	CurrentBoomRotation = CameraBoom->GetComponentRotation();
	CurrentBoomRotation = FMath::RInterpConstantTo(CurrentBoomRotation, GetActorRotation(), DeltaTime, 50);
	CameraBoom->SetWorldRotation(CurrentBoomRotation);
	PitchFloat = CameraBoom->GetComponentRotation().Pitch;
}

void ACTFPawn::Turn(float Val)
{
	
	if (CTFComponent == nullptr) return;	
	
	CTFComponent->YawInput = Val * YawSensitivity * GetWorld()->GetDeltaSeconds();
}

void ACTFPawn::LookUp(float Val)
{
	if (CTFComponent == nullptr) return;

	PitchVal = Val;
	
	if (!bTransitioningfromFlight || !CTFComponent->bIsFlying)
	{		
		
		PitchFloat = FMath::Clamp(PitchFloat + (Val * PitchSensitivity * GetWorld()->GetDeltaSeconds() * 50), -MaxPitch, MaxPitch);
		
		CurrentBoomRotation = FRotator(PitchFloat, GetActorRotation().Yaw, GetActorRotation().Roll);

		CameraBoom->SetWorldRotation(CurrentBoomRotation);
		 
	}
	else
	{
		CTFComponent->PitchInput = -Val;			
	}	
}

void ACTFPawn::Roll(float Val)
{
	CTFComponent->RollInput = Val;
	RollVal = Val * -2;
}

void ACTFPawn::Crouch()
{
	if (CTFComponent == nullptr) return;
	CTFComponent->Crouch();
	GetIsCrouched();
}

bool ACTFPawn::GetIsCrouched()
{
	if (CTFComponent && CTFComponent->bIsCrouching)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ACTFPawn::SetAimingWalkSpeed(bool Aiming)
{
	CTFComponent->SetAiming(Aiming);
}

void ACTFPawn::SetBoosting(bool Boosting)
{
	CTFComponent->SetBoosting(Boosting);
}

void ACTFPawn::EnterFlight()
{

}

