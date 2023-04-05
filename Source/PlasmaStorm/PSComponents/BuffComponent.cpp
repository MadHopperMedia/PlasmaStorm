// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// ...
}


// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
	StaminaRampUp(DeltaTime);
	
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;

	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
	

}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishingShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
	if (Character)
	{
		Character->MulticastShieldRecharge();
	}
	
}

void UBuffComponent::StopReplenishingShield()
{
	bReplenishingShield = false;
}

void UBuffComponent::ReplenishStamina(float StaminaAmount, float StaminaTime)
{
	bReplenishingStamina = true;
	bDrainingStamina = false;
	StaminaReplenishRate = StaminaAmount / StaminaTime;
	StaminaReplenishAmount += StaminaAmount;
	
}

void UBuffComponent::DrainingStamina(float StaminaAmount, float StaminaTime)
{
	bDrainingStamina = true;
	bReplenishingStamina = false;
	StaminaDrainRate = StaminaAmount / StaminaTime;
	StaminaDrainAmount += StaminaAmount;
	
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing|| Character == nullptr || Character->IsElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;
	if (AmountToHeal < 0 || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}

}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishingShield || Character == nullptr || Character->IsElimmed()) return;

	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	ShieldReplenishAmount -= ReplenishThisFrame;
	if (ShieldReplenishAmount < 0 || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishingShield = false;
		ShieldReplenishAmount = 0.f;
	}

}

void UBuffComponent::StaminaRampUp(float DeltaTime)
{
	if (!bReplenishingStamina && !bDrainingStamina || Character == nullptr || Character->IsElimmed()) return;

	if (bReplenishingStamina || bDrainingStamina && !Character->GetbIsAccelerating())
	{
		const float ReplenishThisFrame = StaminaReplenishRate * DeltaTime;
		Character->SetStamina(FMath::Clamp(Character->GetStamina() + ReplenishThisFrame, 0, Character->GetMaxStamina()));
		Character->UpdateHUDStamina();
		StaminaReplenishAmount -= ReplenishThisFrame;
		if (StaminaReplenishAmount < 0 || Character->GetStamina() >= Character->GetMaxStamina())
		{
			bReplenishingStamina = false;
			StaminaReplenishAmount = 0.f;
		}
	}

	if (bDrainingStamina && Character->GetbIsAccelerating())
	{		
		StaminaReplenishAmount = 0.f;
		bReplenishingStamina = false;
		const float DrainThisFrame = StaminaDrainRate * DeltaTime;
		Character->SetStamina(FMath::Clamp(Character->GetStamina() - DrainThisFrame, 0, Character->GetMaxStamina()));
		Character->UpdateHUDStamina();
		StaminaDrainAmount -= DrainThisFrame;
		if (Character->GetStamina() <= 0 || Character->GetStamina() >= Character->GetMaxStamina())
		{
			bDrainingStamina = false;
			StaminaDrainAmount = 0.f;
			
			if (Character->GetStamina() < Character->GetMaxStamina())
			{
				bReplenishingStamina = true;
				Character->SetStopBoosting();
			}
		}
	}
	
}



