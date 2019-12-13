// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class USHealthComponent;
class UParticleSystem;
class UMaterial;
class URadialForceComponent;

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	 GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "ExplosiveBarrel")
		UStaticMeshComponent* MeshComp;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(VisibleAnywhere, Category = "ExplosiveBarrel")
		URadialForceComponent* RadialForceComp;

	UPROPERTY(EditDefaultsOnly, Category = "ExplosiveBarrel")
		float ExplosionImpulse;

	UPROPERTY(Replicated,EditDefaultsOnly, Category = "ExplosiveBarrel")
		UParticleSystem* ExplosionVFX;

	UPROPERTY(ReplicatedUsing = OnRep_Exploded,EditDefaultsOnly,Category = "ExplosiveBarrel")
		bool IsExplode;

	UPROPERTY(EditAnywhere, Category = "ExplosiveBarrel")
		UMaterial* BurnedMaterial;
	
	UFUNCTION()
		void OnRep_Exploded();

	
	void Explosion();

public:	
	

	
};
