#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AStarController.generated.h"

UCLASS()
class MASSIVE_API AAStarController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAStarController();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AStar")
	int32 GridWidth = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AStar")
	int32 GridHeight = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AStar")
	float CellSize = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AStar")
	FVector GridOrigin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AStar")
	TArray<int32> CostMap;

	UFUNCTION(CallInEditor, BlueprintCallable, Category="AStar")
	void GenerateCostMap();
	
	// Runtime options
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AStar")
	bool bAllowDiagonal = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AStar")
	bool bPreventDiagonalCutting = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AStar", meta=(ClampMin="1.0", ClampMax="2.0"))
	float DiagonalCost = 1.41421356237f;

	// Debug
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AStar")
	bool bDrawDebugPath = true;

	UFUNCTION(CallInEditor, BlueprintCallable, Category="AStar")
	void CreateDefaultCostMap(int32 InGridWidth = -1, int32 InGridHeight = -1);

	UFUNCTION(BlueprintCallable, Category="AStar")
	TArray<FVector> FindPath(const FVector& StartWorld, const FVector& GoalWorld);

	TArray<FIntPoint> RunAStar(const FIntPoint& StartCell, const FIntPoint& GoalCell);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	FORCEINLINE int32 XYToIndex(int32 X, int32 Y) const { return Y * GridWidth + X; }
	FORCEINLINE void IndexToXY(int32 Index, int32& OutX, int32& OutY) const { OutY = Index / GridWidth; OutX = Index % GridWidth; }
	bool IsInsideGrid(int32 X, int32 Y) const { return X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight; }
	bool IsWalkableIndex(int32 Index) const;
	float MovementCostBetween(int32 AIndex, int32 BIndex) const;
	TArray<int32> GetNeighborsIndices(int32 Index) const;

	FIntPoint WorldToCell(const FVector& WorldLocation) const;
	FVector CellToWorld(const FIntPoint& Cell) const;
	
	void DrawDebugGrid();

    // Open set (binary heap with positions for decrease-key)
    struct FHeapItem
    {
        int32 NodeIndex;
        float F;
    };

    struct FBinaryHeapOpenSet
    {
        void Init(int32 NumNodes)
        {
            Heap.Empty();
            Pos.Init(-1, NumNodes);
        }

        bool IsEmpty() const { return Heap.Num() == 0; }

        void Clear()
        {
            for (const FHeapItem& It : Heap)
            {
                if (It.NodeIndex >= 0 && It.NodeIndex < Pos.Num())
                    Pos[It.NodeIndex] = -1;
            }
            Heap.Empty();
        }

        void Push(int32 NodeIndex, float FVal)
        {
            FHeapItem Item{ NodeIndex, FVal };
            Heap.Add(Item);
            int32 i = Heap.Num() - 1;
            Pos[NodeIndex] = i;
            SiftUp(i);
        }

        int32 PopMin()
        {
            if (Heap.Num() == 0) return -1;
            int32 Best = Heap[0].NodeIndex;
            SwapNodes(0, Heap.Num() - 1);
            Pos[Best] = -1;
            Heap.Pop();
            if (Heap.Num() > 0) SiftDown(0);
            return Best;
        }

        void PushOrDecrease(int32 NodeIndex, float NewF)
        {
            if (NodeIndex < 0 || NodeIndex >= Pos.Num()) return;
            int32 p = Pos[NodeIndex];
            if (p == -1)
            {
                Push(NodeIndex, NewF);
            }
            else
            {
                if (NewF < Heap[p].F)
                {
                    Heap[p].F = NewF;
                    SiftUp(p);
                }
            }
        }

        void ReservePos(int32 Num) { Pos.Init(-1, Num); }

    private:
        TArray<FHeapItem> Heap;
        TArray<int32> Pos;

        void SwapNodes(int32 A, int32 B)
        {
            if (A == B) return;
            Exchange(Heap[A], Heap[B]);
            Pos[Heap[A].NodeIndex] = A;
            Pos[Heap[B].NodeIndex] = B;
        }

        void SiftUp(int32 i)
        {
            while (i > 0)
            {
                int32 Parent = (i - 1) >> 1;
                if (Heap[i].F < Heap[Parent].F)
                {
                    SwapNodes(i, Parent);
                    i = Parent;
                }
                else break;
            }
        }

        void SiftDown(int32 i)
        {
            const int32 N = Heap.Num();
            while (true)
            {
                int32 Left = i * 2 + 1;
                int32 Right = Left + 1;
                int32 Smallest = i;
                if (Left < N && Heap[Left].F < Heap[Smallest].F) Smallest = Left;
                if (Right < N && Heap[Right].F < Heap[Smallest].F) Smallest = Right;
                if (Smallest != i) { SwapNodes(i, Smallest); i = Smallest; }
                else break;
            }
        }
    };

    // Node arrays used in the search (simple POD arrays)
    struct FSearchNode
    {
        float G = TNumericLimits<float>::Max(); // cost from stat
        float H = 0.f; // heuristic
        float F = TNumericLimits<float>::Max(); // final cost
        int32 Parent = -1;
        bool bClosed = false;
    };
};
