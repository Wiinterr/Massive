#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FlowFieldSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FFlowFieldCell
{
	GENERATED_BODY();

	UPROPERTY(BlueprintReadOnly)
	FVector WorldLocation;

	UPROPERTY(BlueprintReadOnly)
	FVector FlowDirection;

	UPROPERTY(BlueprintReadOnly)
	float IntegrationCost = TNumericLimits<float>::Max();

	UPROPERTY(BlueprintReadOnly)
	float MovementCost = 1.0f;
};

UCLASS(Blueprintable, BlueprintType)
class MASSIVE_API UFlowFieldSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	public:
		virtual void Initialize(FSubsystemCollectionBase& Collection) override;

		virtual void Deinitialize() override;

		UFUNCTION(BlueprintCallable, Category = "Flow Field")
		void CreateFlowFieldGrid(int32 Width, int32 Height, float CellSize, const FVector& GridOrigin);

		UFUNCTION(BlueprintCallable, Category = "Flow Field")
		void GenerateFlowFieldToLocation(const FVector& TargetLocation);

		UFUNCTION(BlueprintPure, Category = "Flow Field")
		FVector GetFlowDirectionAtLocation(const FVector& WorldLocation) const;

		UFUNCTION(BlueprintPure, Category = "Flow Field")
		bool GetGridCellAtLocation(const FVector& WorldLocation, FFlowFieldCell& OutCell) const;

		UFUNCTION(BlueprintCallable, Category = "Flow Field")
		void DrawDebugFlowField(bool bEnabled, float Duration = -1.0f);

	private:
		TArray<TArray<FFlowFieldCell>> Grid;
		float CellSize = 200.0f;
		FVector GridOrigin = FVector::ZeroVector;
		int32 GridWidth = 0;
		int32 GridHeight = 0;

		bool bDebugDrawingEnabled = false;
		float DebugDrawDuration = -1.0f;

		void CalculateIntegrationField(const FIntPoint& TargetCell);
		void CalculateFlowDirections();
		FIntPoint WorldToGrid(const FVector& WorldLocation) const;
		FVector GridToWorld(const FIntPoint& GridCoord) const;
		bool IsValidGridCoordinate(const FIntPoint& Coord) const;
};

