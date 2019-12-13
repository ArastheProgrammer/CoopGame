// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SRailGun.generated.h"

/**
 *
 */
UCLASS()
class COOPGAME_API ASRailGun : public ASWeapon
{
	GENERATED_BODY()

public:

	ASRailGun();

	virtual void BeginPlay() override;
	

	void StartFire() override;

	void StopFire() override;

	void ResetChargeTime();

	void Fire() override;

protected:

	
	FTimerHandle TimerHandle_Charge;

	UPROPERTY(EditDefaultsOnly, Category = "RailGun")
		bool CanShoot;
	UPROPERTY(ReplicatedUsing = OnRep_Boost, EditDefaultsOnly, Category = "RailGun")
		bool bIsBoosted;

		float DefaultDamage;

	UPROPERTY(Replicated,EditDefaultsOnly, Category = "RailGun")
		float DoubleDamageCounter;

	UPROPERTY(EditDefaultsOnly, Category = "RailGun")
		float ChargeTime;

	UPROPERTY(Replicated,EditDefaultsOnly, Category = "RailGun")
		float BoostDamage;
	
	UFUNCTION()
		void OnRep_Boost();
};