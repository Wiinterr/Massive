// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseUnit.generated.h"

UCLASS(Blueprintable)
class MASSIVE_API ABaseUnit : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// -------------------- PROPERTIES --------------------

	UPROPERTY(EditDefaultsOnly, Category = "Unit Stats")
	float baseHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Unit Stats")
	float baseMovementSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Unit Stats")
	float baseMovementSpeedMultiplier = 1.0f;

	// Movement speed value used during runtime
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Unit Stats")
	float movementSpeed;

	// -------------------- FUNCTIONS ---------------------

	// Takes in a unit's multiplier value and applies it to this unit's movement speed accordingly
	UFUNCTION(BlueprintCallable, Category = "Unit")
	void SetMovementSpeed(float multiplier = -1.0f);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};

