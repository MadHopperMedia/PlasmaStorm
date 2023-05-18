// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CTFComponent.h"
#include "CTFPawn.generated.h"

UCLASS()
class CHARACTERTRUEFLIGHT_API ACTFPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACTFPawn();
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	
	

	UPROPERTY(EditAnywhere, Category = CharactorProperties)
	float PitchSensitivity = 2.0f;
	UPROPERTY(EditAnywhere, Category = CharactorProperties)
	float YawSensitivity = 0.5f;

	UFUNCTION(BlueprintCallable)
		void MoveForward(float Val);
	UFUNCTION(BlueprintCallable)
		void MoveRight(float Val);
	UFUNCTION(BlueprintCallable)
		void Jump();
	UFUNCTION(BlueprintCallable)
		void StopJump();
	UFUNCTION(BlueprintCallable)
		void LookUp(float Val);
	UFUNCTION(BlueprintCallable)
		void Turn(float Val);
	UFUNCTION(BlueprintCallable)
		void Roll(float Val);
	UFUNCTION(BlueprintCallable)
		void Crouch();
	bool GetIsCrouched();

	FVector Speed;
	bool bIsGrounded;
	float ForwardVal;
	bool bIsChrouched;
	UPROPERTY()
	float PitchFloat = 0;

	float PitchVal;
	float RollVal;
	

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCTFComponent* CTFComponent;
	UPROPERTY(VisibleAnywhere, Category = Camera, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* BoxCollision;
	UPROPERTY(VisibleAnywhere, Category = Camera, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = Camera, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere, Category = Camera, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh;
	void SetAimingWalkSpeed(bool Aiming);
	void SetBoosting(bool Boosting);
	bool bIsFlying;
	bool bTransitioningfromFlight = false;
	bool bIsIdeling = true;
	void SetCanMove(bool bCanMove);
	bool bCanBoostJump = true;
	bool bIsSliding = false;
	void Dodge(FVector Direction);
	

private:	

	void TurnPawnTowardFlightDirection(float DeltaTime);
	void SetRotationForFlight(float DeltaTime);
	

	UPROPERTY(EditAnywhere, Category = CharactorProperties)
	float MaxPitch = 80;
	
	FRotator AutoRotationDirection;
	FRotator CurrentBoomRotation = FRotator(0.0f, 90.0f, 0.0f);
	UPROPERTY(EditAnywhere, Category = CharactorProperties)
	FVector BoxExtentWalking = FVector(30, 30, 80);
	UPROPERTY(EditAnywhere, Category = CharactorProperties)
	FVector BoxExtentFlying = FVector(70, 25, 20);
	UPROPERTY(EditAnywhere, Category = CharactorProperties)
	FVector BoxExtentCrouching = FVector(30, 30, 30);
	FVector CurrentBoxExtent = FVector(30, 30, 70);
	UPROPERTY(EditAnywhere, Category = CharactorProperties)
	FVector WalkingMeshLocation = FVector(0, 0, -100);
	UPROPERTY(EditAnywhere, Category = CharactorProperties)
	FVector CrouchingMeshLocation = FVector(0, 0, -100);
	FVector CurrentMeshLocation = WalkingMeshLocation;
	FVector StartingBoomLocation;
	FVector FlyingBoomLocation = FVector (0.0, 0.0, 50.0);

	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE USkeletalMeshComponent* GetMesh() { return Mesh; }
	FORCEINLINE UBoxComponent* GetBox() { return BoxCollision; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE float GetMaxSpeed() const { return CTFComponent->GetMaxSpeed(); }
	
	
	

};
