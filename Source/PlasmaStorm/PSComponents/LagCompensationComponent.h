// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

		UPROPERTY()
		FVector Location;

	UPROPERTY()
		FRotator Rotation;

	UPROPERTY()
		FVector BoxExtent;
};


USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	APSCharacter* Character;

};

USTRUCT(BlueprintType)
struct FServerSideRewindResault
{
	GENERATED_BODY()

		UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResault
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<APSCharacter*, uint32> HeadShots;
	UPROPERTY()
	TMap<APSCharacter*, uint32> BodyShots;
	

	UPROPERTY()
		bool bHeadShot;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PLASMASTORM_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	ULagCompensationComponent();
	friend class APSCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage Package, const FColor Color);

	/** 
	* Hitscan
	*/
	FServerSideRewindResault ServerSideRewind(
		class APSCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);

	/**
	* Projectile
	*/
	FServerSideRewindResault ProjectileServerSideRewind(
		class APSCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime);

	/**
	* Shotgun
	*/

	FShotgunServerSideRewindResault ShotgunServerSideRewind(
		const TArray<APSCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(APSCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime, 
		class AWeapon* DamageCauser);

	UFUNCTION(Server, Reliable)
		void ProjectileServerScoreRequest(APSCharacter* HitCharacter,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize100& InitialVelocity,
			float HitTime);
		

	UFUNCTION(Server, Reliable)
		void ShotgunServerScoreRequest(
			const TArray<APSCharacter*>& HitCharacters,
			const FVector_NetQuantize& TraceStart,
			const TArray<FVector_NetQuantize>& HitLocations,
			float HitTime
		);

	UPROPERTY(EditAnywhere)
	bool bShowTraceCheck = true;

protected:	
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	
	void CacheBoxPositions(APSCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(APSCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(APSCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(APSCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	FFramePackage GetFrameToCheck(APSCharacter* HitCharacter, float HitTime);

	/**
	* Hitscan
	*/
	FServerSideRewindResault ConfirmedHit(
		const FFramePackage& Package,
		APSCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	/**
	* Projectile
	*/
	FServerSideRewindResault ProjectileConfirmedHit(
		const FFramePackage& Package,
		class APSCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime);

	/**
	* Shotgun
	*/
	FShotgunServerSideRewindResault ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);

private:

	UPROPERTY()
	APSCharacter* Character;

	UPROPERTY()
	class APSPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:		

		
};
