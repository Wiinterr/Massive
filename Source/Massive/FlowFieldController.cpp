#include "FlowFieldController.h"
#include "DrawDebugHelpers.h"
#include "GridManager.h"
#include "PropertyAccess.h"
#include "Kismet/GameplayStatics.h"

AFlowFieldController::AFlowFieldController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFlowFieldController::BeginPlay()
{
	Super::BeginPlay();
	if (!GridManager)
	{
		GridManager = Cast<AGridManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass())
		);
	}

	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("FlowFieldController: Could not find GridManager!"));
	}
}

TArray<FGridCell> const& AFlowFieldController::SetTargetCellByWorldLocation(FVector const& WorldLocation)
{
	if (GridManager->Grid.Num() == 0) return GridManager->Grid;

	TargetIndex = WorldLocationToIndex(WorldLocation);
	
	SetTargetCell(TargetIndex.X, TargetIndex.Y);

	return GridManager->Grid;
}

FIntPoint AFlowFieldController::WorldLocationToIndex(FVector const& WorldLocation)
{
	const FVector GridOrigin = GridManager->GridOrigin;
	const int32 GridWidth = GridManager->GridWidth;
	const int32 GridHeight = GridManager->GridHeight;
	const int32 CellSize = GridManager->CellSize;
	
	FVector const LocalPos = WorldLocation - GridOrigin;

	float const OffsetX = (GridWidth * CellSize) * 0.5f - CellSize * 0.5f;
	float const OffsetY = (GridHeight * CellSize) * 0.5f - CellSize * 0.5f;

	float const LocalX = LocalPos.X + OffsetX;
	float const LocalY = LocalPos.Y + OffsetY;

	int32 GridX = FMath::RoundToInt(LocalX / CellSize);
	int32 GridY = FMath::RoundToInt(LocalY / CellSize);

	GridX = FMath::Clamp(GridX, 0, GridWidth - 1);
	GridY = FMath::Clamp(GridY, 0, GridHeight - 1);
	
	return FIntPoint(GridX, GridY);
}

FVector AFlowFieldController::GetFlowOfCell(FIntPoint index, const TArray<FGridCell> OverrideGrid)
{
	int32 const FlattenedIndex = index.Y * GridManager->GridWidth + index.X;

	if (FlattenedIndex < 0 && FlattenedIndex >= GridManager->Grid.Num())
	{
		return FVector::ZeroVector;
	}
	
	return OverrideGrid[FlattenedIndex].FlowDirection;
}

void AFlowFieldController::SetTargetCell(int32 const& x, int32 const& y)
{
	TargetIndex = FIntPoint(x, y);

	for (FGridCell& Cell : GridManager->Grid)
	{
		Cell.IntegrationValue = TNumericLimits<int32>::Max();
	}

	ComputeIntegrationField();
	ComputeDirections();
}

void AFlowFieldController::ComputeIntegrationField()
{
	if (!GridManager) return;

	// Reset all integration values
	for (FGridCell& Cell : GridManager->Grid) { Cell.IntegrationValue = FLT_MAX; }
	
	FIntPoint TargetCellIndex = TargetIndex; // Assuming TargetIndex is valid
	int32 TargetIdx = GridManager->XYToIndex(TargetCellIndex.X, TargetCellIndex.Y);
	if (!GridManager->IsInside(TargetCellIndex.X, TargetCellIndex.Y)) return;

	FGridCell* TargetCell = &GridManager->Grid[TargetIdx];
	TargetCell->IntegrationValue = 0.f;
	
	TQueue<FGridCell*> CellQueue;
	CellQueue.Enqueue(TargetCell);

	while (!CellQueue.IsEmpty())
	{
		FGridCell* Current;
		CellQueue.Dequeue(Current);

		for (const FGridCell* NeighborConst : GridManager->GetNeighbors(Current->X, Current->Y))
		{
			FGridCell* Neighbor = const_cast<FGridCell*>(NeighborConst); // safe, GridManager owns the data
			
			FVector2D const DirToTarget = FVector2D(TargetCell->WorldLocation - Neighbor->WorldLocation).GetSafeNormal();
			float const Angle = FMath::Atan2(DirToTarget.Y, DirToTarget.X);
			float const Snapped = FMath::RoundToFloat(Angle / (PI / 2.f)) * (PI / 2.f);

			// If an angle is 15 degrees more than a cardinal direction we consider it 'diagonal' and add a cost to it
			float NeighborCost = Neighbor->Cost;
			if (FMath::Abs(Angle - Snapped) > (PI / 12.f)) NeighborCost += AnglePenalty;

			float NewCost = Current->IntegrationValue + NeighborCost;
			
			if (NewCost < Neighbor->IntegrationValue)
			{
				Neighbor->IntegrationValue = NewCost;
				CellQueue.Enqueue(Neighbor);
			}
		}
	}
}

void AFlowFieldController::ComputeDirections()
{
	for (FGridCell& Cell : GridManager->Grid)
	{
		TArray<const FGridCell*> Neighbors = GridManager->GetNeighbors(Cell.X, Cell.Y);

		const FGridCell* BestNeighbor = nullptr;
		float BestCost = Cell.IntegrationValue;

		for (const FGridCell* Neighbor : Neighbors)
		{
			if (Neighbor->IntegrationValue < BestCost)
			{
				BestNeighbor = Neighbor;
				BestCost = Neighbor->IntegrationValue;
			}
		}

		if (BestNeighbor)
		{
			FVector const Direction = (BestNeighbor->WorldLocation - Cell.WorldLocation).GetSafeNormal();
			Cell.FlowDirection = Direction;
		}
		else
		{
			Cell.FlowDirection = FVector::ZeroVector;
		}
	}
}

