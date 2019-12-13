// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	ASCharacter();

protected:
	
	virtual void BeginPlay() override;

	//Animation Methods
	void MoveForward(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();

	void BeginJump();

	bool bWantsToZoom =false;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
		float ZoomedInterpSpeed;
	//Default FOV set during begin play	
	float DefaultFOV;


	
	void BeginZoom();

	void EndZoom();
	UPROPERTY(Replicated)
	ASWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly,Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	//Pawn died previously
	UPROPERTY(Replicated,BlueprintReadOnly,Category = "Player")
	bool bDied;

	

	//Burasý devam edilicek
	/*UPROPERTY(BlueprintReadOnly,Category = "Player")
	float Yaw_Aim;
	UPROPERTY(BlueprintReadOnly,Category = "Player")
	float Pitch_Aim;*/

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category = "Components")
	USHealthComponent* HealthComp;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;
	
	UFUNCTION(BlueprintCallable, Category = "Player")
		void StartFire();
	UFUNCTION(BlueprintCallable, Category = "Player")
		void StopFire();

	/*virtual void AddControllerPitchInput(float Val) override;
	virtual void AddControllerYawInput(float Val) override;*/
};
