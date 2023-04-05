// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"

void UOverheadWidget::NativeDestruct()

{

	RemoveFromParent();

	Super::NativeDestruct();

}

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
	
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString Role;
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("Autonomous");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("Simulated");
		break;
	}

	FString LocalRoleString = FString::Printf(TEXT("Local Role %s"), *Role);
	SetDisplayText(LocalRoleString);
}