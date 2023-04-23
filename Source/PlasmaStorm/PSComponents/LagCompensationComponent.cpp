// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PlasmaStorm/Weapon/Weapon.h"
#include "PlasmaStorm/PlasmaStorm.h"


ULagCompensationComponent::ULagCompensationComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;
	
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SaveFramePackage();
	
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		//ShowFramePackage(ThisFrame, FColor::Red);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<APSCharacter>(GetOwner()) : Character;
	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}

}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage Package, const FColor Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			100.f
		);
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;

		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;

		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}

void ULagCompensationComponent::CacheBoxPositions(APSCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(APSCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			const FBoxInformation* BoxValue = Package.HitBoxInfo.Find(HitBoxPair.Key);
			if (BoxValue)
			{
				HitBoxPair.Value->SetWorldLocation(BoxValue->Location);
				HitBoxPair.Value->SetWorldRotation(BoxValue->Rotation);
				HitBoxPair.Value->SetBoxExtent(BoxValue->BoxExtent);
			}
		}
	}

}

void ULagCompensationComponent::ResetHitBoxes(APSCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			const FBoxInformation* BoxValue = Package.HitBoxInfo.Find(HitBoxPair.Key);

			if (BoxValue)
			{
				HitBoxPair.Value->SetWorldLocation(BoxValue->Location);
				HitBoxPair.Value->SetWorldRotation(BoxValue->Rotation);
				HitBoxPair.Value->SetBoxExtent(BoxValue->BoxExtent);
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(APSCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

FServerSideRewindResault ULagCompensationComponent::ServerSideRewind(APSCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmedHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FServerSideRewindResault ULagCompensationComponent::ProjectileServerSideRewind(APSCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime, AWeapon* DamageCauser)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

	return ProjectileConfirmedHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FShotgunServerSideRewindResault ULagCompensationComponent::ShotgunServerSideRewind(const TArray<APSCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for (APSCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}

	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FServerSideRewindResault ULagCompensationComponent::ConfirmedHit(const FFramePackage& Package, APSCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr)return FServerSideRewindResault();

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	if (!HitCharacter->IsElimmed())
	{
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	}

	//
	// enable collision for the head first

	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + ((HitLocation - TraceStart) * 1.25f) + ((HitCharacter->PlayerVelocity() / 600) * 49); //  + (HitCharacter->GetActorUpVector() * -100)

	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);
		if (bShowTraceCheck)
		{
			DrawDebugSphere(World, TraceEnd, 5, 12, FColor::Yellow, true, 100);
			DrawDebugLine(World, TraceStart, TraceEnd, FColor::Red, true, 100);
			ShowFramePackage(CurrentFrame, FColor::Cyan);
		}

		if (ConfirmHitResult.bBlockingHit) // We hit the head return early
		{
			if (ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if (Box)
				{
					//DrawDebugBox(World, Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
				}
			}

			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResault{ true, true };
		}
		else // Didnt hit the head, check the rest of the boxes
		{
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}			
			}
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			if (ConfirmHitResult.bBlockingHit)
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if (Box)
				{
					//DrawDebugBox(World, Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
				}
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResault{ true, false };
			}
		}

	}
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResault{ false, false };
}

FServerSideRewindResault ULagCompensationComponent::ProjectileConfirmedHit(const FFramePackage& Package, APSCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString(TEXT("Testing Hit")));
	}
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	if (!HitCharacter->IsElimmed())
	{
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	}

	//
	// enable collision for the head first

	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	PathParams.OverrideGravityZ = 0.001f;
	//PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::None;
	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	if (PathResult.HitResult.bBlockingHit) // We hit the head return early
	{
		if (PathResult.HitResult.Component.IsValid())
		{
			UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			if (Box)
			{
				//DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
			}
		}

		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FServerSideRewindResault{ true, true };
	}
	else // We didn't hit the head, check the rest of the boxes
	{
		for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}		
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if (PathResult.HitResult.bBlockingHit)
		{
			if (PathResult.HitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
				if (Box)
				{
					//DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
				}
			}

			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResault{ true, false };
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResault{ false, false };
}

FShotgunServerSideRewindResault ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : FramePackages)
	{
		if (Frame.Character == nullptr) return FShotgunServerSideRewindResault();
	}

	FShotgunServerSideRewindResault ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurrentFrame);
		MoveBoxes(Frame.Character, Frame);
		if (!Frame.Character->IsElimmed())
		{
			EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		}
		CurrentFrames.Add(CurrentFrame);
	}

	for (auto& Frame : FramePackages)
	{
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}
	UWorld* World = GetWorld();
	// Check for headshots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + ((HitLocation - TraceStart) * 1.25f);

		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			APSCharacter* PSCharacter = Cast<APSCharacter>(ConfirmHitResult.GetActor());
			if (PSCharacter)
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if (Box)
				{
					//DrawDebugBox(World, Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
				}
				if (ShotgunResult.HeadShots.Contains(PSCharacter))
				{
					ShotgunResult.HeadShots[PSCharacter]++;
				}
				else
				{
					ShotgunResult.HeadShots.Emplace(PSCharacter, 1);
				}
			}
		}
	}

	// Enable collision for all boxes then disable collision for head boxes
	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
			UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
			HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + ((HitLocation - TraceStart) * 1.25f);

		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			APSCharacter* PSCharacter = Cast<APSCharacter>(ConfirmHitResult.GetActor());
			if (PSCharacter)
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if (Box)
				{
					//DrawDebugBox(World, Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
				}
				if (ShotgunResult.BodyShots.Contains(PSCharacter))
				{
					ShotgunResult.BodyShots[PSCharacter]++;
				}
				else
				{
					ShotgunResult.BodyShots.Emplace(PSCharacter, 1);
				}
			}
		}
	}
	for (auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}
	return ShotgunResult;
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(APSCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;

	if (bReturn) return FFramePackage();
	// frame package the we check to verity a hit
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	// Frame history of the hit character
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	if (OldestHistoryTime > HitTime)
	{
		// to far back - to laggy to do ssr
		return FFramePackage();
	}
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	while (Older->GetValue().Time > HitTime) // is older still younger than hit time?
	{
		// March back untill oldertime < HitTime < YoungerTime
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if (Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	if (bShouldInterpolate)
	{
		// interpolate between younger and older
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(APSCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResault Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (Character &&HitCharacter && Confirm.bHitConfirmed && DamageCauser)
	{
		const float Damage = Confirm.bHeadShot ? DamageCauser->GetHeadshotDamage() : DamageCauser->GetDamage();
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			DamageCauser,
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(APSCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResault Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime, DamageCauser);

	if (Character && HitCharacter && Confirm.bHitConfirmed)
	{
		const float Damage = Confirm.bHeadShot ? DamageCauser->GetHeadshotDamage() : DamageCauser->GetDamage();
		
		AWeapon* CurrentWeapon = Character->GetEquippedWeapon();
		
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			CurrentWeapon,
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<APSCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FShotgunServerSideRewindResault Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (auto& HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr || HitCharacter->GetEquippedWeapon() == nullptr || Character == nullptr) continue;
		float TotalDamage = 0.f;
		if (Confirm.HeadShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Confirm.HeadShots[HitCharacter] * Character->GetEquippedWeapon()->GetHeadshotDamage();
			TotalDamage += HeadShotDamage;
		}
		if (Confirm.BodyShots.Contains(HitCharacter))
		{
			float BodyShotDamage = Confirm.BodyShots[HitCharacter] * Character->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;
		}
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

