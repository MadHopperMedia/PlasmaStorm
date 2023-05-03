// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenu.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "PlasmaStorm/character/PSCharacter.h"
#include "PlasmaStorm/PlayerController/PSPlayerController.h"

void UPauseMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		//PlayerController == nullptr ? Cast<APSPlayerController*>(World->GetFirstPlayerController()) : PlayerController;
		PlayerController = PlayerController == nullptr ? Cast<APSPlayerController>(World->GetFirstPlayerController()) : PlayerController;
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
			
		}
	}
	if (QuitButton && !QuitButton->OnClicked.IsBound())
	{
		QuitButton->OnClicked.AddDynamic(this, &UPauseMenu::QuitButtonClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiPlayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiPlayerSessionsSubsystem)
		{
			MultiPlayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UPauseMenu::OnDestroySession);
		}
	}
}

bool UPauseMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	return true;
}

void UPauseMenu::MenuTearDown()
{
	RemoveFromParent();

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? Cast<APSPlayerController>(World->GetFirstPlayerController()) : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;			
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if (QuitButton && QuitButton->OnClicked.IsBound())
	{
		QuitButton->OnClicked.RemoveDynamic(this, &UPauseMenu::QuitButtonClicked);
	}
	if (MultiPlayerSessionsSubsystem && MultiPlayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiPlayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UPauseMenu::OnDestroySession);
	}
}

void UPauseMenu::QuitButtonClicked()
{

	QuitButton->SetIsEnabled(false);

	UWorld* World = GetWorld();;;
	if (World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if (FirstPlayerController)
		{
			APSCharacter* PSCharacter = Cast<APSCharacter>(FirstPlayerController->GetPawn());
			if (PSCharacter)
			{
				PSCharacter->ServerLeaveGame();
				PSCharacter->OnLeftGame.AddDynamic(this, &UPauseMenu::OnPlayerLeftGame);
			}
			else
			{
				QuitButton->SetIsEnabled(true);
			}
		}			
	}	
}

void UPauseMenu::OnPlayerLeftGame()
{
	if (MultiPlayerSessionsSubsystem)
	{
		MultiPlayerSessionsSubsystem->DestroySession();
	}
}

void UPauseMenu::OnDestroySession(bool bWasSuccessful)
{

	if (!bWasSuccessful)
	{
		QuitButton->SetIsEnabled(true);
		return;
	}
	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode =  World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PlayerController = PlayerController == nullptr ? Cast<APSPlayerController>(World->GetFirstPlayerController()) : PlayerController;
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}