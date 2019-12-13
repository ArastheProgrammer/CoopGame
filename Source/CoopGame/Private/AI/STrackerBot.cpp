// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "Components/SphereComponent.h"
#include "SCharacter.h"
#include "Sound/SoundCue.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(
	TEXT("COOP.DebugTrackerBotDrawing"),
	DebugTrackerBotDrawing,
	TEXT("Draw Debug Lines for TrackerBot"),
	ECVF_Cheat);

// Sets default values
ASTrackerBot::ASTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	//Only want to use raycast overlap etc query only is good more infor shows on the editor panel
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SphereComp->SetupAttachment(RootComponent);

	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;
	bExploded = false;
	ExplosionDamage = 20;
	ExplosionRadius = 350;
	SelfDamageInterval = 0.25f;

}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	//Find initial move-to
	if (Role == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint();

		//Every 1 seconds bots detect area will updating
		FTimerHandle TimerHandle_CheckPowerLevel;
		GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ASTrackerBot::OnCheckNearbyBots, 1.0f, true);


	}


}


void ASTrackerBot::HandleTakeDamage(USHealthComponent * OwningHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	//Explode on hitpoints == 0
	//UE_LOG(LogTemp, Log, TEXT("Health: %s"), *FString::SanitizeFloat(Health));

	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{	//parameter name of the editor we create right mouse click and named it
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}


	//UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());

	if (Health <= 0)
	{

		SelfDestruct();
	}

}

FVector ASTrackerBot::GetNextPathPoint()
{
	/*ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);*/

	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		//Eleman atamasý yaptýk
		APawn* TestPawn = It->Get();
		// Player tarafýndan kontrol edilmiyorsa veya yoksa bakmýyoruz
		if (TestPawn == nullptr || USHealthComponent::IsFriendly(TestPawn, this))
		{
			continue;
		}

		USHealthComponent* TestCompHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));

		if (HealthComp && HealthComp->GetHealth() > 0.0f)
		{
			if (NearestTargetDistance)
			{
				float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

				if (Distance < NearestTargetDistance)
				{
					BestTarget = TestPawn;
					NearestTargetDistance = Distance;
				}
			}

		}

	}

	if (BestTarget)
	{
		UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath, 5.0f, false);

		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			//Return next point in the path
			return NavPath->PathPoints[1];
		}


	}





	//Failed to find path
	return GetActorLocation();
}

void ASTrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}
	bExploded = true;


	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
	//Waiting to Destroy(SetLifeSpan) so we need to invisible for 2 sec which includes aperiance and collision
	this->MeshComp->SetVisibility(false, true);
	this->MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);
		UE_LOG(LogTemp, Log, TEXT("ActualDamage value is: %s"), *FString::SanitizeFloat(ActualDamage));
		//Apply Damage!
		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		if (DebugTrackerBotDrawing)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);
		}



		//Delete Actor immediately
		SetLifeSpan(2.0f);
	}

}



void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



	//ASTrackerBot* TrackerBot = Cast<ASTrackerBot>(OverlapingActor);
	if (Role == ROLE_Authority && !bExploded)
	{
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();
		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();

			if (DebugTrackerBotDrawing)
			{
				DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
			}
		}
		else
		{
			//Keep moving towards next target
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			if (DebugTrackerBotDrawing)
			{
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
			}
		}



		//UE_LOG(LogTemp, Log, TEXT("Group up with me damage increase %s"), *FString::SanitizeFloat(ExplosionDamage));
		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 4.0f, 1.0f);
	}

}

void ASTrackerBot::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	if (!bStartedSelfDestruction && !bExploded)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);


		//We overlapped with a player!!
		if (PlayerPawn && !USHealthComponent::IsFriendly(OtherActor, this))
		{

			if (Role == ROLE_Authority)
			{
				//Start self destruction sequence SelftDamageInterval hangi sýklýkla metodu çaðýdýgýmýzý belirler, self damage intervvalda o sýklýgýn parametresidir
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}


			bStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);

		}


	}

}

void ASTrackerBot::OnCheckNearbyBots()
{
	//Distance to check nearby bots
	const float Radius = 600;
	//Create temporary collision shape for overlaps

	CollShape.SetSphere(Radius);

	//Only find Pawns (eg. players and AI bots)


	//Our tracker bot's mesh component is set to Physics Body in Blueprint (default profile of physics simulated actors)

	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

	if (DebugTrackerBotDrawing)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.0f);
	}
	int32 NrOfBots = 0;
	//loop over the results using a "range based for loop"
	for (FOverlapResult Result : Overlaps)
	{
		//Check if we overlapped with another tracker bot (ignoring players and other bot types)
		ASTrackerBot* Bot = Cast<ASTrackerBot>(Result.GetActor());
		//Ignore this trackerbot instance
		if (Bot && Bot != this)
		{
			NrOfBots++;
		}
	}

	const int32 MaxPowerLevel = 4;

	//Clamp between min = 0 and max = 4
	PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInst)
	{

		float Alpha = PowerLevel / (float)MaxPowerLevel;

		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}

	if (DebugTrackerBotDrawing)
	{
		DrawDebugString(GetWorld(),FVector(0,0,0),FString::FromInt(PowerLevel),this,FColor::White,1.0f,true);
	}

}





void ASTrackerBot::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}

/*
TArray<AActor*> OverlapingActors;
	GetOverlappingActors(OverlapingActors, TSubclassOf<ASTrackerBot>());

for (int i = 0; i < OverlapingActors.Num(); i++)
		{
			//Damage Increase
			ExplosionDamage = 100;
			MatDamageInc->SetScalarParameterValue("DamageIncrease", GetWorld()->TimeSeconds);
		}

		if (OverlapingActors.Num() == 0)
		{
			ExplosionDamage = 20;
			MatDamageInc = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
		}*/