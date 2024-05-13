#include "TPIPathfindingComponent.h"
#include "TPIPathfindingComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UTPIPathfindingComponent::UTPIPathfindingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Configuracoes iniciais
void UTPIPathfindingComponent::BeginPlay() {

	Super::BeginPlay();
	ACharacter* owner = Cast<ACharacter>(GetOwner());

	if (owner) {
	
		UCapsuleComponent* capsule = owner->GetCapsuleComponent();
		CapsuleRadius = capsule->GetScaledCapsuleRadius();
		CapsuleHalfHeight = capsule->GetScaledCapsuleHalfHeight();
	}
	else {
	
		CapsuleRadius = CapsuleHalfHeight = 50.0f;
	}

	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	Axis = GetOwnerSurfaceAxis(GetOwner()->GetActorLocation());
}

// Verifica se o owner esta apoiado em uma parede ou no chao
FVector UTPIPathfindingComponent::GetOwnerSurfaceAxis(FVector currentLocation)
{
	FVector axis;

	FVector endFront = currentLocation + ((CapsuleRadius + 1) * GetOwner()->GetActorRotation().Vector());
	FVector endDown = FVector(currentLocation.X, currentLocation.Y, currentLocation.Z - CapsuleHalfHeight);

	FHitResult frontResult;
	FHitResult downResult;

	GetWorld()->LineTraceSingleByObjectType(frontResult, currentLocation, endFront, ObjectQueryParams);
	GetWorld()->LineTraceSingleByObjectType(downResult, currentLocation, endDown, ObjectQueryParams);

	axis = (frontResult.ImpactNormal * -1.0f) + downResult.ImpactNormal;

	return axis;
}

// Realiza as configuracoes necessarias antes de iniciar a busca
TArray<FVector> UTPIPathfindingComponent::StartSearch(FVector posFinal) {

	ResetLists();
	StartPos = GetOwner()->GetActorLocation();
	EndPos = posFinal;

	if (bDebug) {
	
		DrawDebugSphere(GetWorld(), StartPos, 30.0f, 32, FColor::White, false, 1.0f);
		DrawDebugSphere(GetWorld(), EndPos, 30.0f, 32, FColor::White, false, 1.0f);
	}
	
	if (StartPos == EndPos) {
	
		return TArray<FVector>();
	}

	return FindPath();
}

// Esvazia as listas de analisados e descobertos
void UTPIPathfindingComponent::ResetLists() {

	Discovered.Empty();
	Analysed.Empty();
}

// Realiza a busca e a montagem do caminho requisitado
TArray<FVector> UTPIPathfindingComponent::FindPath() {

	TArray<FVector> path = TArray<FVector>();

	Discover(StartPos);

	bool bTargetFound = false;

	do {

		bTargetFound = AnalyseNextLocation();

		if (bDebug) {
		
			for (FVector a : Analysed) {

				DrawDebugSphere(GetWorld(), a, 10.0f, 32, FColor::Yellow, false, 1.0f);
			}

			for (FVector d : Discovered) {

				DrawDebugSphere(GetWorld(), d, 10.0f, 32, FColor::Red, false, 1.0f);
			}
		}
	} while (Discovered.Num() > 0 && !bTargetFound);

	if (bTargetFound) {

		path = GeneratePath();

		if (bDebug) {
		
			for (FVector p : path) {

				DrawDebugSphere(GetWorld(), p, 10.0f, 32, FColor::Blue, false, 1.0f);
			}
		}
	}

	return path;
}

// Adiciona um ponto a lista de pontos descobertos, buscando manter a estrutura de um heap 
void UTPIPathfindingComponent::Discover(FVector ponto)
{
	int i = Discovered.Add(ponto);

	while (i != 0 && FVector::Distance(Discovered[Parent(i)], EndPos) > FVector::Distance(Discovered[i], EndPos)) {

		Discovered.Swap(i, Parent(i));
		i = Parent(i);
	}
}

// No heap (lista de descobertos), retorna o index do pai do elemento de index i
int UTPIPathfindingComponent::Parent(int i)
{
	return (i - 1) / 2;
}

// Analisa o proximo ponto na lista de descobertos
bool UTPIPathfindingComponent::AnalyseNextLocation() {

	FVector currentPoint = RemoveFromDiscovered();

	Analysed.Add(currentPoint);

	TArray<FVector> neighbors = GetValidNeighbors(currentPoint);

	for (FVector n : neighbors)
	{
		if (!Analysed.Contains(n)) {

			int indexInDiscovered = Discovered.Find(n);

			if (indexInDiscovered == -1) {

				Discover(n);
			}
			else {

				Heapify(indexInDiscovered);
			}

			if (FVector::Distance(n, EndPos) < DistanceBetweenPoints) {

				TargetLocation = n;
				return true;
			}
		}
	}

	return false;
}

// Remove o primeiro elemento da lista de descobertos, buscando manter a estrutura do heap
FVector UTPIPathfindingComponent::RemoveFromDiscovered()
{
	FVector root = Discovered[0];
	Discovered[0] = Discovered[Discovered.Num() - 1];
	Discovered.RemoveAt(Discovered.Num() - 1);

	Heapify(0);

	return root;
}

// Verifica e reorganiza a lista de descobertos para manter a estrutura de heap
void UTPIPathfindingComponent::Heapify(int i)
{
	int l = (2 * i + 1);;
	int r = (2 * i + 2);
	int nearest = i;

	if (l < Discovered.Num() && FVector::Distance(Discovered[l], EndPos) < FVector::Distance(Discovered[i], EndPos))
		nearest = l;
	if (r < Discovered.Num() && FVector::Distance(Discovered[r], EndPos) < FVector::Distance(Discovered[nearest], EndPos))
		nearest = r;
	if (nearest != i) {
		Discovered.Swap(i, nearest);
		Heapify(nearest);
	}
}

// Procura pelas posicoes vizinhas de um local especifico
TArray<FVector> UTPIPathfindingComponent::GetValidNeighbors(FVector location)
{
	TArray<FVector> neighbors;
	FHitResult hit;
	bool bHit;

	// Adiciona os vizinhos dos lados, que sempre vao ser considerados
	if (Axis.X != 0.0f || Axis.Z != 0.0f) {
	
		neighbors.Add(FVector(location.X, location.Y + DistanceBetweenPoints, location.Z));
		neighbors.Add(FVector(location.X, location.Y - DistanceBetweenPoints, location.Z));
	}	

	// Adiciona os vizinhos da frente e de trás se estiver apoiado no chao
	if (Axis.Z != 0.0f) {
	
		neighbors.Add(FVector(location.X + DistanceBetweenPoints, location.Y, location.Z));
		neighbors.Add(FVector(location.X - DistanceBetweenPoints, location.Y, location.Z));

		if (bIncludeDiagonals) {
		
			neighbors.Add(FVector(location.X + DistanceBetweenPoints, location.Y + DistanceBetweenPoints, location.Z));
			neighbors.Add(FVector(location.X + DistanceBetweenPoints, location.Y - DistanceBetweenPoints, location.Z));
			neighbors.Add(FVector(location.X - DistanceBetweenPoints, location.Y - DistanceBetweenPoints, location.Z));
			neighbors.Add(FVector(location.X - DistanceBetweenPoints, location.Y + DistanceBetweenPoints, location.Z));
		}
	}
	
	// Adiciona os vizinhos de cima e de baixo se estiver apoiado em uma parede à frente
	if (Axis.X != 0.0f) {
	
		neighbors.Add(FVector(location.X, location.Y, location.Z + DistanceBetweenPoints));
		neighbors.Add(FVector(location.X, location.Y, location.Z - DistanceBetweenPoints));

		if (bIncludeDiagonals) {
		
			neighbors.Add(FVector(location.X, location.Y + DistanceBetweenPoints, location.Z + DistanceBetweenPoints));
			neighbors.Add(FVector(location.X, location.Y - DistanceBetweenPoints, location.Z + DistanceBetweenPoints));
			neighbors.Add(FVector(location.X, location.Y + DistanceBetweenPoints, location.Z - DistanceBetweenPoints));
			neighbors.Add(FVector(location.X, location.Y - DistanceBetweenPoints, location.Z - DistanceBetweenPoints));
		}
	}
	
	int i = 0;
	
	while (i < neighbors.Num()) {

		bHit = GetWorld()->LineTraceSingleByObjectType(hit, location, neighbors[i], ObjectQueryParams);

		if (bHit && hit.ImpactPoint != neighbors[i]) {
		
			neighbors.RemoveAt(i);
		}
		else {
		
			i++;
		}
	}

	return neighbors;
}

// Cria o caminho que será seguido, a partir dos pontos analisados
TArray<FVector> UTPIPathfindingComponent::GeneratePath()
{
	TArray<FVector> invertedPath = TArray<FVector>();
	TArray<FVector> path = TArray<FVector>();

	invertedPath.Add(TargetLocation);
	FVector currentLocation = TargetLocation;

	while (currentLocation != StartPos) {

		currentLocation = GetNearestInAnalysed(currentLocation);
		invertedPath.Add(currentLocation);
	}

	for (int i = invertedPath.Num() - 1; i >= 0; i--) {

		path.Add(invertedPath[i]);
	}

	return path;
}

// Procura qual ponto na lista de analisados é o mais perto de um ponto especifico
FVector UTPIPathfindingComponent::GetNearestInAnalysed(FVector location)
{
	int nearest = Analysed.Num() - 1;
	int i = nearest;

	while (i >= 0) {
	
		if (FVector::Distance(Analysed[i], location) < FVector::Distance(Analysed[nearest], location)) {
		
			nearest = i;
		}
		i--;
	}

	FVector result = Analysed[nearest];
	Analysed.RemoveAt(nearest);

	return result;
}