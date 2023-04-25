// Fill out your copyright notice in the Description page of Project Settings.


#include "PSGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlasmaStorm/PlayerState/PSPlayerState.h"
#include "PlasmaStorm//PlayerController/PSPlayerController.h"

void APSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	
	DOREPLIFETIME(APSGameState, TopScoringPlayers);
	DOREPLIFETIME(APSGameState, RedTeamScore);
	DOREPLIFETIME(APSGameState, BlueTeamScore);
}

void APSGameState::UpdateTopScore(class APSPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void APSGameState::RedTeamScores()
{
	++RedTeamScore;

	APSPlayerController* PSPlayer = Cast<APSPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PSPlayer)
	{
		PSPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void APSGameState::BlueTeamScores()
{
	++BlueTeamScore;

	APSPlayerController* PSPlayer = Cast<APSPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PSPlayer)
	{
		PSPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}


void APSGameState::OnRep_RedTeamScore()
{
	APSPlayerController* PSPlayer = Cast<APSPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PSPlayer)
	{
		PSPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void APSGameState::OnRep_BlueTeamScore()
{
	APSPlayerController* PSPlayer = Cast<APSPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PSPlayer)
	{
		PSPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}