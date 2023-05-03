// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu.generated.h"

/**
 * 
 */
UCLASS()
class PLASMASTORM_API UPauseMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	void MenuSetup();
	UFUNCTION(BlueprintCallable)
	void MenuTearDown();

protected:

	virtual bool Initialize() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnPlayerLeftGame();
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* QuitButton;
private:
		
	UFUNCTION()
	void QuitButtonClicked();

	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiPlayerSessionsSubsystem;

	UPROPERTY()
	class APSPlayerController* PlayerController;
	
};
