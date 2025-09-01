#include "AStarController.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// Sets default values
AAStarController::AAStarController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAStarController::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AAStarController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAStarController::GenerateCostMap()
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
}

void AAStarController::CreateDefaultCostMap(int32 InGridWidth, int32 InGridHeight)
{
	if (InGridWidth > 0) GridWidth = InGridWidth;
	if (InGridHeight > 0) GridHeight = InGridHeight;

	CostMap.Init(1, GridWidth * GridHeight); // default cost 1, walkable
}

bool AAStarController::IsWalkableIndex(int32 Index) const
{
	if (Index < 0 || Index >= CostMap.Num()) return false;
	return CostMap[Index] >= 0;
}

float AAStarController::MovementCostBetween(int32 AIndex, int32 BIndex) const
{
	int Ax, Ay, Bx, By;
	IndexToXY(AIndex, Ax, Ay);
	IndexToXY(BIndex, Bx, By);

	int Dx = FMath::Abs(Ax - Bx);
	int Dy = FMath::Abs(Ay - By);

	if (Dx + Dy == 1) return 1.f;
	return DiagonalCost;
}

TArray<int32> AAStarController::GetNeighborsIndices(int32 Index) const
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

TArray<FIntPoint> AAStarController::FindPathBP(FIntPoint StartCell, FIntPoint GoalCell)
{
    TArray<FIntPoint> ResultPath;

    // Validate inputs
    if (!IsInsideGrid(StartCell.X, StartCell.Y) || !IsInsideGrid(GoalCell.X, GoalCell.Y))
        return ResultPath;

    const int StartIdx = XYToIndex(StartCell.X, StartCell.Y);
    const int GoalIdx  = XYToIndex(GoalCell.X, GoalCell.Y);

    if (!IsWalkableIndex(StartIdx) || !IsWalkableIndex(GoalIdx))
        return ResultPath;

    const int NumNodes = GridWidth * GridHeight;

    // Allocate/search buffers
    TArray<FSearchNode> SearchNodes;
    SearchNodes.SetNum(NumNodes);
    for (int i = 0; i < NumNodes; ++i)
    {
        SearchNodes[i].G = TNumericLimits<float>::Max();
        SearchNodes[i].Parent = -1;
        SearchNodes[i].bClosed = false;
    }

    FBinaryHeapOpenSet OpenSet;
    OpenSet.ReservePos(NumNodes);
    OpenSet.Init(NumNodes);

    auto Heuristic = [&](int A, int B)->float
    {
        int Ax,Ay,Bx,By;
        IndexToXY(A, Ax, Ay);
        IndexToXY(B, Bx, By);
        int const Dx = FMath::Abs(Ax - Bx);
        int const Dy = FMath::Abs(Ay - By);
    	
        // Octile heuristic (good for 8-way)
        const float F = float(FMath::Min(Dx, Dy));
        return ( (float)(FMath::Max(Dx, Dy) - F) + DiagonalCost * F );
    };

    // init start
    SearchNodes[StartIdx].G = 0.f;
    SearchNodes[StartIdx].H = Heuristic(StartIdx, GoalIdx);
    SearchNodes[StartIdx].F = SearchNodes[StartIdx].G + SearchNodes[StartIdx].H;
    OpenSet.Push(StartIdx, SearchNodes[StartIdx].F);

    bool bFound = false;

    while (!OpenSet.IsEmpty())
    {
        int const Curr = OpenSet.PopMin();
        if (Curr == -1) break;

        if (Curr == GoalIdx)
        {
            bFound = true;
            break;
        }

        SearchNodes[Curr].bClosed = true;

        // expand neighbors
        TArray<int32> const Neighbors = GetNeighborsIndices(Curr);
        for (int32 const Nb : Neighbors)
        {
            if (SearchNodes[Nb].bClosed) continue;

            float const MoveCost = MovementCostBetween(Curr, Nb);
            float const TentativeG = SearchNodes[Curr].G + MoveCost * ( (CostMap.IsValidIndex(Nb) ? (float)CostMap[Nb] : 1.f) );

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

    // Reconstruct path
    if (bFound)
    {
        TArray<int32> PathIdx;
        int Curr = GoalIdx;
        while (Curr != -1)
        {
            PathIdx.Add(Curr);
            Curr = SearchNodes[Curr].Parent;
        }
        Algo::Reverse(PathIdx);

        // Convert to FIntPoint list
        for (int Idx : PathIdx)
        {
            int X, Y;
            IndexToXY(Idx, X, Y);
            ResultPath.Add(FIntPoint(X, Y));
        }

        // Optional debug draw
        if (bDrawDebugPath)
        {
            DrawDebugPath(PathIdx);
        }
    }

    return ResultPath;
}

void AAStarController::DrawDebugPath(const TArray<int32>& PathIndices) const
{
	if (PathIndices.Num() < 2) return;

	UWorld* W = GetWorld();
	if (!W) return;

	for (int i = 0; i < PathIndices.Num() - 1; ++i)
	{
		int32 A = PathIndices[i];
		int32 B = PathIndices[i+1];
		int Ax,Ay,Bx,By;
		IndexToXY(A, Ax, Ay);
		IndexToXY(B, Bx, By);

		FVector LocA = GridOrigin + FVector(Ax * CellSize, Ay * CellSize, 50.f);
		FVector LocB = GridOrigin + FVector(Bx * CellSize, By * CellSize, 50.f);

		DrawDebugLine(W, LocA, LocB, DebugPathColor, true, 10.0f, 0, 4.f);
		DrawDebugPoint(W, LocA, 8.f, DebugPathColor, true, 10.f);
	}
	// Draw last point
	int Last = PathIndices.Last();
	int LX, LY; IndexToXY(Last, LX, LY);
	DrawDebugPoint(GetWorld(), GridOrigin + FVector(LX * CellSize, LY * CellSize, 50.f), 8.f, DebugPathColor, true, 10.f);
}