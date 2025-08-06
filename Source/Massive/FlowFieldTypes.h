#pragma once

#include "CoreMinimal.h"
#include "FlowFieldTypes.generated.h"

USTRUCT(BlueprintType)
struct FFlowFieldCell
{
	GENERATED_BODY()

	FFlowFieldCell()
		: WorldLocation(FVector::ZeroVector)
		, FlowDirection(FVector::ZeroVector)
		, GridX(0), GridY(0), Cost(1)
		, IntegrationValue(TNumericLimits<int32>::Max()){}

	UPROPERTY(VisibleAnywhere)
	FVector WorldLocation;

	UPROPERTY(VisibleAnywhere)
	FVector FlowDirection;

	UPROPERTY(VisibleAnywhere)
	int32 GridX;

	UPROPERTY(VisibleAnywhere)
	int32 GridY;

	UPROPERTY(VisibleAnywhere)
	int32 Cost;

	UPROPERTY(VisibleAnywhere)
	int32 IntegrationValue;
};

