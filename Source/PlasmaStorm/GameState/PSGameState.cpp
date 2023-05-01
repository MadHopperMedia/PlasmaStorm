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
		ScoringPlayer->SetHasTheLead(true);
		if (TakenTheLeadFirstBlood && ScoringPlayer->GetTeam() == ETeam::ET_NoTeam)
		{
			ScoringPlayer->PlayAnnouncement(TakenTheLeadFirstBlood, AnnouncmentDelay);
		}		
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers[0]->SetHasTheLead(false);
		TopScoringPlayers.AddUnique(ScoringPlayer);
		if (TiedTheLead && ScoringPlayer->GetTeam() == ETeam::ET_NoTeam)
		{
			for (auto Players : TopScoringPlayers)
			{
				Players->PlayAnnouncement(TiedTheLead, AnnouncmentDelay);
			}
			
		}
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		if (!ScoringPlayer->GetHasTheLead())
		{
			if (LostTheLead && ScoringPlayer->GetTeam() == ETeam::ET_NoTeam)
			{
				TopScoringPlayers[0]->PlayAnnouncement(LostTheLead, AnnouncmentDelay);
			}
			if (TakenTheLead && ScoringPlayer->GetTeam() == ETeam::ET_NoTeam)
			{
				ScoringPlayer->PlayAnnouncement(TakenTheLead, AnnouncmentDelay);
				ScoringPlayer->SetHasTheLead(true);
			}
		}
		
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
	if (RedTeamScore > BlueTeamScore && RedHasTakenTheLead && bRedInLead == false)
	{
		bRedInLead = true;
		bBlueInLead = false;
		for (auto PSPState : RedTeam)
		{
			PSPState->PlayAnnouncement(RedHasTakenTheLead, AnnouncmentDelay);
		}
		for (auto PSPState : BlueTeam)
		{
			PSPState->PlayAnnouncement(RedHasTakenTheLead, AnnouncmentDelay);
		}
	}
	if (RedTeamScore == BlueTeamScore && TiedTheLead)
	{
		bRedInLead = false;
		bBlueInLead = false;
		for (auto PSPState : RedTeam)
		{
			PSPState->PlayAnnouncement(TiedTheLead, AnnouncmentDelay);
		}
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
	if (RedTeamScore < BlueTeamScore && BlueHasTakenTheLead && bBlueInLead == false)
	{
		bRedInLead = false;
		bBlueInLead = true;
		for (auto PSPState : BlueTeam)
		{
			PSPState->PlayAnnouncement(BlueHasTakenTheLead, AnnouncmentDelay);
		}
		for (auto PSPState : RedTeam)
		{
			PSPState->PlayAnnouncement(BlueHasTakenTheLead, AnnouncmentDelay);
		}
	}
	if (RedTeamScore == BlueTeamScore && TiedTheLead)
	{
		bRedInLead = false;
		bBlueInLead = false;
		for (auto PSPState : BlueTeam)
		{
			PSPState->PlayAnnouncement(TiedTheLead, AnnouncmentDelay);
		}
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