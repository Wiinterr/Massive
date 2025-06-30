// Fill out your copyright notice in the Description page of Project Settings.


#include "NormalUnit.h"

// Sets default values
ANormalUnit::ANormalUnit()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANormalUnit::BeginPlay()
{
	SetMovementSpeed(normalMovementSpeedMultiplier);
	Super::BeginPlay();
	
}

// Called every frame
void ANormalUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ANormalUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

