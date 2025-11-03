#include "ThetaStarController.h"
#include "DrawDebugHelpers.h"
#include "Components/LineBatchComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

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
	if (!GridManager)
	{
		GridManager = Cast<AGridManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass())
		);
	}

	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ThetaStarController: Could not find GridManager!"));
	}
}

// Called every frame
void AThetaStarController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

TArray<FVector> AThetaStarController::FindPath(const FVector& StartWorld, const FVector& GoalWorld)
{
	// Convert to grid space
	FIntPoint StartCell = GridManager->WorldToCell(StartWorld);
	FIntPoint GoalCell  = GridManager->WorldToCell(GoalWorld);

	// Check grid bounds + walkability
	if (!GridManager->IsInside(StartCell.X, StartCell.Y) || !GridManager->IsInside(GoalCell.X, GoalCell.Y))
	{
		return {};
	}
	
	if (GridManager->Grid[GridManager->XYToIndex(StartCell.X, StartCell.Y)].Cost < 0 ||
		GridManager->Grid[GridManager->XYToIndex(GoalCell.X, GoalCell.Y)].Cost < 0)
	{
		return {};
	}

	// Run the A* core (this part can stay private, taking FIntPoints)
	TArray<FIntPoint> CellPath = RunThetaStar(StartCell, GoalCell);

	// Convert back to world space
	TArray<FVector> WorldPath;
	for (const FIntPoint& Cell : CellPath)
	{
		WorldPath.Add(GridManager->CellToWorld(Cell));
	}

	// Optional debug draw
	if (GridManager->bDrawDebug && GridManager->bDrawThetaStarPath)
	{
		// Draw line for path
		for (int32 i = 0; i + 2 < WorldPath.Num(); i++)
		{
			DrawDebugLine(
				GetWorld(),
				WorldPath[i],
				WorldPath[i + 1],
				FColor::Black,
				false,
				1.5f,
				0,
				5.f
				);
		}

		// Draw arrow as last line in path
		DrawDebugDirectionalArrow(
			GetWorld(),
			WorldPath[WorldPath.Num() - 2],
			WorldPath.Last(),
			20.f,             // arrow size
			FColor::Black,    // make it stand out
			false,            // persistent lines?
			1.5f,              // life time
			0,                // depth priority
			5.f               // thickness
		);
	}

	return WorldPath;
}

TArray<FIntPoint> AThetaStarController::RunThetaStar(const FIntPoint& StartCell, const FIntPoint& GoalCell)
{
    TArray<FIntPoint> ResultPath;

    // Validate cells in-bounds
    if (!GridManager->IsInside(StartCell.X, StartCell.Y) || !GridManager->IsInside(GoalCell.X, GoalCell.Y))
    {
        return ResultPath;
    }

    const int32 StartIdx = GridManager->XYToIndex(StartCell.X, StartCell.Y);
    const int32 GoalIdx  = GridManager->XYToIndex(GoalCell.X, GoalCell.Y);

    // Validate walkability (-1 means blocked)
    if (!GridManager->Grid.IsValidIndex(StartIdx) || !GridManager->Grid.IsValidIndex(GoalIdx) ||
        GridManager->Grid[StartIdx].Cost < 0 || GridManager->Grid[GoalIdx].Cost < 0)
    {
        return ResultPath;
    }

    const int32 NumNodes = GridManager->GridWidth * GridManager->GridHeight;

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

    	int32 CurrX, CurrY;
    	IndexToXY(Curr, CurrX, CurrY);
    	
        // Expand neighbors
    	const TArray<const FGridCell*> Neighbors = GridManager->GetNeighbors(CurrX, CurrY);
    	for (const FGridCell* NbCell : Neighbors)
    	{
    		const int32 Nb = GridManager->XYToIndex(NbCell->X, NbCell->Y);
    		if (SearchNodes[Nb].bClosed) continue;

    		float TerrainCost = (GridManager->Grid.IsValidIndex(Nb) ? FMath::Max(1, GridManager->Grid[Nb].Cost) : 1);
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
		int32 Idx = GridManager->XYToIndex(X, Y);
		if (!GridManager->Grid.IsValidIndex(Idx) || !GridManager->Grid[GridManager->XYToIndex(X, Y)].bIsBlocked)
			return false;

		if (X == X1 && Y == Y1) break;

		int32 E2 = 2 * Err;
		if (E2 > -Dy) { Err -= Dy; X += SX; }
		if (E2 <  Dx) { Err += Dx; Y += SY; }
	}

	return true;
}