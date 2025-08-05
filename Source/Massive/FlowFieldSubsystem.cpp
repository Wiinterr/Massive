#include "FlowFieldSubsystem.h"
#include "DrawDebugHelpers.h"

void UFlowFieldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UFlowFieldSubsystem::Deinitialize()
{
    Grid.Empty();
    Super::Deinitialize();
}

void UFlowFieldSubsystem::CreateFlowFieldGrid(int32 Width, int32 Height, float InCellSize, const FVector& InGridOrigin)
{
    GridWidth = Width;
    GridHeight = Height;
    CellSize = InCellSize;
    GridOrigin = InGridOrigin;

    Grid.SetNum(GridWidth);
    for (int32 x = 0; x < GridWidth; ++x)
    {
        Grid[x].SetNum(GridHeight);
        for (int32 y = 0; y < GridHeight; ++y)
        {
            FFlowFieldCell& Cell = Grid[x][y];
            Cell.WorldLocation = GridToWorld(FIntPoint(x, y));
            Cell.FlowDirection = FVector::ZeroVector;
            Cell.IntegrationCost = TNumericLimits<float>::Max();
            Cell.MovementCost = 1.0f;
        }
    }
}

void UFlowFieldSubsystem::GenerateFlowFieldToLocation(const FVector& TargetLocation)
{
    if (Grid.IsEmpty()) return;

    const FIntPoint TargetCell = WorldToGrid(TargetLocation);
    if (!IsValidGridCoordinate(TargetCell)) return;

    CalculateIntegrationField(TargetCell);
    CalculateFlowDirections();

    if (bDebugDrawingEnabled)
    {
        DrawDebugFlowField(true, DebugDrawDuration);
    }
}

void UFlowFieldSubsystem::CalculateIntegrationField(const FIntPoint& TargetCell)
{
    for (int32 x = 0; x < GridWidth; ++x)
    {
        for (int32 y = 0; y < GridHeight; ++y)
        {
            Grid[x][y].IntegrationCost = TNumericLimits<float>::Max();
        }
    }

    // Dijkstra's algorithm
    TArray<FIntPoint> OpenSet;
    Grid[TargetCell.X][TargetCell.Y].IntegrationCost = 0;
    OpenSet.Add(TargetCell);

    while (OpenSet.Num() > 0)
    {
        const FIntPoint CurrentCell = OpenSet.Pop();

        // Check all 8 neighbors
        for (int32 x = -1; x <= 1; ++x)
        {
            for (int32 y = -1; y <= 1; ++y)
            {
                if (x == 0 && y == 0) continue;

                const FIntPoint Neighbor(CurrentCell.X + x, CurrentCell.Y + y);
                if (!IsValidGridCoordinate(Neighbor)) continue;

                const FFlowFieldCell& Current = Grid[CurrentCell.X][CurrentCell.Y];
                FFlowFieldCell& NeighborCell = Grid[Neighbor.X][Neighbor.Y];

                // Skip blocked cells (movement cost 0)
                if (NeighborCell.MovementCost <= 0.0f) continue;

                // Calculate move cost (orthogonal = 1, diagonal ≈ 1.4)
                const bool bIsDiagonal = (x != 0) && (y != 0);
                const float MoveCost = bIsDiagonal ? 1.4f : 1.0f;
                const float NewCost = Current.IntegrationCost + (MoveCost * NeighborCell.MovementCost);

                if (NewCost < NeighborCell.IntegrationCost)
                {
                    NeighborCell.IntegrationCost = NewCost;
                    OpenSet.Add(Neighbor);
                }
            }
        }
    }
}

void UFlowFieldSubsystem::CalculateFlowDirections()
{
    for (int32 x = 0; x < GridWidth; ++x)
    {
        for (int32 y = 0; y < GridHeight; ++y)
        {
            FFlowFieldCell& CurrentCell = Grid[x][y];
            CurrentCell.FlowDirection = FVector::ZeroVector;

            if (CurrentCell.IntegrationCost == TNumericLimits<float>::Max()) continue;

            float LowestCost = TNumericLimits<float>::Max();
            FIntPoint BestNeighbor;

            // Check all 8 neighbors
            for (int32 nx = -1; nx <= 1; ++nx)
            {
                for (int32 ny = -1; ny <= 1; ++ny)
                {
                    if (nx == 0 && ny == 0) continue;

                    const FIntPoint Neighbor(x + nx, y + ny);
                    if (!IsValidGridCoordinate(Neighbor)) continue;

                    const FFlowFieldCell& NeighborCell = Grid[Neighbor.X][Neighbor.Y];
                    if (NeighborCell.IntegrationCost < LowestCost)
                    {
                        LowestCost = NeighborCell.IntegrationCost;
                        BestNeighbor = Neighbor;
                    }
                }
            }

            if (LowestCost < TNumericLimits<float>::Max())
            {
                const FVector CurrentWorld = GridToWorld(FIntPoint(x, y));
                const FVector BestWorld = GridToWorld(BestNeighbor);
                CurrentCell.FlowDirection = (BestWorld - CurrentWorld).GetSafeNormal();
            }
        }
    }
}

FVector UFlowFieldSubsystem::GetFlowDirectionAtLocation(const FVector& WorldLocation) const
{
    FFlowFieldCell Cell;
    if (GetGridCellAtLocation(WorldLocation, Cell))
    {
        return Cell.FlowDirection;
    }
    return FVector::ZeroVector;
}

bool UFlowFieldSubsystem::GetGridCellAtLocation(const FVector& WorldLocation, FFlowFieldCell& OutCell) const
{
    const FIntPoint GridCoord = WorldToGrid(WorldLocation);
    if (IsValidGridCoordinate(GridCoord))
    {
        OutCell = Grid[GridCoord.X][GridCoord.Y];
        return true;
    }
    return false;
}

void UFlowFieldSubsystem::DrawDebugFlowField(bool bEnabled, float Duration)
{
    bDebugDrawingEnabled = bEnabled;
    DebugDrawDuration = Duration;

    if (!bDebugDrawingEnabled) return;

    UWorld* World = GetWorld();
    if (!World) return;

    const float ArrowSize = CellSize * 0.4f;
    const float ArrowThickness = 2.0f;

    for (int32 x = 0; x < GridWidth; ++x)
    {
        for (int32 y = 0; y < GridHeight; ++y)
        {
            const FFlowFieldCell& Cell = Grid[x][y];
            
            // Draw cell boundary
            DrawDebugBox(
                World,
                Cell.WorldLocation,
                FVector(CellSize * 0.5f, CellSize * 0.5f, 10.0f),
                FColor::White,
                false,
                DebugDrawDuration,
                0,
                1.0f
            );

            // Draw flow direction arrow if valid
            if (!Cell.FlowDirection.IsNearlyZero())
            {
                const FVector EndPoint = Cell.WorldLocation + (Cell.FlowDirection * CellSize * 0.4f);
                DrawDebugDirectionalArrow(
                    World,
                    Cell.WorldLocation,
                    EndPoint,
                    ArrowSize,
                    FColor::Green,
                    false,
                    DebugDrawDuration,
                    0,
                    ArrowThickness
                );
            }

            // Draw integration cost
            if (Cell.IntegrationCost < TNumericLimits<float>::Max())
            {
                DrawDebugString(
                    World,
                    Cell.WorldLocation + FVector(0, 0, 20.0f),
                    FString::Printf(TEXT("%.1f"), Cell.IntegrationCost),
                    nullptr,
                    FColor::Yellow,
                    DebugDrawDuration,
                    false,
                    1.0f
                );
            }
        }
    }
}

FIntPoint UFlowFieldSubsystem::WorldToGrid(const FVector& WorldLocation) const
{
    const FVector LocalLocation = WorldLocation - GridOrigin;
    const int32 X = FMath::FloorToInt(LocalLocation.X / CellSize + 0.5f);
    const int32 Y = FMath::FloorToInt(LocalLocation.Y / CellSize + 0.5f);
    return FIntPoint(
        FMath::Clamp(X, 0, GridWidth - 1),
        FMath::Clamp(Y, 0, GridHeight - 1)
    );
}

FVector UFlowFieldSubsystem::GridToWorld(const FIntPoint& GridCoord) const
{
    return GridOrigin + FVector(
        GridCoord.X * CellSize,
        GridCoord.Y * CellSize,
        0.0f
    );
}

bool UFlowFieldSubsystem::IsValidGridCoordinate(const FIntPoint& Coord) const
{
    return Coord.X >= 0 && Coord.Y >= 0 && Coord.X < GridWidth && Coord.Y < GridHeight;
}