#include "FlowFieldController.h"
#include "DrawDebugHelpers.h"
#include "PropertyAccess.h"
#include "Kismet/GameplayStatics.h"

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
	if (bDrawDebugPath) DrawDebugGrid();
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
	FlushPersistentDebugLines(GetWorld());
	FlushDebugStrings(GetWorld());
	
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
			FString::Printf(TEXT("%.2f"), Cell.IntegrationValue),
			nullptr,
			FColor::White,
			-1.f,
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

void AFlowFieldController::UpdateGrid(TSubclassOf<AActor> ObstacleClass)
{
	if (GridCells.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Grid not initialized!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid world context for UpdateGrid!"));
		return;
	}

	// Clear any previous debug visuals if you use them
	// for (auto& Cell : GridCells) DrawDebugBox(World, Cell.WorldPos, FVector(CellSize * 0.5f), FColor::White, false, 0.1f);

	for (auto& Cell : GridCells)
	{
		// Randomly decide to increase cost (20% chance)
		const float Chance = FMath::FRand();
		if (Chance < 0.2f)
		{
			// Set a very high cost to make it avoided
			Cell.Cost = 255;

			// Optional: Spawn obstacle actor
			if (ObstacleClass && World)
			{
				FVector SpawnLocation = Cell.WorldLocation;
				FRotator SpawnRotation = FRotator::ZeroRotator;
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				AActor* SpawnedObstacle = World->SpawnActor<AActor>(ObstacleClass, SpawnLocation, SpawnRotation, SpawnParams);

				if (!SpawnedObstacle)
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to spawn obstacle at (%f, %f, %f)"), 
						SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
				}
			}
		}
		else
		{
			// Reset to normal cost (optional)
			Cell.Cost = 1;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Grid updated — random obstacle costs applied."));
}

void AFlowFieldController::SetTargetCellByWorldLocation(FVector const& WorldLocation)
{
	if (GridCells.Num() == 0) return;

	TargetIndex = WorldLocationToIndex(WorldLocation);
	
	SetTargetCell(TargetIndex.X, TargetIndex.Y);
}

FIntPoint AFlowFieldController::WorldLocationToIndex(FVector const& WorldLocation)
{
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

FVector AFlowFieldController::GetFlowOfCell(FIntPoint index)
{
	int32 const FlattenedIndex = index.Y * GridWidth + index.X;
	return GridCells[FlattenedIndex].FlowDirection;
}


void AFlowFieldController::SetTargetCell(int32 const& x, int32 const& y)
{
	TargetIndex = FIntPoint(x, y);

	for (FFlowFieldCell& Cell : GridCells)
	{
		Cell.IntegrationValue = TNumericLimits<int32>::Max();
	}

	ComputeIntegrationField();
	ComputeDirections();
	
	if (bDrawDebugPath) DrawDebugGrid();
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

		for (FFlowFieldCell* Neighbor : GetCellNeighbors(*Current))
		{
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
	for (FFlowFieldCell& Cell : GridCells)
	{
		TArray<FFlowFieldCell*> Neighbors = GetCellNeighbors(Cell);

		FFlowFieldCell* BestNeighbor = nullptr;
		float BestCost = Cell.IntegrationValue;

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
			FVector const Direction = (BestNeighbor->WorldLocation - Cell.WorldLocation).GetSafeNormal();
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

	const int32 Offsets[8][2] =
	{
		{0, 1}, {0, -1}, {-1, 0}, {1, 0},	// Cardinal
		{-1, -1}, {1, -1}, {1, 1}, {-1, 1}	// Diagonal
	};

	for (const auto& Offset : Offsets)
	{
		int32 NeighborX = Cell.GridX + Offset[0];
		int32 NeighborY = Cell.GridY + Offset[1];

		if (FFlowFieldCell* Neighbor = GetCellAt(NeighborX, NeighborY))
		{
			Neighbors.Add(Neighbor);
		}
	}

	return Neighbors;
}

