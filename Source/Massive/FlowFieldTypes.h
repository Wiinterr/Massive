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
		, IntegrationValue(TNumericLimits<float>::Max()){}

	UPROPERTY(VisibleAnywhere)
	FVector WorldLocation;

	UPROPERTY(VisibleAnywhere)
	FVector FlowDirection;

	UPROPERTY(VisibleAnywhere)
	int32 GridX;

	UPROPERTY(VisibleAnywhere)
	int32 GridY;

	UPROPERTY(VisibleAnywhere)
	float Cost;

	UPROPERTY(VisibleAnywhere)
	float IntegrationValue;
};

