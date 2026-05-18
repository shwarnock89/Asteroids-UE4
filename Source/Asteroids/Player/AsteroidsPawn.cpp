
#include "AsteroidsPawn.h"

#include "AsteroidsHealthComponent.h"
#include "AsteroidsMovementComponent.h"
#include "AsteroidsProjectile.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UE_DEFINE_GAMEPLAY_TAG(ShipComponentTag, "Component.Ship");

AAsteroidsPawn::AAsteroidsPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	NetDormancy = DORM_Never;
	SetNetUpdateFrequency(60.0f);
	SetMinNetUpdateFrequency(30.0f);

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
	}

	RootComponent = CapsuleComponent;
	MovementComponent = CreateDefaultSubobject<UAsteroidsMovementComponent>(TEXT("AsteroidsMovementComponent"));
	HealthComponent = CreateDefaultSubobject<UAsteroidsHealthComponent>(TEXT("AsteroidsHealthComponent"));
}

void AAsteroidsPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent)

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!ensureAlways(IsValid(PlayerController)))
	{
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!ensureAlways(IsValid(Subsystem)))
	{
		return;
	}

	Subsystem->AddMappingContext(DefaultMappingContext, 0);

	// Cast to EnhancedInputComponent
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (!ensureAlways(IsValid(EnhancedInputComponent)))
	{
		return;
	}

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAsteroidsPawn::HandleMovement);
	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AAsteroidsPawn::FireShot);
}

void AAsteroidsPawn::HandleMovement(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();

	MovementComponent->SetInputVector(Input);

	Server_SetInput(Input);
}

void AAsteroidsPawn::Server_SetInput_Implementation(const FVector2D& Input)
{
	MovementComponent->SetInputVector(Input);
}

void AAsteroidsPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ShipMeshComponent = FindComponentByTag<UStaticMeshComponent>(ShipComponentTag.GetTag().GetTagName());
	CapsuleComponent = FindComponentByClass<UCapsuleComponent>();
}

void AAsteroidsPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		static const TArray Palette =
		{
			FLinearColor::Red,
			FLinearColor::Green,
			FLinearColor::Blue,
			FLinearColor::Yellow,
		};

		static int CurrentIndex = 0;
		PlayerColor = Palette[CurrentIndex % Palette.Num()];
		++CurrentIndex;
		SetPlayerColor();
	}
}

void AAsteroidsPawn::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetReplicateMovement(false);
	}

	FVector WorldLocation = GetActorLocation();
	WorldLocation.Z = 0.0f;
	SetActorLocation(WorldLocation);

	if (!ensureAlways(IsValid(HealthComponent)))
	{
		HealthComponent->OnPlayerDied.AddDynamic(this, &AAsteroidsPawn::HandlePlayerDeath);
	}
}

void AAsteroidsPawn::HandleBulletDestroyed(AActor*)
{
	--CurrentBullets;
}

void AAsteroidsPawn::DestroyPawn()
{
	Destroy();
}

void AAsteroidsPawn::HandlePlayerDeath()
{
	if (!ensureAlways(IsValid(GetWorld())))
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!ensureAlways(IsValid(PlayerController)))
	{
		return;
	}

	DisableInput(PlayerController);

	if (!ensureAlways(IsValid(ShipMeshComponent)))
	{
		return;
	}

	ShipMeshComponent->SetHiddenInGame(true);

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, FTimerDelegate::CreateUObject(this, &AAsteroidsPawn::DestroyPawn), 1.0f, false);
}

void AAsteroidsPawn::FireShot()
{
	Server_Fire(FInputActionValue());
}

void AAsteroidsPawn::Server_Fire_Implementation(const FInputActionValue&)
{
	if (!bCanFire)
	{
		return;
	}

	// If it's ok to fire again
	if (CurrentBullets < 2)
	{
		const FVector FireDirection = GetActorForwardVector();
		const FRotator FireRotation = FireDirection.Rotation();
		// Spawn projectile at an offset from this pawn
		const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

		UWorld* const World = GetWorld();
		if (!ensureAlways(IsValid(World)))
		{
			return;
		}

		// spawn the projectile
		if (!ensureAlways(IsValid(ProjectileClass)))
		{
			return;
		}

		AActor* SpawnedActor = World->SpawnActor(ProjectileClass, &SpawnLocation, &FireRotation);
		if (!ensureAlways(IsValid(SpawnedActor)))
		{
			return;
		}

		SpawnedActor->OnDestroyed.AddDynamic(this, &AAsteroidsPawn::HandleBulletDestroyed);

		World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AAsteroidsPawn::Server_ShotTimerExpired, FireRate);

		// try and play the sound if specified
		if (FireSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		bCanFire = false;
		CurrentBullets++;
	}
}

void AAsteroidsPawn::Server_ShotTimerExpired_Implementation()
{
	bCanFire = true;
}


void AAsteroidsPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAsteroidsPawn, CurrentBullets);
	DOREPLIFETIME(AAsteroidsPawn, bCanFire);
	DOREPLIFETIME(AAsteroidsPawn, TimerHandle_ShotTimerExpired);
	DOREPLIFETIME(AAsteroidsPawn, PlayerColor);
}

void AAsteroidsPawn::OnRep_PlayerColor() const
{
	SetPlayerColor();
}

void AAsteroidsPawn::SetPlayerColor() const
{
	if (!ensureAlways(IsValid(ShipMeshComponent)))
	{
		return;
	}

	UMaterialInstanceDynamic* MaterialInstanceDynamic = ShipMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (!ensureAlways(IsValid(MaterialInstanceDynamic)))
	{
		return;
	}

	MaterialInstanceDynamic->SetVectorParameterValue("DiffuseColor", PlayerColor);
}
