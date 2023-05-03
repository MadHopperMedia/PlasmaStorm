// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsPSGameMode.h"
#include "PlasmaStorm/GameState/PSGameState.h"
#include "PlasmaStorm/PlayerState/PSPlayerState.h"
#include "PlasmaStorm//PlayerController/PSPlayerController.h"
#include "Kismet/Gameplaystatics.h"


ATeamsPSGameMode::ATeamsPSGameMode()
{
	bTeamsMatch = true;
}

void ATeamsPSGameMode::PostLogin(APlayerController* NewPlayer)
{
	
	Super::PostLogin(NewPlayer);
	
	//float SortingType = FMath::FRandRange(0, 20); FMath::Rand()%20;

	int32 SortingType = FMath::Rand()%20;
	APSGameState* PSGameState = Cast<APSGameState>(UGameplayStatics::GetGameState(this));
	if (PSGameState)
	{
		if (SortingType > 5 && SortingType <= 10 || SortingType > 15)
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
		else
		{
			APSPlayerState* PSPState = NewPlayer->GetPlayerState<APSPlayerState>();
			if (PSPState && PSPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (PSGameState->BlueTeam.Num() <= PSGameState->RedTeam.Num())
				{
					PSGameState->BlueTeam.AddUnique(PSPState);
					PSPState->SetTeam(ETeam::ET_BlueTeam);
				}
				else
				{
					PSGameState->RedTeam.AddUnique(PSPState);
					PSPState->SetTeam(ETeam::ET_RedTeam);
				}
			}
		}
		
	}
}

void ATeamsPSGameMode::PlayerEliminated(class APSCharacter* ElimmedCharacter, class APSPlayerController* VictimController, class APSPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	APSGameState* PSGameState = Cast<APSGameState>(UGameplayStatics::GetGameState(this));
	APSPlayerState* AttackerPlayerState = AttackerController ? Cast<APSPlayerState>(AttackerController->PlayerState) : nullptr;
	APSPlayerState* VictimPlayerState = VictimController ? Cast<APSPlayerState>(VictimController->PlayerState) : nullptr;
	if (PSGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam && VictimPlayerState->GetTeam() != ETeam::ET_BlueTeam)
		{
			PSGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam && VictimPlayerState->GetTeam() != ETeam::ET_RedTeam)
		{
			PSGameState->RedTeamScores();
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



