#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

protected:
	virtual void BeginPlay() override;

private:
	void DrawDebugGrid();
	
};
