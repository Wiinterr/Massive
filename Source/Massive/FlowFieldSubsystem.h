#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FlowFieldTypes.h"
#include "FlowFieldSubsystem.generated.h"

UCLASS()
class MASSIVE_API UFlowFieldSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void CreateGrid(int32 inWidth, int32 inHeight, float inCellSize, FVector inOrigin);
	void SetTargetCell(int32 x, int32 y);
	void ComputeIntegrationField();
	//void ComputeFlowDirections(); TODO

	const TArray<FFlowFieldCell>& GetGridCells() const { return GridCells; };
	
	FFlowFieldCell* GetCellAt(int32 x, int32 y);

private:
	int32 GridWidth = 0;
	int32 GridHeight = 0;
	float CellSize = 100.f;
	FVector GridOrigin;
	
	TArray<FFlowFieldCell> GridCells;
	FIntPoint TargetCellIndex = FIntPoint(-1, -1);
	TArray<FFlowFieldCell*> GetCellNeighbors(FFlowFieldCell& Cell);
};

