// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PSPlayerController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingToHigh);

/**
 * 
 */
UCLASS()
class PLASMASTORM_API APSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDStamina(float Stamina, float MaxStamina);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCooldown();
	UPROPERTY()
	class APSCharacter* PSCharacter;
	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;
	
protected:

	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	/**
	*Sync time between client and server
	*/
	UFUNCTION(Server,Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimerServerRecievedClientRequest);

	
	float ClientServerDelta = 0;
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float WarmUp, float Match, float Cooldown, float StartingTime);

	

	UFUNCTION(BlueprintCallable)
	void SetInverted(bool Inverted);
	UFUNCTION(BlueprintCallable)
	void SetToggleBoost(bool ToggleBoost);

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);
	

private:
	UPROPERTY()
	class APSHud* PSHUD;
	UPROPERTY()
	class APSGameMode* PSGameMode;
	float MatchTime = 0.f;
	float WarmUpTime = 0.f;
	float CooldownTime = 0.f;
	float LevelStartingTime = .0f;
	uint32 CountDownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	bool bInitializeHealth = false;
	float HUDMaxHealth;
	float HUDScore;
	bool bInitializeScore = false;
	int32 HUDDefeats;
	bool bInitializeDefeats = false;
	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
	int32 HUDGrenades;
	bool bInitializeGrenades = false;
	float HUDShield;
	bool bInitializeShield = false;
	float HUDMaxShield;

	float HUDStamina;
	bool bInitializeStamina = false;
	float HUDMaxStamina;

	float HighPingRunningTime = 0.f;
	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	float PingAnimationRunningtime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrenquency = 20.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
	
	
public:
	UPROPERTY(BlueprintReadWrite)
	bool bInverted = false;
	UPROPERTY(BlueprintReadWrite)
	bool bToggleBoost = false;
	
	
	
};
