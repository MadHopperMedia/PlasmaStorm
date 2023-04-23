// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElimAnnouncment.generated.h"

/**
 * 
 */
UCLASS()
class PLASMASTORM_API UElimAnnouncment : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetElimAnnouncmentText(FString AttackerName, FString VictimName);
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* AnnouncmentBox;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AnnouncmentText;
};
