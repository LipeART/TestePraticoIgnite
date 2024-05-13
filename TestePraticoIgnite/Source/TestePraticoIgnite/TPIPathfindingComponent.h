// Componente utilizado para realizar buscas por caminhos entre pontos no cen�rio

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPIPathfindingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTEPRATICOIGNITE_API UTPIPathfindingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UTPIPathfindingComponent();

protected:

	// Configuracoes iniciais
	virtual void BeginPlay() override;

	// Dist�ncia entre pontos vizinhos do "grafo" montado.
	UPROPERTY(EditDefaultsOnly, Category = "Pathfinding")
	float DistanceBetweenPoints = 50.0f;

	// Flag que definir� se elementos de debug ser�o exibidos na tela. 
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDebug = false;

	// Flag que define se pontos vizinhos na diagonal ser�o considerados.
	UPROPERTY(EditDefaultsOnly, Category = "Pathfinding")
	bool bIncludeDiagonals = true;

	// Par�metros de colis�o utilizados em Line Traces.
	FCollisionObjectQueryParams ObjectQueryParams;

	//Dimens�es do colisor do owner do componente.
	float CapsuleRadius;
	float CapsuleHalfHeight;

	// Posi��o inicial da busca.
	FVector StartPos;

	// Posi��o desejada.
	FVector EndPos;

	// Ponto mais pr�ximo encontrado ao final da busca.
	FVector TargetLocation;

	// Lista de pontos descobertos durante a busca.
	// Esta lista � montada seguindo uma estrutura de heap,
	// buscando garantir que o primeiro elemento seja o mais
	// pr�ximo do destino. 
	TArray<FVector> Discovered;

	// Lista de pontos analisados durante a busca.
	// N�o requer que esteja ordenada, logo � montada
	// como uma lista normal.
	TArray<FVector> Analysed;

	// Vetor que armazena dados sobre a superf�cie em que
	// o owner est� apoiado. � utilizado para obter os vizinhos
	// que ser�o descobertos e analisados durante a busca
	FVector Axis;

public:	

	// Verifica se o owner esta apoiado em uma parede ou no chao
	FVector GetOwnerSurfaceAxis(FVector currentLocation);
	
	// Realiza as configuracoes necessarias antes de iniciar a busca
	TArray<FVector> StartSearch(FVector posFinal);
	
	// Esvazia as listas de analisados e descobertos
	void ResetLists();

	// Realiza a busca e a montagem do caminho requisitado
	TArray<FVector> FindPath();

	// Adiciona um ponto a lista de pontos descobertos, buscando manter a estrutura de um heap 
	void Discover(FVector ponto);

	// No heap (lista de descobertos), retorna o index do pai do elemento de index i
	int Parent(int i);

	// Analisa o proximo ponto na lista de descobertos
	bool AnalyseNextLocation();

	// Remove o primeiro elemento da lista de descobertos, buscando manter a estrutura do heap
	FVector RemoveFromDiscovered();

	// Verifica e reorganiza a lista de descobertos para manter a estrutura de heap
	void Heapify(int i);

	// Procura pelas posicoes vizinhas de um local especifico
	TArray<FVector> GetValidNeighbors(FVector location);

	// Cria o caminho que sera seguido, a partir dos pontos analisados
	TArray<FVector> GeneratePath();

	// Procura qual ponto na lista de analisados � o mais perto de um ponto especifico
	FVector GetNearestInAnalysed(FVector location);
};
