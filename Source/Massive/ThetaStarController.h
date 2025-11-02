#pragma once

#include "CoreMinimal.h"
#include "GridManager.h"
#include "GameFramework/Actor.h"
#include "ThetaStarController.generated.h"

UCLASS()
class MASSIVE_API AThetaStarController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AThetaStarController();

	AGridManager* GridManager;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ThetaStar", meta=(ClampMin="1.0", ClampMax="2.0"))
	float DiagonalCost = 1.41421356237f;

	UFUNCTION(BlueprintCallable, Category="ThetaStar")
	TArray<FVector> FindPath(const FVector& StartWorld, const FVector& GoalWorld);

	TArray<FIntPoint> RunThetaStar(const FIntPoint& StartCell, const FIntPoint& GoalCell);
	
	bool HasLineOfSight(const int32 FromIdx, int32 ToIdx) const;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	FORCEINLINE void IndexToXY(int32 Index, int32& OutX, int32& OutY) const
	{
		OutY = Index / GridManager->GridWidth; OutX = Index % GridManager->GridWidth;
	}

	float MovementCostBetween(int32 AIndex, int32 BIndex) const;

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
