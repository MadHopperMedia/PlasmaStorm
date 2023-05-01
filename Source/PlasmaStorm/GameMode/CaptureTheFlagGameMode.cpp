// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"
#include "PlasmaStorm/Weapon/Flag.h"
#include "PlasmaStorm/CaptureTheFlag/FlagZone.h"
#include "PlasmaStorm/GameState/PSGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(class APSCharacter* ElimmedCharacter, class APSPlayerController* VictimController, class APSPlayerController* AttackerController)
{
	APSGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

}

void ACaptureTheFlagGameMode::FlagCaptured(class AFlag* Flag, class AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	APSGameState* PSGameState = Cast<APSGameState>(GameState);
	if (PSGameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			PSGameState->BlueTeamScores();
		}
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			PSGameState->RedTeamScores();
		}
	}
}