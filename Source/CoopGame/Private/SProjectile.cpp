// Fill out your copyright notice in the Description page of Project Settings.

#include "SProjectile.h"
#include "SGranadeLauncher.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
// Sets default values
ASProjectile::ASProjectile()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	/*MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereComp"));*/
	
	

	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ASProjectile::OnHit);	// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 1.5f;

}



void ASProjectile::Tick(float DeltaTime)
{
	if (InitialLifeSpan == 0)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionFX, GetActorLocation());
		
	}
}

void ASProjectile::OnHit(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	/*FHitResult Hit;
	ASGranadeLauncher* gl = Cast<ASGranadeLauncher>(Instigator);
	AActor* HitActor = Hit.GetActor();*/
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

		
		
	}
	//ApplyPointDamage();
	//UGameplayStatics::ApplyRadialDamage(HitActor, 30.0f, GetActorLocation(), 40.0f, DamageType, this, AController* InstigatedByController);
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionFX, GetActorLocation());
	Destroy();
}

