#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FGridCell
{
	GENERATED_BODY()

	// Universal grid position
	int32 X;
	int32 Y;

	// Universal world-space location
	FVector WorldLocation;

	// Shared between FlowField and Theta*
	int32 Cost = 1;           // 1 = walkable, -1 = blocked
	bool bIsBlocked = false;

	// Flow Field specific (ignored by Theta*/A*)
	float IntegrationValue = 0.f;
	FVector FlowDirection = FVector::ZeroVector;
};

UCLASS()
class MASSIVE_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:
	
	// Sets default values for this actor's properties
	AGridManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	FVector GridOrigin = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 GridWidth = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 GridHeight = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	float CellSize = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	bool bDrawFlowDirection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	bool bDrawAStarPath = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	bool bDrawThetaStarPath = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	bool bSpawnObstacles = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid",
				meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ObstacleSpawnChance = 0.2f;
	
	TArray<FGridCell> Grid;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Core functions
	UFUNCTION(BlueprintCallable)
	void GenerateGrid();

	UFUNCTION(BlueprintCallable)
	void DrawDebugGrid();

	UFUNCTION(BlueprintCallable)
	void FlushDebug();
	
	void RandomizeGridCosts(float Chance = 0.2f);

	UFUNCTION(BlueprintCallable)
	void SpawnObstacles(TSubclassOf<AActor> ObstacleClass);
	
	TArray<const FGridCell*> GetNeighbors(int32 const& X, int32 const& Y) const;
	
	// Index helpers
	FORCEINLINE int32 XYToIndex(int32 X, int32 Y) const
	{
		return Y * GridWidth + X;
	}

	FORCEINLINE bool IsInside(int32 X, int32 Y) const
	{
		return X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight;
	}

	FIntPoint WorldToCell(const FVector& WorldLocation) const;
	FVector CellToWorld(const FIntPoint& Cell) const;
};
