#pragma once

#include "CoreMinimal.h"
#include "GridManager.h"
#include "GameFramework/Actor.h"
#include "FlowFieldController.generated.h"

UCLASS()
class MASSIVE_API AFlowFieldController : public AActor
{
	GENERATED_BODY()

public:
	AFlowFieldController();

	AGridManager* GridManager;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FlowField")
	float AnglePenalty = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FlowField")
	FIntPoint TargetIndex = FIntPoint(5, 5);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FlowField")
	bool bDrawDebugPath = false;
	
	UFUNCTION(CallInEditor, BlueprintCallable, Category="FlowField")
	TArray<FGridCell> const& SetTargetCellByWorldLocation(FVector const& WorldLocation);

	UFUNCTION(CallInEditor, BlueprintCallable, Category="FlowField")
	FIntPoint WorldLocationToIndex(FVector const& WorldLocation);

	UFUNCTION(CallInEditor, BlueprintCallable, Category="FlowField")
	FVector GetFlowOfCell(FIntPoint index, TArray<FGridCell> OverrideGrid);

protected:
	virtual void BeginPlay() override;

private:
	
	void SetTargetCell(int32 const& x, int32 const& y);
	void ComputeIntegrationField();
	void ComputeDirections();
};
