#include "GridManager.h"
#include "DrawDebugHelpers.h"

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGridManager::BeginPlay()
{
    Super::BeginPlay();
}

void AGridManager::GenerateGrid()
{
    //UE_LOG(LogTemp, Warning, TEXT("Generating Grid"));
    Grid.Empty();
    Grid.SetNum(GridWidth * GridHeight);

    const float OffsetX = (GridWidth * CellSize) * 0.5f - CellSize * 0.5f;
    const float OffsetY = (GridHeight * CellSize) * 0.5f - CellSize * 0.5f;

    for (int32 y = 0; y < GridHeight; y++)
    {
        for (int32 x = 0; x < GridWidth; x++)
        {
            const int32 Index = XYToIndex(x, y);

            FGridCell& Cell = Grid[Index];
            Cell.X = x;
            Cell.Y = y;

            FVector WorldPos = GridOrigin +
                FVector(x * CellSize - OffsetX, y * CellSize - OffsetY, 0);

            Cell.WorldLocation = WorldPos;
            Cell.Cost = 1;
            Cell.bIsBlocked = false;
        }
    }

    if (bSpawnObstacles) RandomizeGridCosts();
    if (bDrawDebug) DrawDebugGrid();
}

void AGridManager::DrawDebugGrid()
{
    if (!bDrawDebug) return;
    
    if (!GetWorld()) return;

    for (const FGridCell& Cell : Grid)
    {
        // Color based on blocked/unblocked for Theta* visualization
        const FColor CellColor = Cell.bIsBlocked || Cell.Cost < 0 ? FColor::Red : FColor::White;

        // Thickness based on blocked/unblocked for Theta* visualization
        const float CellThickness = Cell.bIsBlocked || Cell.Cost < 0 ? 0.f : 2.f;
        
        // Draw the cell box
        DrawDebugBox(
            GetWorld(),
            Cell.WorldLocation,
            FVector(CellSize * 0.5f, CellSize * 0.5f, 5.f),
            CellColor,
            true,   // persistent
            5.f,     // duration
            0.f,
            CellThickness
        );

        const FString& ToPrint = FString::Printf(
            TEXT("%d\n%.2f"),
            Cell.Cost,
            Cell.IntegrationValue == FLT_MAX ? -1.0f : Cell.IntegrationValue);
        // Draw integration and cost values
        DrawDebugString(
            GetWorld(),
            Cell.WorldLocation + FVector(0.f, CellSize * 0.5f, 5.f),
            ToPrint,
            nullptr,
            FColor::White,
            -1.f,
            false
        );

        if (bDrawFlowDirection)
        {
            // Draw FlowField directional arrow
            DrawDebugDirectionalArrow(
                GetWorld(),
                Cell.WorldLocation,
                Cell.WorldLocation + Cell.FlowDirection * 30.f,
                10.f,
                FColor::Yellow,
                true,
                5.f,
                0,
                4.f
            );
        }
    }
}

void AGridManager::FlushDebug()
{
    FlushPersistentDebugLines(GetWorld());
    FlushDebugStrings(GetWorld());
}

void AGridManager::RandomizeGridCosts(float Chance)
{
    for (FGridCell& Cell : Grid)
    {
        if (FMath::FRand() < Chance)
        {
            Cell.Cost = -1;
            Cell.bIsBlocked = true;
        }
        else
        {
            Cell.Cost = 1;
            Cell.bIsBlocked = false;
        }
    }
}

void AGridManager::SpawnObstacles(TSubclassOf<AActor> ObstacleClass)
{
    if (Grid.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Grid not initialized!"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid world context for SpawnObstacles!"));
        return;
    }

    for (const FGridCell& Cell : Grid)
    {
        if (Cell.bIsBlocked) // only spawn where blocked
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            AActor* SpawnedObstacle = World->SpawnActor<AActor>(ObstacleClass, Cell.WorldLocation, FRotator::ZeroRotator, Params);
            if (!SpawnedObstacle)
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to spawn obstacle at (%f, %f, %f)"),
                    Cell.WorldLocation.X, Cell.WorldLocation.Y, Cell.WorldLocation.Z);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Obstacles spawned for blocked cells."));
}

TArray<const FGridCell*> AGridManager::GetNeighbors(int32 const& X, int32 const& Y) const
{
    TArray<const FGridCell*> Neighbors;

    const int Offsets[8][2] =
    {
        {0, 1}, {0, -1}, {-1, 0}, {1, 0},     // Cardinal
        {-1, -1}, {1, -1}, {1, 1}, {-1, 1}    // Diagonal
    };

    for (int i = 0; i < 8; i++)
    {
        int NX = X + Offsets[i][0];
        int NY = Y + Offsets[i][1];

        // Bounds
        if (!IsInside(NX, NY)) continue;

        int NIdx = XYToIndex(NX, NY);
        const FGridCell* NCell = &Grid[NIdx];

        // Skip obstacle
        if (NCell->Cost >= 500 || NCell->bIsBlocked) continue;

        // Diagonal checking
        bool bIsDiagonal = (FMath::Abs(Offsets[i][0]) + FMath::Abs(Offsets[i][1]) == 2);

        if (bIsDiagonal)
        {
            int AX = X + Offsets[i][0];
            int AY = Y;
            int BX = X;
            int BY = Y + Offsets[i][1];

            if (!IsInside(AX, AY) || !IsInside(BX, BY))
                continue;

            int AIdx = XYToIndex(AX, AY);
            int BIdx = XYToIndex(BX, BY);

            if (Grid[AIdx].Cost >= 500 || Grid[AIdx].bIsBlocked ||
                Grid[BIdx].Cost >= 500 || Grid[BIdx].bIsBlocked)
            {
                continue;
            }
        }

        Neighbors.Add(NCell);
    }

    return Neighbors;
}

FIntPoint AGridManager::WorldToCell(const FVector& WorldLocation) const
{
    FVector Local = WorldLocation - GridOrigin;

    const float OffsetX = (GridWidth * CellSize) * 0.5f - CellSize * 0.5f;
    const float OffsetY = (GridHeight * CellSize) * 0.5f - CellSize * 0.5f;

    int32 X = FMath::RoundToInt((Local.X + OffsetX) / CellSize);
    int32 Y = FMath::RoundToInt((Local.Y + OffsetY) / CellSize);

    X = FMath::Clamp(X, 0, GridWidth - 1);
    Y = FMath::Clamp(Y, 0, GridHeight - 1);

    return FIntPoint(X, Y);
}

FVector AGridManager::CellToWorld(const FIntPoint& Cell) const
{
    const float OffsetX = (GridWidth * CellSize) * 0.5f - CellSize * 0.5f;
    const float OffsetY = (GridHeight * CellSize) * 0.5f - CellSize * 0.5f;

    return GridOrigin +
        FVector(Cell.X * CellSize - OffsetX, Cell.Y * CellSize - OffsetY, 0);
}
