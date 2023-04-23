// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsPSGameMode.h"
#include "PlasmaStorm/GameState/PSGameState.h"
#include "PlasmaStorm/PlayerState/PSPlayerState.h"
#include "Kismet/Gameplaystatics.h"


ATeamsPSGameMode::ATeamsPSGameMode()
{
	bTeamsMatch = true;
}

void ATeamsPSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	APSGameState* PSGameState = Cast<APSGameState>(UGameplayStatics::GetGameState(this));
	if (PSGameState)
	{
		APSPlayerState* PSPState = NewPlayer->GetPlayerState<APSPlayerState>();
		if (PSPState && PSPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (PSGameState->BlueTeam.Num() >= PSGameState->RedTeam.Num())
			{
				PSGameState->RedTeam.AddUnique(PSPState);
				PSPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				PSGameState->BlueTeam.AddUnique(PSPState);
				PSPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsPSGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APSGameState* PSGameState = Cast<APSGameState>(UGameplayStatics::GetGameState(this));
	APSPlayerState* PSPState = Exiting->GetPlayerState<APSPlayerState>();
	if (PSGameState && PSPState)
	{
		if (PSGameState->RedTeam.Contains(PSPState))
		{
			PSGameState->RedTeam.Remove(PSPState);
		}

		if (PSGameState->BlueTeam.Contains(PSPState))
		{
			PSGameState->BlueTeam.Remove(PSPState);
		}		
	}
}

void ATeamsPSGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	APSGameState* PSGameState = Cast<APSGameState>(UGameplayStatics::GetGameState(this));
	if (PSGameState)
	{
		for (auto PState : PSGameState->PlayerArray)
		{
			APSPlayerState* PSPState = Cast<APSPlayerState>(PState.Get());
			if (PSPState && PSPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (PSGameState->BlueTeam.Num() >= PSGameState->RedTeam.Num())
				{
					PSGameState->RedTeam.AddUnique(PSPState);
					PSPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					PSGameState->BlueTeam.AddUnique(PSPState);
					PSPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}		
	}
}
