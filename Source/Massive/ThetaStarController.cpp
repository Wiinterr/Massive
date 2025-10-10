#include "ThetaStarController.h"
#include "DrawDebugHelpers.h"
#include "Components/LineBatchComponent.h"
#include "Engine/World.h"

// Sets default values
AThetaStarController::AThetaStarController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AThetaStarController::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AThetaStarController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AThetaStarController::GenerateCostMap()
{
	// If grid origin is zero, derive it from actor location to mimic your FlowField behavior
	if (GridOrigin.IsZero())
	{
		// centre grid around actor (same math as your FlowField CreateGrid())
		const float GridOffsetX = (GridWidth * CellSize) * 0.5f - CellSize * 0.5f;
		const float GridOffsetY = (GridHeight * CellSize) * 0.5f - CellSize * 0.5f;
		GridOrigin = GetActorLocation() - FVector(GridOffsetX, GridOffsetY, 0.f);
	}

	// If cost map empty, create defaults
	if (CostMap.Num() != GridWidth * GridHeight)
	{
		CreateDefaultCostMap();
	}
	
	if (bDrawDebugPath) DrawDebugGrid();
}

void AThetaStarController::CreateDefaultCostMap(int32 InGridWidth, int32 InGridHeight)
{
	if (InGridWidth > 0) GridWidth = InGridWidth;
	if (InGridHeight > 0) GridHeight = InGridHeight;

	CostMap.Init(1, GridWidth * GridHeight); // default cost 1, walkable
}

void AThetaStarController::DrawDebugGrid()
{
	if (!GetWorld()) return;

	//FlushPersistentDebugLines(GetWorld());
	//FlushDebugStrings(GetWorld());

	for (int32 y = 0; y < GridHeight; y++)
	{
		for (int32 x = 0; x < GridWidth; x++)
		{
			int32 Index = XYToIndex(x, y);
			if (!CostMap.IsValidIndex(Index)) continue;

			// Convert cell to world position
			FVector CellWorld = CellToWorld(FIntPoint(x, y));

			// Draw a box for each cell
			DrawDebugBox(
				GetWorld(),
				CellWorld,
				FVector(CellSize * 0.5f, CellSize * 0.5f, 5.f),
				CostMap[Index] >= 0 ? FColor::White : FColor::Red, // red for blocked
				true,
				-1.f
			);
		}
	}
}

bool AThetaStarController::IsWalkableIndex(int32 Index) const
{
	if (Index < 0 || Index >= CostMap.Num()) return false;
	return CostMap[Index] >= 0;
}

float AThetaStarController::MovementCostBetween(int32 AIndex, int32 BIndex) const
{
	int Ax, Ay, Bx, By;
	IndexToXY(AIndex, Ax, Ay);
	IndexToXY(BIndex, Bx, By);

	int Dx = FMath::Abs(Ax - Bx);
	int Dy = FMath::Abs(Ay - By);

	if (Dx + Dy == 1) return 1.f;
	return DiagonalCost;
}

TArray<int32> AThetaStarController::GetNeighborsIndices(int32 Index) const
{
	TArray<int32> Out;
	int X, Y;
	IndexToXY(Index, X, Y);

	const int Offsets[8][2] =
	{
		{0, 1}, {0, -1}, {1, 0}, {-1, 0},
		{1, 1}, {1, -1}, {-1, 1}, {-1, -1}
	};

	for (int i = 0; i < (bAllowDiagonal ? 8 : 4); ++i)
	{
		int NX = X + Offsets[i][0];
		int NY = Y + Offsets[i][1];

		if (!IsInsideGrid(NX, NY)) continue;

		int NIdx = XYToIndex(NX, NY);
		if (!IsWalkableIndex(NIdx)) continue;

		// Prevent corner cutting if enabled and this neighbor is diagonal
		if (bPreventDiagonalCutting && (FMath::Abs(Offsets[i][0]) + FMath::Abs(Offsets[i][1]) == 2))
		{
			// Check the two adjacent cardinal tiles
			int AIdx = XYToIndex(X + Offsets[i][0], Y);
			int BIdx = XYToIndex(X, Y + Offsets[i][1]);
			if (!IsWalkableIndex(AIdx) || !IsWalkableIndex(BIdx))
			{
				continue;
			}
		}

		Out.Add(NIdx);
	}

	return Out;
}

TArray<FVector> AThetaStarController::FindPath(const FVector& StartWorld, const FVector& GoalWorld)
{
	// Convert to grid space
	FIntPoint StartCell = WorldToCell(StartWorld);
	FIntPoint GoalCell  = WorldToCell(GoalWorld);

	// Check grid bounds + walkability
	if (!IsInsideGrid(StartCell.X, StartCell.Y) || !IsInsideGrid(GoalCell.X, GoalCell.Y))
	{
		return {};
	}
	if (CostMap[XYToIndex(StartCell.X, StartCell.Y)] < 0 ||
		CostMap[XYToIndex(GoalCell.X, GoalCell.Y)] < 0)
	{
		return {};
	}

	// Run the A* core (this part can stay private, taking FIntPoints)
	TArray<FIntPoint> CellPath = RunThetaStar(StartCell, GoalCell);

	// Convert back to world space
	TArray<FVector> WorldPath;
	for (const FIntPoint& Cell : CellPath)
	{
		WorldPath.Add(CellToWorld(Cell));
	}

	// Optional debug draw
	if (bDrawDebugPath)
	{
		FlushDebugStrings(GetWorld());
		
		for (int32 i = 0; i + 2 < WorldPath.Num(); i++)
		{
			DrawDebugLine(
				GetWorld(),
				WorldPath[i],
				WorldPath[i + 1],
				FColor::White,
				false,
				1.5f,
				0,
				6.f
				);
		}

		DrawDebugDirectionalArrow(
			GetWorld(),
			WorldPath[WorldPath.Num() - 2],
			WorldPath.Last(),
			20.f,             // arrow size
			FColor::White,    // make it stand out
			false,            // persistent lines?
			1.5f,              // life time
			0,                // depth priority
			6.f               // thickness
		);
		
		// Draw updated cost values
		for (int32 y = 0; y < GridHeight; y++)
		{
			for (int32 x = 0; x < GridWidth; x++)
			{
				int32 Index = XYToIndex(x, y);
				if (!CostMap.IsValidIndex(Index)) continue;

				FVector CellWorld = CellToWorld(FIntPoint(x, y));

				DrawDebugString(
					GetWorld(),
					CellWorld + FVector(0, 0, 20.f),
					FString::Printf(TEXT("%d"), CostMap[Index]),
					nullptr,
					FColor::White,
					-1,
					false
				);
			}
		}
	}

	return WorldPath;
}

TArray<FIntPoint> AThetaStarController::RunThetaStar(const FIntPoint& StartCell, const FIntPoint& GoalCell)
{
    TArray<FIntPoint> ResultPath;

    // Validate cells in-bounds
    if (!IsInsideGrid(StartCell.X, StartCell.Y) || !IsInsideGrid(GoalCell.X, GoalCell.Y))
    {
        return ResultPath;
    }

    const int32 StartIdx = XYToIndex(StartCell.X, StartCell.Y);
    const int32 GoalIdx  = XYToIndex(GoalCell.X, GoalCell.Y);

    // Validate walkability (-1 means blocked)
    if (!CostMap.IsValidIndex(StartIdx) || !CostMap.IsValidIndex(GoalIdx) ||
        CostMap[StartIdx] < 0 || CostMap[GoalIdx] < 0)
    {
        return ResultPath;
    }

    const int32 NumNodes = GridWidth * GridHeight;

    // Search buffers
    TArray<FSearchNode> SearchNodes;
    SearchNodes.SetNum(NumNodes);
    for (int32 i = 0; i < NumNodes; ++i)
    {
        SearchNodes[i].G = TNumericLimits<float>::Max();
        SearchNodes[i].H = 0.f;
        SearchNodes[i].F = TNumericLimits<float>::Max();
        SearchNodes[i].Parent = -1;
        SearchNodes[i].bClosed = false;
    }

    FBinaryHeapOpenSet OpenSet;
    OpenSet.ReservePos(NumNodes);
    OpenSet.Init(NumNodes);

    // Octile heuristic (admissible/consistent for 8-way with DiagonalCost)
    auto Heuristic = [&](int32 A, int32 B) -> float
    {
        int32 Ax, Ay, Bx, By;
        IndexToXY(A, Ax, Ay);
        IndexToXY(B, Bx, By);
        const int32 Dx = FMath::Abs(Ax - Bx);
        const int32 Dy = FMath::Abs(Ay - By);
        const float F  = float(FMath::Min(Dx, Dy));
        return float(FMath::Max(Dx, Dy) - F) + DiagonalCost * F;
    };

    // Initialize start
    SearchNodes[StartIdx].G = 0.f;
    SearchNodes[StartIdx].H = Heuristic(StartIdx, GoalIdx);
    SearchNodes[StartIdx].F = SearchNodes[StartIdx].H; // G=0 so F=H
    OpenSet.Push(StartIdx, SearchNodes[StartIdx].F);

    bool bFound = false;

    while (!OpenSet.IsEmpty())
    {
        const int32 Curr = OpenSet.PopMin();
        if (Curr == -1) break;

        if (Curr == GoalIdx)
        {
            bFound = true;
            break;
        }

        // Mark closed
        SearchNodes[Curr].bClosed = true;

        // Expand neighbors
        const TArray<int32> Neighbors = GetNeighborsIndices(Curr);
    	for (const int32 Nb : Neighbors)
    	{
    		if (SearchNodes[Nb].bClosed) continue;

    		float TerrainCost = (CostMap.IsValidIndex(Nb) ? FMath::Max(1, CostMap[Nb]) : 1);
    		float MoveCost = MovementCostBetween(Curr, Nb);
    
    		// Lazy Theta*: Attempt to connect neighbor to parent of current if LOS exists
    		int32 ParentIdx = SearchNodes[Curr].Parent;
    		if (ParentIdx != -1 && HasLineOfSight(ParentIdx, Nb))
    		{
    			float TentativeG = SearchNodes[ParentIdx].G + MovementCostBetween(ParentIdx, Nb) * TerrainCost;
    			if (TentativeG < SearchNodes[Nb].G)
    			{
    				SearchNodes[Nb].Parent = ParentIdx;
    				SearchNodes[Nb].G = TentativeG;
    				SearchNodes[Nb].H = Heuristic(Nb, GoalIdx);
    				SearchNodes[Nb].F = SearchNodes[Nb].G + SearchNodes[Nb].H;
    				OpenSet.PushOrDecrease(Nb, SearchNodes[Nb].F);
    			}
    		}
    		else
    		{
    			// Fallback to normal Theta* behavior
    			float TentativeG = SearchNodes[Curr].G + MoveCost * TerrainCost;
    			if (TentativeG < SearchNodes[Nb].G)
    			{
    				SearchNodes[Nb].Parent = Curr;
    				SearchNodes[Nb].G = TentativeG;
    				SearchNodes[Nb].H = Heuristic(Nb, GoalIdx);
    				SearchNodes[Nb].F = SearchNodes[Nb].G + SearchNodes[Nb].H;
    				OpenSet.PushOrDecrease(Nb, SearchNodes[Nb].F);
    			}
    		}
    	}
    }

    // Reconstruct path (grid-space)
    if (bFound)
    {
        TArray<int32> PathIdx;
        PathIdx.Reserve(64);
        int32 Trace = GoalIdx;
        while (Trace != -1)
        {
            PathIdx.Add(Trace);
            Trace = SearchNodes[Trace].Parent;
        }
        Algo::Reverse(PathIdx);

        ResultPath.Reserve(PathIdx.Num());
        for (const int32 Idx : PathIdx)
        {
            int32 X, Y;
            IndexToXY(Idx, X, Y);
            ResultPath.Add(FIntPoint(X, Y));
        }
    }

    return ResultPath;
}

bool AThetaStarController::HasLineOfSight(int32 FromIdx, int32 ToIdx) const
{
	int32 X0, Y0, X1, Y1;
	IndexToXY(FromIdx, X0, Y0);
	IndexToXY(ToIdx, X1, Y1);

	// Bresenham's line algorithm (grid-based)
	int32 Dx = FMath::Abs(X1 - X0);
	int32 Dy = FMath::Abs(Y1 - Y0);

	int32 SX = (X0 < X1) ? 1 : -1;
	int32 SY = (Y0 < Y1) ? 1 : -1;

	int32 Err = Dx - Dy;

	int X = X0;
	int Y = Y0;

	while (true)
	{
		if (!IsWalkableIndex(XYToIndex(X, Y)))
			return false;

		if (X == X1 && Y == Y1)
			break;

		int32 E2 = 2 * Err;
		if (E2 > -Dy) { Err -= Dy; X += SX; }
		if (E2 <  Dx) { Err += Dx; Y += SY; }
	}

	return true;
}

// Convert world position into grid cell coordinates
FIntPoint AThetaStarController::WorldToCell(const FVector& WorldLocation) const
{
	FVector Local = WorldLocation - GridOrigin;
	int32 X = FMath::FloorToInt(Local.X / CellSize);
	int32 Y = FMath::FloorToInt(Local.Y / CellSize);
	return FIntPoint(X, Y);
}

// Convert grid cell back to world position (useful for moving units)
FVector AThetaStarController::CellToWorld(const FIntPoint& Cell) const
{
	return GridOrigin + FVector(Cell.X * CellSize + CellSize * 0.5f, 
								Cell.Y * CellSize + CellSize * 0.5f, 
								0.f);
}