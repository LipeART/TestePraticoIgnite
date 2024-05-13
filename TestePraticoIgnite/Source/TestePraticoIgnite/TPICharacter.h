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

	// Inst�ncia do componente de pathfinding
	UPROPERTY(EditAnywhere, Category = "Pathfinding")
	UTPIPathfindingComponent* PathfindingComponent;

	//Componente utilizado para posicionar e movimentar a c�mera
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* CameraComponent;

	// Lista que armazena o caminho que est� sendo percorrido.
	TArray<FVector> CurrentPath;

	// �ndice do pr�ximo ponto a ser alcan�ado no caminho.
	int CurrentIndex;

	// Flag que indica se o personagem est� se movendo.
	bool bIsMoving = false;

	// Flag que indica se o personagem est� escalando.
	bool bIsClimbing = false;

	// Armazena dados sobre a parede onde o personagem est� escalando (se houver).
	FHitResult ClimbingLineTraceData;

public:	
	// Atualiza a movimenta��o do personagem
	virtual void Tick(float DeltaTime) override;

	// Configura��es dos controles
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Ao clicar com o bot�o esquerdo do mouse, executa os m�todos do componente de pathfinding.
	void GetPath();

	// Ao apertar A ou D, rotaciona a c�mera para a esquerda ou para a direita.
	void SetCameraRotation(float value);

	// Verifica se h� uma parede � frente para escalar.
	bool CheckIfCanClimb();
};
