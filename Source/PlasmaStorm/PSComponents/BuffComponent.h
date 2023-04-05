// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PLASMASTORM_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuffComponent();
	friend class APSCharacter;
	void Heal(float HealAmount, float HealingTime);
	void ReplenishShield(float ShieldAmount, float ReplenishTime);
	void StopReplenishingShield();
	void ReplenishStamina(float StaminaAmount, float StaminaTime);
	void DrainingStamina(float StaminaAmount, float StaminaTime);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);

	void ShieldRampUp(float DeltaTime);

	void StaminaRampUp(float DeltaTime);

	
	

private:

	UPROPERTY()
	class APSCharacter* Character;

	
	bool bReplenishingShield = false;
	UPROPERTY(EditAnywhere)
	float ShieldReplenishRate = 0.f;
	float ShieldReplenishAmount = 0.f;

	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0;	

	bool bReplenishingStamina = false;
	bool bDrainingStamina = false;
	float StaminaReplenishRate = 0.f;
	float StaminaReplenishAmount = 0;
	float StaminaDrainRate = 0.f;
	float StaminaDrainAmount = 0.f;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
