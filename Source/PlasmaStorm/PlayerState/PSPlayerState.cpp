// Fill out your copyright notice in the Description page of Project Settings.


#include "PSPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "PlasmaStorm/PlayerController/PSPlayerController.h"


void APSPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APSPlayerState, Defeats);
}

void APSPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);	

	Character = Character == nullptr ? Cast<APSCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<APSPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}		
	}
}
void APSPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<APSCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<APSPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}		
	}
}

void APSPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<APSCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<APSPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void APSPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<APSCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<APSPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}