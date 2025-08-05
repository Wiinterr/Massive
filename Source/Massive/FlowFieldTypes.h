#pragma once

#include "CoreMinimal.h"
#include "FlowFieldTypes.generated.h"

USTRUCT(BlueprintType)
struct FFlowFieldCell
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector WorldLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector FlowDirection = FVector(0,0,0);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 GridX = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 GridY = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Cost = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 IntegrationValue = TNumericLimits<int32>::Max();
};

