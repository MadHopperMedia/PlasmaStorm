// Fill out your copyright notice in the Description page of Project Settings.


#include "ElimAnnouncment.h"
#include "Components/TextBlock.h"

void UElimAnnouncment::SetElimAnnouncmentText(FString AttackerName, FString VictimName)
{
	FString ElimAnnouncmentText = FString::Printf(TEXT("%s Killed %s"), *AttackerName, *VictimName);
	if (AnnouncmentText)
	{
		AnnouncmentText->SetText(FText::FromString(ElimAnnouncmentText));
	}
}