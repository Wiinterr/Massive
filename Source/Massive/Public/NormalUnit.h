// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NormalUnit.generated.h"
#include "BaseUnit.h"

UCLASS()
class MASSIVE_API ANormalUnit : public ABaseUnit
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANormalUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// -------------------- PROPERTIES --------------------

	UPROPERTY(EditDefaultsOnly, Category = "Normal Unit Stats")
	float normalMovementSpeedMultiplier = 1.0f;

	// -------------------- FUNCTIONS ---------------------

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
