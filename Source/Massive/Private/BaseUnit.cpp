// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUnit.h"

// Sets default values
ABaseUnit::ABaseUnit()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseUnit::BeginPlay()
{
	Super::BeginPlay();
	SetMovementSpeed();
}

// Called every frame
void ABaseUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// Calculate and set movement speed for individual units
void ABaseUnit::SetMovementSpeed(float multiplier)
{
	// By default multiplier input param is -1.f so if less than 0.f
	// assumes that unit will take baseMovementSpeedMultiplier instead of its own multiplier value
	float multiplierToUse = (multiplier > 0.f) ? multiplier : baseMovementSpeedMultiplier;

	movementSpeed = baseMovementSpeed * multiplierToUse;
}
