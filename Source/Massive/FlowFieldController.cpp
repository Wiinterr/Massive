#include "FlowFieldController.h"
#include "DrawDebugHelpers.h"

AFlowFieldController::AFlowFieldController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFlowFieldController::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFlowFieldController::GenerateGrid()
{
	CreateGrid();
	DrawDebugGrid();
}

void AFlowFieldController::CreateGrid()
{
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

void AFlowFieldController::DrawDebugGrid()
{
	for (const FFlowFieldCell& Cell : GridCells)
	{
		DrawDebugBox(
			GetWorld(),
			Cell.WorldLocation,
			FVector(CellSize * 0.5, CellSize * 0.5, 5.0f),
			FColor::White,
			true,
			5.f
			);

		DrawDebugString(
			GetWorld(),
			Cell.WorldLocation,
			FString::FromInt(Cell.Cost),
			nullptr,
			FColor::White,
			5.f,
			false
			);

		DrawDebugDirectionalArrow(
			GetWorld(),
			Cell.WorldLocation,
			Cell.WorldLocation + Cell.FlowDirection * 30.f,
			10.f,
			FColor::White,
			true,
			5.f,
			0,
			4.f
			);
	}
}

void AFlowFieldController::SetTargetCellByWorldLocation(FVector const& WorldLocation)
{
	if (GridCells.Num() == 0) return;

	FVector LocalPos = WorldLocation - GridOrigin;

	float const OffsetX = (GridWidth * CellSize) * 0.5f - CellSize * 0.5f;
	float const OffsetY = (GridHeight * CellSize) * 0.5f - CellSize * 0.5f;

	float LocalX = LocalPos.X + OffsetX;
	float LocalY = LocalPos.Y + OffsetY;

	int32 GridX = FMath::RoundToInt(LocalX / CellSize);
	int32 GridY = FMath::RoundToInt(LocalY / CellSize);

	GridX = FMath::Clamp(GridX, 0, GridWidth);
	GridY = FMath::Clamp(GridY, 0, GridHeight);

	TargetIndex = FIntPoint(GridX, GridY);
	SetTargetCell(GridX, GridY);
}

void AFlowFieldController::SetTargetCell(int32 x, int32 y)
{
	TargetIndex = FIntPoint(x, y);

	for (FFlowFieldCell& Cell : GridCells)
	{
		Cell.IntegrationValue = TNumericLimits<int32>::Max();
	}

	ComputeIntegrationField();
	ComputeDirections();

	FlushPersistentDebugLines(GetWorld());
	DrawDebugGrid();
}

void AFlowFieldController::ComputeIntegrationField()
{
	TQueue<FFlowFieldCell*> CellQueue;

	FFlowFieldCell* TargetCell = GetCellAt(TargetIndex.X, TargetIndex.Y);
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

void AFlowFieldController::ComputeDirections()
{
	for (FFlowFieldCell& Cell : GridCells)
	{
		TArray<FFlowFieldCell*> Neighbors = GetCellNeighbors(Cell);

		FFlowFieldCell* BestNeighbor = nullptr;
		int32 BestCost = Cell.IntegrationValue;

		for (FFlowFieldCell* Neighbor : Neighbors)
		{
			if (Neighbor->IntegrationValue < BestCost)
			{
				BestNeighbor = Neighbor;
				BestCost = Neighbor->IntegrationValue;
			}
		}

		if (BestNeighbor)
		{
			FVector Direction = (BestNeighbor->WorldLocation - Cell.WorldLocation).GetSafeNormal();
			Cell.FlowDirection = Direction;
		}
		else
		{
			Cell.FlowDirection = FVector::ZeroVector;
		}
	}
}


FFlowFieldCell* AFlowFieldController::GetCellAt(int32 const& x, int32 const& y)
{
	if (x < 0 || x >= GridWidth || y < 0 || y >= GridHeight) return nullptr;
	
	return &GridCells[y * GridWidth + x];
}

TArray<FFlowFieldCell*> AFlowFieldController::GetCellNeighbors(FFlowFieldCell const& Cell)
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