// Fill out your copyright notice in the Description page of Project Settings.


#include "EliteUnit.h"

// Sets default values
AEliteUnit::AEliteUnit()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEliteUnit::BeginPlay()
{
	SetMovementSpeed(eliteMovementSpeedMultiplier);
	Super::BeginPlay();
	
}

// Called every frame
void AEliteUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEliteUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

