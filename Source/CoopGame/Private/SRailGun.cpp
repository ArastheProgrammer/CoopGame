// Fill out your copyright notice in the Description page of Project Settings.

#include "SRailGun.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "CoopGame.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine.h"
#include "Console.generated.h"
#include "Net/UnrealNetwork.h"

ASRailGun::ASRailGun()
{
	ChargeTime = 1.3f;
	CanShoot = false;


	BaseDamage = 80.0f;
	BoostDamage = 160.0f;

	DefaultDamage = BaseDamage;

}

void ASRailGun::BeginPlay()
{

	Super::BeginPlay();
	DoubleDamageCounter = 0;
	bIsBoosted = false;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("DefaultDamage:  %f"), DefaultDamage));
}



void ASRailGun::StartFire()
{
	//Charge Time 	
	GetWorldTimerManager().SetTimer(TimerHandle_Charge, this, &ASRailGun::ResetChargeTime, ChargeTime, false);
}

void ASRailGun::ResetChargeTime()
{
	CanShoot = true;
	UE_LOG(LogTemp, Log, TEXT("Charged"));
	DoubleDamageCounter++;

	GetWorldTimerManager().SetTimer(TimerHandle_Charge, this, &ASRailGun::ResetChargeTime, ChargeTime, false);

	if (DoubleDamageCounter > 2)
	{
		//Not replicate base damage to client
		/*BaseDamage = BoostDamage;*/
		OnRep_Boost();
		Fire();
		GetWorldTimerManager().ClearTimer(TimerHandle_Charge);
		BaseDamage = DefaultDamage;
		DoubleDamageCounter = 0;
		CanShoot = false;

	}


}

void ASRailGun::Fire()
{
	OnRep_Boost();
	Super::Fire();

}

void ASRailGun::OnRep_Boost()
{
	if (bIsBoosted)
	{
		BaseDamage = BoostDamage;
	}
	else
	{
		BaseDamage = 80.0f;
	}

}

void ASRailGun::StopFire()
{
	if (CanShoot)
	{
		CanShoot = false;

		bIsBoosted = DoubleDamageCounter == 2;

		UE_LOG(LogTemp, Log, TEXT("Launched"));
		if (DoubleDamageCounter == 2)
		{
			OnRep_Boost();

		}
		UE_LOG(LogTemp, Log, TEXT("Sub class Base Damage is: %s"), *FString::SanitizeFloat(BaseDamage));
		Fire();

		BaseDamage = 80;
		bIsBoosted = false;
		DoubleDamageCounter = 0;

	}
	GetWorldTimerManager().ClearTimer(TimerHandle_Charge);
}

void ASRailGun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASRailGun, BaseDamage);
	DOREPLIFETIME(ASRailGun, BoostDamage);
	DOREPLIFETIME(ASRailGun, bIsBoosted);
	DOREPLIFETIME(ASRailGun, DoubleDamageCounter);
}

