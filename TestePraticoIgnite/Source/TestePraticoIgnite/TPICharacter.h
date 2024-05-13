// Personagem criado para este teste, que se move utilizando o componente de pathfinding criado.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPICharacter.generated.h"

class UTPIPathfindingComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class TESTEPRATICOIGNITE_API ATPICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATPICharacter();

protected:
	virtual void BeginPlay() override;

	// Instância do componente de pathfinding
	UPROPERTY(EditAnywhere, Category = "Pathfinding")
	UTPIPathfindingComponent* PathfindingComponent;

	//Componente utilizado para posicionar e movimentar a câmera
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* CameraComponent;

	// Lista que armazena o caminho que está sendo percorrido.
	TArray<FVector> CurrentPath;

	// Índice do próximo ponto a ser alcançado no caminho.
	int CurrentIndex;

	// Flag que indica se o personagem está se movendo.
	bool bIsMoving = false;

	// Flag que indica se o personagem está escalando.
	bool bIsClimbing = false;

	// Armazena dados sobre a parede onde o personagem está escalando (se houver).
	FHitResult ClimbingLineTraceData;

public:	
	// Atualiza a movimentação do personagem
	virtual void Tick(float DeltaTime) override;

	// Configurações dos controles
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Ao clicar com o botão esquerdo do mouse, executa os métodos do componente de pathfinding.
	void GetPath();

	// Ao apertar A ou D, rotaciona a câmera para a esquerda ou para a direita.
	void SetCameraRotation(float value);

	// Verifica se há uma parede à frente para escalar.
	bool CheckIfCanClimb();
};
