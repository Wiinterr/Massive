// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include <BaseUnit.h>
#include "UnitSpawner.generated.h"

// Enum to define different unit types
UENUM(BlueprintType)
enum class EUnitType : uint8
{
	Normal	UMETA(DisplayName = "Normal Unit"),
	Elite	UMETA(DisplayName = "Elite Unit"),
	Hero	UMETA(DisplayName = "Hero Unit")
};

UCLASS(Blueprintable)
class MASSIVE_API UUnitSpawner : public UWorldSubsystem
{
	GENERATED_BODY()

private:

	// Map unit types to their classes
	UPROPERTY(EditDefaultsOnly, Category = "Unit Spawning")
	TMap<EUnitType, TSubclassOf<ABaseUnit>> unitClassMap =
	{
		{EUnitType::Normal, nullptr},
		{EUnitType::Elite, nullptr},
		{EUnitType::Hero, nullptr}
	};

public:

	// Spawn a unit of specified type at location
	UFUNCTION(BlueprintCallable, Category = "Unit Spawning")
	ABaseUnit* SpawnUnit(EUnitType unitType, const FVector& location);

	// Spawning multiple units in a location
	UFUNCTION(BlueprintCallable, Category = "Unit Spawning")
	TArray<ABaseUnit*> SpawnUnits(
		EUnitType unitType,
		int count,
		const FVector& origin,
		float radius = 500.f);

	// Helper to get terrain's height for snapping
	float GetTerrainHeightAtLocation(const FVector& location);
};
