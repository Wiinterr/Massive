#include "FlowFieldSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UFlowFieldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("FlowFieldSubsystem Initialized"));
}

void UFlowFieldSubsystem::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("FlowFieldSubsystem Deinitialized"));
}