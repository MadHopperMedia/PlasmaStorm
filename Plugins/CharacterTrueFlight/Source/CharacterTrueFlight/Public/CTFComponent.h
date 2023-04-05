// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "CTFComponent.generated.h"


UENUM(BlueprintType)
enum class EMovementState : uint8
{
	
	EMS_Walking UMETA(DisplayName = "Walking"),
	EMS_Falling UMETA(DisplayName = "Falling"),
	EMS_Hovering UMETA(DisplayName = "Hovering"),
	EMS_Flying UMETA(DisplayName = "Flying"),
	EMS_Crouching UMETA(DisplayName = "Crouching"),
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),
	EMS_Max UMETA(DisplayName = "DefaultMax"),
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHARACTERTRUEFLIGHT_API UCTFComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCTFComponent();
	friend class ACTFPawn;
	friend class APSCharacter;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY()
	class APawn* PlayerPawn;

	UFUNCTION(BlueprintCallable)
	void MoveForward(FVector ForwardVector, float Val);
	UFUNCTION(BlueprintCallable)
	void MoveRight(FVector RightVector, float Val);
	UFUNCTION(BlueprintCallable)
	void JumpButtonPressed();
	UFUNCTION(BlueprintCallable)
	void JumpButtonReleased();	
	UFUNCTION(BlueprintCallable)
	void Turn(float Val);
	UFUNCTION(BlueprintCallable)
	void Pitch(float Val);
	UFUNCTION(BlueprintCallable)
	void Roll(float Val);
	UFUNCTION(BlueprintCallable)
	void Crouch();
	
	UPROPERTY(BlueprintReadOnly)
	FVector PlayerVelocity;
	

	

	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	// Mass of Actor in (KG)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Mass = 500;
	// The Force Applied to the actor in order to move forward
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxForwardForce = 800;
	// The Force Applied to the actor in order to move forward
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxRightForce = 600;
	// the maximum speed the actor can rotate While on the ground
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxDegreesPerSecond = 360;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxDegreesPitchPerSecondFlying = 180;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxDegreesRollPerSecond = 150;
	// Applies Drag to the actor. Higher numbers equal more drag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DragCoefficient = 16;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AirDragCoefficient = 0.9f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundResistanceCoefficient = 0.01f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float FlyingResistanceCoefficient = 0.001;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GravityForce = 980;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpHoldTime = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float EndJumpTime = 0.3;	
	// Adjusts the amount of control you have while jumping. 0 equals none and 1 is full control
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AirControlWhileJumpingMultiplier = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AirControlWhileFlyingMultiplier = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 8.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeedAiming = 2.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeedBoosting = 12.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float FlyingSpeed = 12.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float FlyingSpeedBoosting = 16.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CrouchingSpeed = 4.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float TraceForGroundRange = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float TraceForGroundCrouchingRange = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float TraceForCelingRange = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GravityMultiplier = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpForce = 500;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float TraceCapsuleRadious = 20;	
	UPROPERTY(BlueprintReadOnly)
	bool bIsGrounded = true;
	UPROPERTY(BlueprintReadOnly)
	bool bIsIdeling = false;
	UPROPERTY(BlueprintReadOnly)
	bool bIsJumping = false;
	UPROPERTY(BlueprintReadOnly)
	bool bIsFlying = false;
	UPROPERTY(BlueprintReadOnly)
	bool bIsCrouching = false;
	UPROPERTY(BlueprintReadOnly)
	bool bIsBoosting = false;
	UPROPERTY(BlueprintReadOnly)
	FVector MovementVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator OriginialOrientation;
	bool bCanMove = true;
	bool bCanFly = true;

	UPROPERTY(BlueprintReadOnly)
	FVector AnimationVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector GravityDirection = FVector(0.0f, 0.0f, 1.0f);

	UPROPERTY(ReplicatedUsing = OnRep_MovementState, BlueprintReadWrite, EditAnywhere, Category = "Movement")	
	EMovementState MovementState;	
	
	UPROPERTY(BlueprintReadOnly)
	bool bCanTransitionFlight = false;
	UPROPERTY(BlueprintReadOnly)
	bool IsAccelerating = false;
	UPROPERTY(BlueprintReadOnly)
	float ForwardInput = 0.f;

	ACTFPawn* PlayerCharacter;
private:
	
	FVector ForwardInputVector;	
	FVector RightVectorInput;
	void AddYawRotation(float DeltaTime);
	void AddRollRotation(float DeltaTime);
	void AddPitchRotation(float DeltaTime);
	void PlayerJump(float DeltaTime);
	void PlayerStopJump();
	void JumpTimer();
	void MoveCharacter(float DeltaTime);
	void CalculateAnimationVelocity(float DeltaTime);

	
	FVector LastPosition;
	float LastUpdateTime;
	
	bool bAiming = false;
	
	//float ForwardInput = 0.f;
	FVector ForwardVelocity;
	FVector RightVelocity;
	FVector UpVelocity;
	FVector GroundMovementVelocity;
	float RightInput = 0.f;
	bool bAffectedByGravity = true;
	float JumpImpulse;	
	FVector ForwardRightClampedVelocity;
	bool bIsSlidingDownSlope = false;
	float SlideMultiplier = 1;
	float MaxSpeed;
	float RollInputRunup = 0;
	float YawInputRunup = 0;
	float PitchInputRunup = 0;

	void SetMovementState(EMovementState State);

	UFUNCTION(Server, Reliable)
	void ServerSetMovementState(EMovementState State);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multi_SetMovementState(EMovementState State);

	UFUNCTION()
	void OnRep_MovementState(EMovementState LastMovementState);

	UFUNCTION(Server, Reliable)
	void Server_EnterFlight();

	

	FVector ReflectionVector;
	FVector SlidingVector;
	float MovementTypeMultiplyer;
	float YawInput;
	float PitchInput;
	float RollInput;
	FVector GetForwardAirResistance();
	FVector GetRightAirResistance();
	FVector GetGroundResistance();
	FVector GetOwnerUpVector();
	FVector BounceOffWallVector;	
	void CalculateVelocity(float DeltaTime);
	FTimerHandle JumpTimerHandle;
	FTimerHandle EndJumpTimerHandle;

	void AddGravity(float DeltaTime);
	FVector GravityVector = FVector(0.0f, 0.0f, -1.0f);

	void TraceForGround(float DeltaTime);

	void AutoCorrectOrientation(float DeltaTime);
	float FixedTimeStep = 0.01;
	float ElapsedTime;
	
	
	


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	FORCEINLINE float GetForwardInput() const { return ForwardInput; }
	FORCEINLINE bool SetCanMove(bool bAllowMove) { return bCanMove = bAllowMove; }
	FORCEINLINE void SetBoosting(bool Boosting) { bIsBoosting = Boosting; }
	FORCEINLINE float GetMaxSpeed() const { return MaxSpeed; }
	FORCEINLINE bool SetAiming(bool Aiming) { return bAiming = Aiming; }
	
	
	
	
	

		
};
