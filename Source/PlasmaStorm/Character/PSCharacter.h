// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CTFPawn.h"
#include "PlasmaStorm/PSTypes/TurningInPlace.h"
#include "PlasmaStorm/PSTypes/FlyingRotation.h"
#include "Components/TimelineComponent.h"
#include "PlasmaStorm/PSTypes/CombatState.h"
#include "PlasmaStorm/Interfaces/InteractWithCrosshairsInterface.h"
#include "PSCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

/**
 * 
 */
UCLASS()
class PLASMASTORM_API APSCharacter : public ACTFPawn, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	APSCharacter();
	virtual void BeginPlay() override;
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDStamina();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void SetAimWalkSpeed(bool Aiming);
	void PlayFireMontage(bool bAiming);
	void PlayThrowGrenadeMontage();
	void SpawnDefaultWeapon();
	void PlayReloadMontage();
	void PlayHitReactMontage();
	void PlaySwapMontage();
	UFUNCTION(Client, Unreliable)
	void PlayKillSound();
	void Elim(bool bPlayerLeftGame);
	virtual void Destroyed() override;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);
	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();
	void PlayerPitch(float Val);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShieldRecharge();
	UPROPERTY(Replicated)
	FVector Impulse;
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);


	UPROPERTY()
		TMap<FName, class UBoxComponent*> HitCollisionBoxes;

	/**
	* Hit Boxes used for server side rewind
	*/
	UPROPERTY(EditAnywhere)
	class UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_05;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* foot_r;


	UPROPERTY(BlueprintReadWrite)
	bool bShowTraceinfo = false;

	bool bFinishedSwapping = true;

	FOnLeftGame OnLeftGame;
protected:

	void ForwardMovement(float Val);
	void RightMovement(float Val);
	//void PlayerAddPitchMouse();
	void PlayerYaw(float Val);
	void PlayerRoll(float Val);
	void PlayerCrouch();
	void SwitchWeaponButtonPressed();	
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void FireButtonPressed();
	void FireButtonReleased();
	void ReloadButtonPressed();
	void ReloadButtonReleased();
	
	void SetCollisionsAfterElimmed();
	void GrenadeButtonPressed();
	void BoostButtonPressed();
	void BoostButtonReleased();
	void JumpButtonPressed();
	void MeleeButtonPressed();
	UFUNCTION()
	void RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	void PollInit();
	void UpdateHUDAmmo();
	
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class AWeapon* OverlappingWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class ULagCompensationComponent* LagCompensation;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;
	UPROPERTY(EditAnywhere)
	class USoundCue* RechargeShieldSound;
	UPROPERTY(EditAnywhere)
	class USoundCue* KillSound;
	UPROPERTY(EditAnywhere)
	class USceneComponent* Target;
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed(bool IsEquipingWeapon);
	UFUNCTION(Server, Reliable)
	void ServerSwapWeaponsButtonPressed();


	FTimerHandle EquipTimer;

	UPROPERTY(EditAnywhere)
	float EquipDelay = 0.5f;

	void EquipTimerFinished();
	void HideCharacterIfCharacterClose();

	float Ao_Yaw;
	float Flying_PitchOffset;
	float Flying_RollOffset;

	UPROPERTY(ReplicatedUsing = OnRep_AoPitch)
	float Ao_Pitch;
	float NewAo_Pitch;
	UFUNCTION(Server, Reliable)
	void ServerAimOffset(float Val);
	
	bool PitchingUp;
	bool PitchingDown;
	bool bInverted = false;	
	bool EquippingWeapon = false;
	
	UFUNCTION()
	void OnRep_AoPitch();
	FRotator StartingAimRotation;
	


	
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);
	EFlyingRotation FlyingRotation;

	FRotator LastRotation;
	float LastUpdateTime;
	FRotator LastPitchRotation;

	//
	// Default Weapon
	//
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultSecondaryWeaponClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> MountedWeaponClass;
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapMontage;

	UPROPERTY(EditAnywhere)
	float	OverlappingCharacterMultiplier = 1;	
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	UPROPERTY(EditAnywhere)
	float SniperZoomYawSpeed = 0.05f;
	UPROPERTY(EditAnywhere)
	float ZoomYawSpeed = 4.0f;
	float Recoil = 0;

	/**
	* Player Health
	*/
	UPROPERTY(EditAnywhere, Category = PlayerStats)
	float MaxHealth = 20.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = PlayerStats)
	float Health = 20.f;
	bool bElimmed = false;
	UFUNCTION()
	void OnRep_Health(float LastHealth);

	UPROPERTY(EditAnywhere, Category = PlayerStats)
	float MaxShield = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, BlueprintReadOnly, Category = PlayerStats, meta = (AllowPrivateAccess = "true"))
	float Shield = 100.f;
	UFUNCTION()
	void OnRep_Shield(float LastShield);

	UPROPERTY(EditAnywhere, Category = PlayerStats)
	float MaxStamina = 100.f;
	UPROPERTY(EditAnywhere, Category = PlayerStats)
	float StaminaDrainSpeed = 2.f;
	UPROPERTY(EditAnywhere, Category = PlayerStats)
	float StaminaReplenishSpeed = 4.f;
	UPROPERTY(ReplicatedUsing = OnRep_Stamina, VisibleAnywhere, Category = PlayerStats)
	float Stamina = 100.f;
	UFUNCTION()
	void OnRep_Stamina(float LastStamina);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerStats, meta = (AllowPrivateAccess = "true"))
	float ShieldRequiredToFly = 30;

	FTimerHandle HitTimer;
	float HitDelay = 3.f;
	void HitTimerFinished();
	UPROPERTY()
	class APSPlayerController* PSPlayerController;

	UPROPERTY()
	APSPlayerController* AttackerController;;

	FTimerHandle ElimTimer;
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	void ElimTimerFinished();

	bool bLeftGame = false;

	

	

	/**
	* Dissolve effect
	*/

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UFUNCTION()
	void UpdateDisolveMaterial(float DissolveValue);
	void StartDissolve();

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;
	UPROPERTY(EditAnywhere)
	float DissolveGlowAmount = 200.f;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance1;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance1;
	UPROPERTY()
	class APSPlayerState* PSPlayerState;

	/**
	* Grenade
	**/
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;



	/*
	* Boosting
	*/
	bool bWantsToBoost = false;
	UPROPERTY(Replicated)
	bool bIsBoosting;
	UFUNCTION(Server, Reliable)
	void Server_IsBoosting(bool Boosting);
	bool bIsAccelerating = false;
	bool bToggleBoost = false;
	void StartBoosting();
	void StopBoosting();

	UPROPERTY()
	APSCharacter* TargetCharacter;
	

public:
	bool bIsCrouching;
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	FORCEINLINE float GetAo_Yaw() const { return Ao_Yaw; }
	FORCEINLINE float GetFlying_PitchOffset() const { return Flying_PitchOffset; }
	FORCEINLINE float GetAo_Pitch() const { return Ao_Pitch; }
	FORCEINLINE void SetAo_Pitch(float Pitch) { Ao_Pitch = Pitch; }
	FORCEINLINE float GetFlying_RollOffset() const { return Flying_RollOffset; }
	FORCEINLINE bool GetIsFlying() const { return bIsFlying; }
	FORCEINLINE bool GetIsFlyingForward() const { return bTransitioningfromFlight; }
	FORCEINLINE bool GetIsIdoling() const { return bIsIdeling; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE EFlyingRotation GetFlyingRotation() const { return FlyingRotation; }	
	FORCEINLINE FVector GetVelocity() const { return Speed; }
	FORCEINLINE float SetOverlappingCharacterMultiplier(float Multiplier) {  return OverlappingCharacterMultiplier = Multiplier; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }	
	ECombatState GetCombatState() const;
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UCombatComponent* GetCombat() const {return Combat; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }	
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }	
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE void SetStamina(float Amount) { Stamina = Amount; }
	FORCEINLINE float GetStamina() const { return Stamina; }
	FORCEINLINE float GetMaxStamina() const { return MaxStamina; }
	FORCEINLINE void SetInverted(bool Inverted) { bInverted = Inverted; }
	FORCEINLINE void SetStopBoosting() { StopBoosting(); }
	FORCEINLINE void SetToggleBoost(bool ToggleBoost) { bToggleBoost = ToggleBoost; }
	FORCEINLINE bool GetIsBoosting() { return bIsBoosting; }
	FORCEINLINE bool GetbIsAccelerating() { return bIsAccelerating; }
	FORCEINLINE void AddRecoilOnFire(float RecoilAmount) { PlayerPitch(RecoilAmount); }
	UFUNCTION(BlueprintCallable)
	bool GetIsFlying() { return bIsFlying; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE USceneComponent* GetTarget() const { return Target; }
	FORCEINLINE void SetTargetCharacter(APSCharacter* CurrentTarget) { TargetCharacter = CurrentTarget; }
	FORCEINLINE APSCharacter* GetTargetCharacter() { return TargetCharacter; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FVector PlayerVelocity() const { return Speed; }
	AWeapon* GetEquippedWeapons();
	AWeapon* GetSecondaryWeapons();
	AWeapon* GetMountedWeapons();
	bool IsLocallyReloading();
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
	UFUNCTION(BlueprintCallable)
	AWeapon* GetMountedWeapon();
	
	
};
