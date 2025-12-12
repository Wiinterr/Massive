// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridManager.h"
#include "BoidsComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MASSIVE_API UBoidsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBoidsComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids")
	float NeighborRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids")
	float SeparationRadius = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids")
	float MaxForce = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids")
	float SeparationWeight = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids")
	float AlignmentWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids")
	float CohesionWeight = 1.0f;

	UPROPERTY()
	AGridManager* GridManager = nullptr;

	UFUNCTION(BlueprintCallable)
	FVector ComputeBoidsOffset();
};
