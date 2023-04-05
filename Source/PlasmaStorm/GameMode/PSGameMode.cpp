// Fill out your copyright notice in the Description page of Project Settings.


#include "PSGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlasmaStorm/PlayerState/PSPlayerState.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "PlasmaStorm/GameState/PSGameState.h"
#include "PlasmaStorm/PlayerController/PSPlayerController.h"

namespace MatchState
{
	const FName Cooldown = FName("CoolDown"); 
}

APSGameMode::APSGameMode()
{
	bDelayedStart = true;
}

void APSGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void APSGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void APSGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APSPlayerController* PSPlayer = Cast<APSPlayerController>(*It);
		if (PSPlayer)
		{
			PSPlayer->OnMatchStateSet(MatchState);
		}
	}

}

void APSGameMode::PlayerEliminated(class APSCharacter* ElimmedCharacter, class APSPlayerController* VictimController, class APSPlayerController* AttackerController)
{
	APSPlayerState* AttackerPlayerState = AttackerController ? Cast<APSPlayerState>(AttackerController->PlayerState) : nullptr;
	APSPlayerState* VictimPlayerState = VictimController ? Cast<APSPlayerState>(VictimController->PlayerState) : nullptr;

	

	APSGameState* PSGameState = GetGameState<APSGameState>();
	if (MatchState == MatchState::InProgress)
	{
		if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && PSGameState)
		{
			AttackerPlayerState->AddToScore(1.f);
			PSGameState->UpdateTopScore(AttackerPlayerState);
		}

		if (VictimPlayerState)
		{
			VictimPlayerState->AddToDefeats(1.f);
		}
	}	

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}

void APSGameMode::RequestRespawn(APawn* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() -1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
		
	}
	
}