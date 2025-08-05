#include "FlowFieldSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UFlowFieldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("FlowFieldSubsystem Initialized"));
}

void UFlowFieldSubsystem::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("FlowFieldSubsystem Deinitialized"));
}


void UFlowFieldSubsystem::CreateGrid(int32 inWidth, int32 inHeight, float inCellSize, FVector inOrigin)
{
	GridWidth = inWidth;
	GridHeight = inHeight;
	CellSize = inCellSize;
	GridOrigin = inOrigin;
	
	if(GridWidth <= 0 || GridHeight <= 0 || CellSize <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid grid parameters"));
		return;
	}
	
	GridCells.Empty();
	
	float const GridOffsetX = (GridWidth * CellSize) * 0.5f - CellSize * 0.5f;
	float const GridOffsetY = (GridHeight * CellSize) * 0.5f - CellSize * 0.5f;

	for (int y{}; y < GridHeight; y++)
	{
		for (int x{}; x < GridWidth; x++)
		{
			FVector const CellLocation = GridOrigin +
				FVector(x * CellSize - GridOffsetX, y * CellSize - GridOffsetY, 0.0f);

			FFlowFieldCell Cell;
			Cell.GridX = x;
			Cell.GridY = y;
			Cell.WorldLocation = CellLocation;

			GridCells.Add(Cell);
		}
	}
}

void UFlowFieldSubsystem::SetTargetCell(int32 x, int32 y)
{
	TargetCellIndex = FIntPoint(x, y);

	for (FFlowFieldCell& Cell : GridCells)
	{
		Cell.IntegrationValue = TNumericLimits<int32>::Max();
	}

	ComputeIntegrationField();
}

void UFlowFieldSubsystem::ComputeIntegrationField()
{
	TQueue<FFlowFieldCell*> CellQueue;

	FFlowFieldCell* TargetCell = GetCellAt(TargetCellIndex.X, TargetCellIndex.Y);
	if (!TargetCell) return;

	TargetCell->IntegrationValue = 0;
	CellQueue.Enqueue(TargetCell);

	while (!CellQueue.IsEmpty())
	{
		FFlowFieldCell* Current;
		CellQueue.Dequeue(Current);

		TArray<FFlowFieldCell*> Neighbors = GetCellNeighbors(*Current);

		for (FFlowFieldCell* Neighbor : Neighbors)
		{
			int32 NewCost = Current->IntegrationValue + Neighbor->Cost;
			if (NewCost < Neighbor->IntegrationValue)
			{
				Neighbor->IntegrationValue = NewCost;
				CellQueue.Enqueue(Neighbor);
			}
		}
	}
}

FFlowFieldCell* UFlowFieldSubsystem::GetCellAt(int32 x, int32 y)
{
	if (x < 0 || x >= GridWidth || y < 0 || y >= GridHeight) return nullptr;
	
	return &GridCells[y * GridWidth + x];
}

TArray<FFlowFieldCell*> UFlowFieldSubsystem::GetCellNeighbors(FFlowFieldCell& Cell)
{
	TArray<FFlowFieldCell*> Neighbors;

	constexpr int32 Offsets[4][2] =
		{
		{1, 0}, {-1, 0}, {0, 1}, {0, -1}
		};

	for (const auto& Offset : Offsets)
	{
		int32 neighborX = Cell.GridX + Offset[0];
		int32 neighborY = Cell.GridY + Offset[1];

		if (FFlowFieldCell* Neighbor = GetCellAt(neighborX, neighborY))
		{
			Neighbors.Add(Neighbor);
		}
	}

	return Neighbors;
}