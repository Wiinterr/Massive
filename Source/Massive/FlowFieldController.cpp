#include "FlowFieldController.h"
#include "DrawDebugHelpers.h"
#include "FlowFieldSubsystem.h"

AFlowFieldController::AFlowFieldController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFlowFieldController::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFlowFieldController::GenerateGrid()
{
	UFlowFieldSubsystem* FlowSubsystem = GetGameInstance()->GetSubsystem<UFlowFieldSubsystem>();
	if (!FlowSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Flow Field Subsystem not found when Generating Grid"));
		return;
	}

	FlowSubsystem->CreateGrid(GridWidth, GridHeight, CellSize, GetActorLocation());
	FlowSubsystem->SetTargetCell(TargetIndex.X, TargetIndex.Y);
	FlowSubsystem->ComputeIntegrationField();
	//FlowSubsystem->ComputeFlowDirections();

	DrawDebugGrid();
}

void AFlowFieldController::DrawDebugGrid()
{
	UFlowFieldSubsystem* FlowSubsystem = GetGameInstance()->GetSubsystem<UFlowFieldSubsystem>();
	if (!FlowSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Flow Field Subsystem not found when Drawing Debug Grid"));
		return;
	}
	
	const TArray<FFlowFieldCell>& GridCells = FlowSubsystem->GetGridCells();
	
	for (const FFlowFieldCell& Cell : GridCells)
	{
		DrawDebugBox(
			GetWorld(),
			Cell.WorldLocation,
			FVector(CellSize * 0.5, CellSize * 0.5, 5.0f),
			FColor::White,
			true,
			-1.f
			);

		DrawDebugString(
			GetWorld(),
			Cell.WorldLocation,
			FString::FromInt(Cell.IntegrationValue),
			nullptr,
			FColor::White,
			-1.f
			);

		DrawDebugDirectionalArrow(
			GetWorld(),
			Cell.WorldLocation,
			Cell.WorldLocation + Cell.FlowDirection * 50.f,
			10.f,
			FColor::White,
			true,
			-1.f,
			0,
			2.f
			);
	}
}