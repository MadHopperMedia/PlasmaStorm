// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "PlasmaStorm/Character/PSCharacter.h"
#include "PlasmaStorm/PlasmaStorm.h"
#include "PlasmaStorm/PlayerController/PSPlayerController.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SceneComponent.h"


// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

	

}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (Tracer)
	{
		TraceComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition	
		);
	}
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}

	/*APSCharacter* OwningCharacter = Cast<APSCharacter>(GetOwner());
	if (OwningCharacter)
	{
		CurrentTarget = OwningCharacter->GetTargetCharacter();
	}
		

	if (ProjectileMovementComponent && ProjectileMovementComponent->bIsHomingProjectile && CurrentTarget != nullptr)
	{
		ProjectileMovementComponent->HomingTargetComponent = CurrentTarget->GetTarget();
	}
	*/
}

void AProjectile::SetTarget(APSCharacter* Target)
{
	//CurrentTarget = Target;
	

}

void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{	
	APSCharacter* HitCharacter = Cast<APSCharacter>(OtherActor);
	if (OtherActor == HitCharacter)
	{
		HitCharacter->Impulse = GetVelocity() * ImpulsePower;
	}
	Destroy();
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::ExplodeDamage()
{	

	APawn* FireingPawn = GetInstigator();
	if (FireingPawn && HasAuthority())
	{
		AController* FireingController = FireingPawn->GetController();
		if (FireingController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // world context object
				Damage, // Base Damage
				10.f, // Minimum Damage
				GetActorLocation(), // Origin
				InnerRadious, // Damage inner radious
				OuterRadious, // Damage outer radious
				DamageFalloff, // Damage falloff
				UDamageType::StaticClass(), // Damage Type
				TArray<AActor*>(), // Actors to ignore
				this, // Damage Causer
				FireingController // Instigating controller
			);
		}
	}
}

void AProjectile::Destroyed()
{
	
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	AddImpulse();
	Super::Destroyed();
}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}



