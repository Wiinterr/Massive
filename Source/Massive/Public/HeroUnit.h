// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseUnit.h"
#include "HeroUnit.generated.h"

UCLASS(Blueprintable)
class MASSIVE_API AHeroUnit : public ABaseUnit
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHeroUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// -------------------- PROPERTIES --------------------

	UPROPERTY(EditDefaultsOnly, Category = "Hero Unit Stats")
	float heroMovementSpeedMultiplier = 1.5f;

	// -------------------- FUNCTIONS ---------------------

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
