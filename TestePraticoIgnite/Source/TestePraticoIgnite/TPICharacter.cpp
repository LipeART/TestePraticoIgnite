#include "TPICharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "TPIPathfindingComponent.h"
#include "DrawDebugHelpers.h"

ATPICharacter::ATPICharacter()
{
 	PrimaryActorTick.bCanEverTick = true;

	PathfindingComponent = CreateDefaultSubobject<UTPIPathfindingComponent>("PathfindingComponent");
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCapsuleComponent()->InitCapsuleSize(60.0f, 110.0f);

	GetCharacterMovement()->bOrientRotationToMovement = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = false;
	SpringArmComponent->bInheritYaw = false;
	SpringArmComponent->bInheritPitch = false;
	SpringArmComponent->bInheritRoll = false;

	SpringArmComponent->TargetArmLength = 1500.0f;
	SpringArmComponent->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	SpringArmComponent->bDoCollisionTest = false;

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -110.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
}

void ATPICharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Atualiza a movimentação do personagem
void ATPICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMoving) {
	
		if (FVector::Distance(GetActorLocation(), CurrentPath[CurrentIndex]) < GetCapsuleComponent()->GetUnscaledCapsuleRadius() || FVector::Distance(GetActorLocation(), CurrentPath[CurrentIndex]) < GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()) {

			CurrentIndex++;
			bIsMoving = CurrentIndex != CurrentPath.Num();
		}
		else {
		
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CurrentPath[CurrentIndex]);
			AddMovementInput(LookAtRotation.Vector(), 1.0f);			
		}
	}
	else {
	
		GetMovementComponent()->StopMovementImmediately();
	}
	
	CheckIfCanClimb();
	if (bIsClimbing) {

		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ClimbingLineTraceData.ImpactPoint));
	}
	else {

		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

// Configurações dos controles
void ATPICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &ATPICharacter::GetPath);
	PlayerInputComponent->BindAxis("Camera", this, &ATPICharacter::SetCameraRotation);
}

// Ao clicar com o botão esquerdo do mouse, executa os métodos do componente de pathfinding.
void ATPICharacter::GetPath()
{
	FHitResult Click;
	APlayerController* MyController = Cast<APlayerController>(GetController());
	if (MyController && MyController->GetHitResultUnderCursor(ECollisionChannel::ECC_WorldStatic, true, Click)) {
	
		FVector ClickLocation = Click.ImpactPoint;
		TArray<FVector> FoundPath = PathfindingComponent->StartSearch(ClickLocation);

		if (FoundPath.Num() != 0) {
		
			CurrentPath = FoundPath;
			CurrentIndex = 0;
			bIsMoving = true;
		}
	}
}

// Ao apertar A ou D, rotaciona a câmera para a esquerda ou para a direita.
void ATPICharacter::SetCameraRotation(float value)
{
	if (value != 0) {

		float currentYaw = SpringArmComponent->GetRelativeRotation().Yaw;
		SpringArmComponent->SetRelativeRotation(FRotator(-60.0f, FMath::Clamp(currentYaw + (value), -90.0f, 90.0f), 0.0f));
	}
}

// Verifica se há uma parede à frente para escalar.
bool ATPICharacter::CheckIfCanClimb()
{
	FVector endFront = GetActorLocation() + ((GetCapsuleComponent()->GetScaledCapsuleRadius() + 1) * GetActorForwardVector());
	FCollisionObjectQueryParams ObjectQueryParams;

	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	bool bHit = GetWorld()->LineTraceSingleByObjectType(ClimbingLineTraceData, GetActorLocation(), endFront, ObjectQueryParams);
	if (!bHit) {
	
		bIsClimbing = false;
		return false;
	}
	else {
	
		bIsClimbing = true;
		return true;
	}
}

