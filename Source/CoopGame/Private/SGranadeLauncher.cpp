// Fill out your copyright notice in the Description page of Project Settings.

#include "SGranadeLauncher.h"
#include "SWeapon.h"
#include "SProjectile.h"
#include "SCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

void ASGranadeLauncher::Fire()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;

		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);
	}

	LastFireTime = GetWorld()->TimeSeconds;
}

