// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SGranadeLauncher.generated.h"

/**
 * 
 */


UCLASS()
class COOPGAME_API ASGranadeLauncher : public ASWeapon
{
	GENERATED_BODY()

		virtual void Fire() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "ProjectileWeapon")
		TSubclassOf<AActor> ProjectileClass;
	
	
};
