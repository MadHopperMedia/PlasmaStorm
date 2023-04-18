// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlasmaStorm/HUD/PSHud.h"
#include "PlasmaStorm/Weapon/WeaponTypes.h"
#include "PlasmaStorm/PSTypes/CombatState.h"
#include "CombatComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PLASMASTORM_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	friend class APSCharacter;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void EquipMountedWeapon(class AWeapon* WeaponToEquip);
	void SwapWeapons();
	void SwapToMountedWeapon();
	void Reload();
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize Target);

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);
	void PickupGrenade(int32 GrenadeAmount);
	bool bLocallyReloading = false;
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnRep_EquippedWeapon();
	UFUNCTION()
	void OnRep_SecondaryWeapon();
	UFUNCTION()
	void OnRep_MountedWeapon();
	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	
	void FireButtonPressed(bool bPressed);
	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshairs(float DeltaTime);
	void SetHUDCrosshairs(float DeltaTime);
	UFUNCTION(Server, Reliable)
	void ServerReload();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CrosshairOffset = 0.0f;

	void HandleReload();
	int32 AmountToReload();

	void ThrowGrenade();
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;
	UPROPERTY(EditAnywhere)
	float GrenadeThrowAngle = 10.f;
	void DropEquippedWeapon();
	void DropGrenades();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);	
	void AttachActorSecondarySocket(AActor* ActorToAttach);
	void AttachActorMountedSocket(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	void ReloadEmptyWeapon();
	void SetWeaponRange();
	void SetRecoil();
	void ShowAttachedGrenade(bool bShowGrenade);

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryweapon(AWeapon* WeaponToEquip);
	void AimAssist(float DeltaTime, FHitResult& HitResult);
	void PlayEquippedWeaponZoomSound();
	void PlayEquippedWeaponUnZoomSound();
	void Melee();

private:
	UPROPERTY()
	class APSCharacter* Character;
	UPROPERTY()
	class APSPlayerController* Controller;
	UPROPERTY()
	class APSHud* HUD;
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* MountedWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed = 8.0f;
	
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed = 2.5f;
	
	float FlyingCrosshairTransition;
	bool bFireButtonPressed;
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	FVector HitTarget;
	FVector TraceTargetForWeaponRotation;
	UPROPERTY(Replicated)
	class APSCharacter* TargetCharacter;
	float WeaponRange;
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;
	void InterpFOV(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	float StickyAssistAmount = .1f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float StickyAssistAmountAiming = .5f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float TurnToEnemyInterpSpeed = 20.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float PitchToEnemyInterpSpeed = 15.f;	
	float AimAssistSpeedPitch = 0.f;
	float AimAssistSpeedYaw = 0.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float TimeBeforeLosingTarget = .5f;
	float CurrentTurnSpeed;

	FHUDPackage HUDPackage;

	FTimerHandle FireTimer;

	FTimerHandle LostTargetTimer;

	void LostTargetTimerFinished();
	
	bool bCanFire = true;
	void FireTimerFinished();
	void StartFireTimer();
	bool CanFire();

	bool bPlaySecondaryEquip = false;


	// Carried Ammo for the curently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 0;



	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingMiniGunAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 MaxARAmmo = 400;
	UPROPERTY(EditAnywhere)
	int32 MaxRocketAmmo = 4;
	UPROPERTY(EditAnywhere)
	int32 MaxPistolAmmo = 200;
	UPROPERTY(EditAnywhere)
	int32 MaxSMGAmmo = 600;
	UPROPERTY(EditAnywhere)
	int32 MaxShotgunAmmo = 15;
	UPROPERTY(EditAnywhere)
	int32 MaxSniperAmmo = 12;
	UPROPERTY(EditAnywhere)
	int32 MaxGrenadeLauncherAmmo = 8;
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 2;
	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	void UpdateHUDGrenades();	

	void InitializeCarriedAmmo();

	UPROPERTY(Replicatedusing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();
	UFUNCTION()
	void UpdateAmmoValues();

	float RecoilAmount = 0.f;

public:	
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	FORCEINLINE int32 GetMaxGrenades() const { return MaxGrenades; }
	int32 GetMaxAmmoForWeapon(EWeaponType WeaponType);
	FORCEINLINE TMap<EWeaponType, int32> GetCurrentAmmoAmountForWeapon() const { return CarriedAmmoMap; }
	bool ShouldSwapWeapons();
};
