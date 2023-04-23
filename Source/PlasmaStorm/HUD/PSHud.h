// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PSHud.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairsColor;

};

/**
 * 
 */
UCLASS()
class PLASMASTORM_API APSHud : public AHUD
{
	GENERATED_BODY()

public:

	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "PlayerStats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	UPROPERTY(EditAnywhere, Category = "Anouncements")
	TSubclassOf<class UUserWidget> AnnouncementClass;

	UPROPERTY()
	class UAnnouncement* Announcement;

	void AddAnnouncement();
	void AddElimAnnouncment(FString Attacker, FString Victim);
	void AddCharacterOverlay();

protected:

	virtual void BeginPlay() override;
	

private:
	UPROPERTY()
	class APlayerController* OwningPlayer;
	FHUDPackage HUDPackage;

	float CrosshairsOffset;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncment> ElimAnnouncmentClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncmentTime = 3.f;

	UFUNCTION()
	void ElimAnnouncmentTimerFinished(UElimAnnouncment* MsgToRemove);

	UPROPERTY()
	TArray<UElimAnnouncment*> ElimMessages;
public: 
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
	FORCEINLINE void SetCrosshairsOffset( float Offset) { CrosshairsOffset = Offset; }
	
};
