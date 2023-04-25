// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "InitialState"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "EquippedSecondary"),
	EWS_EquippedMountedWeapon UMETA(DisplayName = "EquippedMountedWeapon"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

EWS_Max UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};


UCLASS()
class PLASMASTORM_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AWeapon();
	friend class UCombatComponent;
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	virtual void Fire(const FVector& HitTarget);
	virtual void Dropped();
	void SetHUDAmmo();
	void AddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .15f;
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;
	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	UPROPERTY(EditAnywhere)
	class USoundCue* ZoomSound;

	UPROPERTY(EditAnywhere)
	class USoundCue* UnZoomSound;
	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category = "WeaponScatter")
	bool bUseScatter = false;
	FVector TraceEndWithScatter(const FVector& HitTarget);
	bool bCanRecharge = false;
	float AddedAmmo = 0;
protected:
	
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();
	virtual void OnEquippedMountedWeapon();
	virtual void DroppedTimerFinished();

	FTimerHandle DroppedTimer;
	UPROPERTY(EditDefaultsOnly)
		float DroppedDelay = 20.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float WeaponRange = 30000.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float TraceRadious = 30.f;
	UPROPERTY()
	class APSCharacter* Target;
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	bool bIsAiming = false;
	UPROPERTY(EditAnywhere)
	FName WeaponName;

	/**
* Trace end with scatter
*/
	UPROPERTY(EditAnywhere, Category = "WeaponScatter")
	float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere, Category = "WeaponScatter")
	float SphereRadious = 80.f;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	class APSCharacter* PSOwnerCharacter;
	UPROPERTY()
	class APSPlayerController* PSOwnerController;
	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

	UPROPERTY(EditAnywhere)
	bool bIsMountedWeapon = false;
	
	UPROPERTY(EditAnywhere)
	float RechargeTime = 1;

	UFUNCTION()
	void RecharageAmmo(float DeltaTime);

private:

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;
	UFUNCTION()
	void OnRep_WeaponState();
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TSubclassOf<class ACasing> CasingClass;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomedFOV = 30.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomInterpSpeed = 20.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Recoil = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	int32 Ammo;	

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	void SpendRound();

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	int32 MagCapacity;

	// Number of unprocessed server requests for ammo.
	// incremented in spend round, decremented in client update ammo.
	int32 Sequence = 0;

	
	


	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
	
	

public:	
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }	
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE float GetWeaponRange() const { return WeaponRange; }
	bool IsEmpty();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetWeaponRecoil() const { return Recoil; }
	FORCEINLINE float GetTraceRadious() const { return TraceRadious; }
	FORCEINLINE void SetTarget(APSCharacter* CurrentTarget) { Target = CurrentTarget; }
	FORCEINLINE void GetIsAiming(bool Aiming) { bIsAiming = Aiming; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FName GetWeaponName() const { return WeaponName; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadshotDamage() const { return HeadShotDamage; }
};
