#include "BoidsComponent.h"
#include "Kismet/GameplayStatics.h"

UBoidsComponent::UBoidsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBoidsComponent::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridManager::StaticClass(), Found);

	if (Found.Num() > 0)
	{
		GridManager = Cast<AGridManager>(Found[0]);
	}

	if (!GridManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("BoidsComponent could not find GridManager!"));
	}
}

FVector UBoidsComponent::ComputeBoidsOffset()
{
	if (!GridManager) return FVector::ZeroVector;

	AActor* Owner = GetOwner();
	if (!Owner) return FVector::ZeroVector;

	const FVector MyPos = Owner->GetActorLocation();
	const FIntPoint MyCell = GridManager->WorldToCell(MyPos);

	// Collect nearby actors
	TArray<AActor*> Neighbors;

	// Scan nearby grid cells
	const int CellRadius = FMath::CeilToInt(NeighborRadius / GridManager->CellSize);

	for (int y = -CellRadius; y <= CellRadius; ++y)
	{
		for (int x = -CellRadius; x <= CellRadius; ++x)
		{
			const int NX = MyCell.X + x;
			const int NY = MyCell.Y + y;

			if (!GridManager->IsInside(NX, NY)) continue;

			const FVector CellWorld = GridManager->CellToWorld(FIntPoint(NX, NY));

			// Very cheap distance check
			if (FVector::DistSquared(CellWorld, MyPos) > NeighborRadius * NeighborRadius) continue;

			// Now find actors near this cell
			TArray<AActor*> Temp;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), Owner->GetClass(), Temp);

			for (AActor* Other : Temp)
			{
				//UE_LOG(LogTemp, Warning, TEXT("The Actor's name is: %s"), *Other->GetName());
				
				if (!Other || Other == Owner) continue;

				const float D = FVector::Dist(MyPos, Other->GetActorLocation());
				if (D <= NeighborRadius)
				{
					Neighbors.Add(Other);
				}
			}
		}
	}

	if (Neighbors.Num() == 0) return FVector::ZeroVector;
	
	FVector Separation = FVector::ZeroVector;
	FVector Alignment = FVector::ZeroVector;
	FVector Cohesion = FVector::ZeroVector;

	int BoidsToAvoid = 0;

	for (AActor* Neighbor : Neighbors)
	{
		const FVector NeighborPos = Neighbor->GetActorLocation();
		const float Dist = FVector::Dist(MyPos, NeighborPos);

		if (Dist < KINDA_SMALL_NUMBER) continue;

		// Separation
		if (Dist <= SeparationRadius)
		{
			Separation += (MyPos - NeighborPos).GetSafeNormal() / Dist;
		}

		Alignment += Neighbor->GetVelocity();
		Cohesion += NeighborPos;

		BoidsToAvoid++;
	}

	if (BoidsToAvoid == 0) return FVector::ZeroVector;
	
	//UE_LOG(LogTemp, Warning, TEXT("We made it past!"));
	
	Separation /= BoidsToAvoid;
	Alignment /= BoidsToAvoid;
	Cohesion /= BoidsToAvoid;

	Separation = Separation.GetSafeNormal();
	Cohesion = (Cohesion - MyPos).GetSafeNormal();
	Alignment = Alignment.GetSafeNormal();

	FVector Steering =
		(Separation * SeparationWeight) +
		(Alignment * AlignmentWeight) +
		(Cohesion * CohesionWeight);

	return Steering.GetClampedToSize(0.f, MaxForce);
}
