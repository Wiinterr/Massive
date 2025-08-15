#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlowFieldTypes.h"
#include "FlowFieldController.generated.h"

UCLASS()
class MASSIVE_API AFlowFieldController : public AActor
{
	GENERATED_BODY()

public:
	AFlowFieldController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FlowField")
	int32 GridWidth = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FlowField")
	int32 GridHeight = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FlowField")
	float CellSize = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FlowField")
	FIntPoint TargetIndex = FIntPoint(5, 5);
	
	UFUNCTION(CallInEditor, BlueprintCallable, Category="FlowField")
	void GenerateGrid();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="FlowField")
	void SetTargetCellByWorldLocation(FVector const& WorldLocation);

	UFUNCTION(CallInEditor, BlueprintCallable, Category="FlowField")
	FIntPoint WorldLocationToIndex(FVector const& WorldLocation);

	UFUNCTION(CallInEditor, BlueprintCallable, Category="FlowField")
	FVector GetFlowOfCell(FIntPoint index);
	
protected:
	virtual void BeginPlay() override;

private:
	FVector GridOrigin;
	TArray<FFlowFieldCell> GridCells;
	
	void CreateGrid();
	void DrawDebugGrid();
	
	void SetTargetCell(int32 const& x, int32 const& y);
	void ComputeIntegrationField();
	void ComputeDirections();
	TArray<FFlowFieldCell*> GetCellNeighbors(FFlowFieldCell const& Cell);
	FFlowFieldCell* GetCellAt(int32 const& x, int32 const& y);
	
};
