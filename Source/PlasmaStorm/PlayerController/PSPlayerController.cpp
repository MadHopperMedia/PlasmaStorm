// Fill out your copyright notice in the Description page of Project Settings.


#include "PSPlayerController.h"
#include "PlasmaStorm/HUD/PSHud.h"
#include "PlasmaStorm/HUD/CharacterOverlay.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Net/UnrealNetwork.h"
#include "PlasmaStorm/HUD/Announcement.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "PlasmaStorm/GameMode/PSGamemode.h"
#include "PlasmaStorm/GameState/PSGameState.h"
#include "PlasmaStorm/PlayerState/PSPlayerState.h"
#include "PlasmaStorm/PSComponents/CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/Image.h"
#include "PlasmaStorm/HUD/PauseMenu.h"




void APSPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	PSHUD = Cast<APSHud>(GetHUD());
	ServerCheckMatchState();
	
}

void APSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent == nullptr) return;

	InputComponent->BindAction("Quit", IE_Pressed, this, &APSPlayerController::ShowPauseMenu);

}

void APSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APSPlayerController, MatchState);
}

void APSPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);
	
}

void APSPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrenquency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold) // Ping is compressed; it's actually ping / 4
			{
				HighPingWarning();
				PingAnimationRunningtime = 0.f;
				//ServerReportPingStatus(true);
			}
			else
			{
				//ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	if (PSHUD && PSHUD->CharacterOverlay && PSHUD->CharacterOverlay->HighPingAnimation && PSHUD->CharacterOverlay->IsAnimationPlaying(PSHUD->CharacterOverlay->HighPingAnimation))
	{
		PingAnimationRunningtime += DeltaTime;
		if (PingAnimationRunningtime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void APSPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	//HighPingDelegate.Broadcast(bHighPing);
}

void APSPlayerController::ShowPauseMenu()
{
	if (PauseMenuWidget == nullptr) return;

	if (PauseMenu == nullptr)
	{
		PauseMenu = CreateWidget<UPauseMenu>(this, PauseMenuWidget);
	}
	if (PauseMenu)
	{
		bPauseMenuOpen = !bPauseMenuOpen;
		if (bPauseMenuOpen)
		{
			PauseMenu->MenuSetup();
		}
		else
		{
			PauseMenu->MenuTearDown();
		}
	}

}

void APSPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (PSHUD && PSHUD->CharacterOverlay)
		{
			CharacterOverlay = PSHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				ServerCheckMatchState();				
				if (bInitializeHealth)SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield)SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeStamina)SetHUDStamina(HUDStamina, HUDMaxStamina);
				if (bInitializeScore)SetHUDScore(HUDScore);
				if (bInitializeDefeats)SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
				

				PSCharacter = PSCharacter == nullptr ? Cast<APSCharacter>(GetPawn()) : PSCharacter;
				if (PSCharacter && PSCharacter->GetCombat())
				{
					//if (bInitializeGrenades)SetHUDGrenades(PSCharacter->GetCombat()->GetGrenades());
					SetHUDGrenades(PSCharacter->GetCombat()->GetGrenades());
					PSCharacter->SetInverted(bInverted);
					PSCharacter->SetToggleBoost(bToggleBoost);
				}
				
			}
		}
	}
}

void APSPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void APSPlayerController::ServerCheckMatchState_Implementation()
{
	APSGameMode* GameMode = Cast<APSGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmUpTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmUpTime, MatchTime, CooldownTime, LevelStartingTime);

		if (PSHUD && MatchState == MatchState::WaitingToStart)
		{
			PSHUD->AddAnnouncement();
		}
	}
}

void APSPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float WarmUp, float Match, float Cooldown, float StartingTime)
{
	WarmUpTime = WarmUp;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);

	if (PSHUD && MatchState == MatchState::WaitingToStart)
	{
		PSHUD->AddAnnouncement();
	}
}

void APSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PSCharacter = Cast<APSCharacter>(InPawn);
	if (PSCharacter)
	{
		SetHUDHealth(PSCharacter->GetHealth(), PSCharacter->GetMaxHealth());
	}

}

void APSPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD && PSHUD->CharacterOverlay && PSHUD->CharacterOverlay->HealthBar)
	{
		const float HealthPercent = Health / MaxHealth;
		PSHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void APSPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD && PSHUD->CharacterOverlay && PSHUD->CharacterOverlay->ShieldBar)
	{
		const float ShieldPercent = Shield / MaxShield;
		PSHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void APSPlayerController::SetHUDStamina(float Stamina, float MaxStamina)
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD && PSHUD->CharacterOverlay && PSHUD->CharacterOverlay->StaminaBar)
	{
		const float StaminaPercent = Stamina / MaxStamina;
		PSHUD->CharacterOverlay->StaminaBar->SetPercent(StaminaPercent);
	}
	else
	{
		bInitializeStamina = true;
		HUDStamina = Stamina;
		HUDMaxStamina = MaxStamina;
	}
}

void APSPlayerController::SetHUDScore(float Score)
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD && PSHUD->CharacterOverlay && PSHUD->CharacterOverlay->ScoreAmount)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		PSHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void APSPlayerController::SetHUDDefeats(int32 Defeats)
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD && PSHUD->CharacterOverlay && PSHUD->CharacterOverlay->DefeatsAmount)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		PSHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void APSPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	if (PSCharacter && PSCharacter->GetIsFlying()) return;
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD && PSHUD->CharacterOverlay && PSHUD->CharacterOverlay->WeaponAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PSHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void APSPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD && PSHUD->CharacterOverlay && PSHUD->CharacterOverlay->CarriedAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PSHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void APSPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD && PSHUD->CharacterOverlay && PSHUD->CharacterOverlay->MatchCountdownText)
	{
		if (CountdownTime < 0.f)
		{
			PSHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"),Minutes, Seconds);
		PSHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void APSPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD && PSHUD->Announcement && PSHUD->Announcement->WarmupTime)
	{
		if (CountdownTime < 0.f)
		{
			PSHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PSHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void APSPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;

	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmUpTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = WarmUpTime + MatchTime + CooldownTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		PSGameMode = PSGameMode == nullptr ? Cast<APSGameMode>(UGameplayStatics::GetGameMode(this)) : PSGameMode;
		if (PSGameMode)
		{
			SecondsLeft = FMath::CeilToInt(PSGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}

	if (CountDownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}	
	}
	CountDownInt = SecondsLeft;
}

void APSPlayerController::SetHUDGrenades(int32 Grenades)
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD && PSHUD->CharacterOverlay && PSHUD->CharacterOverlay->GrenadesText)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		PSHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void APSPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReciept = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReciept);
}

void APSPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimerServerRecievedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimerServerRecievedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float APSPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();	
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void APSPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void APSPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
		if (IsLocalController())
		{
			ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
		if (IsLocalController())
		{
			ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		}
	}
}

void APSPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
		if (IsLocalController())
		{
			ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		}
		
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
		if (IsLocalController())
		{
			ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		}
		
	}
}

void APSPlayerController::HandleMatchHasStarted()
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD)
	{
		if(PSHUD->CharacterOverlay == nullptr) PSHUD->AddCharacterOverlay();
		if (PSHUD->Announcement)
		{
			PSHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void APSPlayerController::HandleCooldown()
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD)
	{
		PSHUD->CharacterOverlay->RemoveFromParent();
		if (PSHUD && PSHUD->Announcement == nullptr)
		{
			PSHUD->AddAnnouncement();
		}
		if (PSHUD->Announcement && PSHUD->Announcement->AnnouncementText && PSHUD->Announcement->InfoText)
		{
			PSHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In:");
			PSHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			APSGameState* PSGameState = Cast<APSGameState>(UGameplayStatics::GetGameState(this));
			APSPlayerState* PSPlayerState = GetPlayerState<APSPlayerState>();
			if (PSGameState && PSPlayerState)
			{
				TArray<APSPlayerState*> TopPlayers = PSGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There Is No Winner");
				}
				else if(TopPlayers.Num() == 1 && TopPlayers[0] == PSPlayerState)
				{
					InfoTextString = FString("You Are The Winner");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players Tied For The Win:\n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}

				PSHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}

			

		}
	}
}

void APSPlayerController::SetInverted(bool Inverted)
{
	bInverted = Inverted;
	PSCharacter = PSCharacter == nullptr ? Cast<APSCharacter>(GetPawn()) : PSCharacter;
	if (PSCharacter)
	{
		PSCharacter->SetInverted(bInverted);
	}
	
}

void APSPlayerController::SetToggleBoost(bool ToggleBoost)
{
	bToggleBoost = ToggleBoost;
	PSCharacter = PSCharacter == nullptr ? Cast<APSCharacter>(GetPawn()) : PSCharacter;
	if (PSCharacter)
	{
		PSCharacter->SetToggleBoost(ToggleBoost);
	}
		
}

void APSPlayerController::HighPingWarning()
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD && PSHUD->CharacterOverlay && PSHUD->CharacterOverlay->HighPingImage && PSHUD->CharacterOverlay->HighPingAnimation)
	{
		PSHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		PSHUD->CharacterOverlay->PlayAnimation(PSHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5.f);
	}
}

void APSPlayerController::StopHighPingWarning()
{
	PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
	if (PSHUD && PSHUD->CharacterOverlay && PSHUD->CharacterOverlay->HighPingImage && PSHUD->CharacterOverlay->HighPingAnimation)
	{
		PSHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (PSHUD->CharacterOverlay->IsAnimationPlaying(PSHUD->CharacterOverlay->HighPingAnimation))
		{
			PSHUD->CharacterOverlay->StopAnimation(PSHUD->CharacterOverlay->HighPingAnimation);
		}		
	}
}

void APSPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncment(Attacker, Victim);
}

void APSPlayerController::ClientElimAnnouncment_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		PSHUD = PSHUD == nullptr ? Cast<APSHud>(GetHUD()) : PSHUD;
		if (PSHUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				PSHUD->AddElimAnnouncment("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				PSHUD->AddElimAnnouncment(Attacker->GetPlayerName(), "You");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				PSHUD->AddElimAnnouncment("You", "Yourself");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				PSHUD->AddElimAnnouncment(Attacker->GetPlayerName(), "Themselves");
				return;
			}
			PSHUD->AddElimAnnouncment(Attacker->GetPlayerName(), Victim->GetPlayerName());			
		}
	}
}

