// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitSpawner.h"
#include "BaseUnit.h"

ABaseUnit* UUnitSpawner::SpawnUnit(EUnitType unitType, const FVector& location)
{
    if (unitClassMap.Contains(unitType) && unitClassMap[unitType])
    {
        return GetWorld()->SpawnActor<ABaseUnit>(unitClassMap[unitType], location, FRotator::ZeroRotator);
    }
    return nullptr;
}

TArray<ABaseUnit*> UUnitSpawner::SpawnUnits(EUnitType unitType, int count, const FVector& origin, float radius)
{
    // Reserve memory for this amount of units first
    TArray<ABaseUnit*> spawnedUnits;
    spawnedUnits.Reserve(count);

    for (int i = 0; i < count; i++)
    {
        // Generate random 2D point and convert to 3D (Z = 0)
        FVector2D randomCirclePoint = FMath::RandPointInCircle(radius);
        FVector spawnOffset(randomCirclePoint.X, randomCirclePoint.Y, 0.f);

        // Snapping to ground
        FVector spawnLocation = origin + spawnOffset;
        spawnLocation.Z = GetTerrainHeightAtLocation(spawnLocation);

        if (ABaseUnit* unit = SpawnUnit(unitType, spawnLocation))
        {
            spawnedUnits.Add(unit);
        }
    }

    return spawnedUnits;
}

float UUnitSpawner::GetTerrainHeightAtLocation(const FVector& location)
{
    FHitResult hit;
    FCollisionQueryParams params;
    params.bTraceComplex = false;

    // Raycast downward to find ground
    if (GetWorld()->LineTraceSingleByChannel(
        hit,
        location + FVector(0, 0, 1000.f),
        location - FVector(0, 0, 1000.f),
        ECC_Visibility,
        params))
        {
            return hit.ImpactPoint.Z;
        }

    // Fallback in case cannot find terrain
    return location.Z;
}