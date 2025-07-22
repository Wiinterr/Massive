// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseUnit.h"
#include "EliteUnit.generated.h"

UCLASS(Blueprintable)
class MASSIVE_API AEliteUnit : public ABaseUnit
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEliteUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// -------------------- PROPERTIES --------------------

	UPROPERTY(EditDefaultsOnly, Category = "Elite Unit Stats")
	float eliteMovementSpeedMultiplier = 1.2f;

	// -------------------- FUNCTIONS ---------------------

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
