// Fill out your copyright notice in the Description page of Project Settings.

#include "AsteroidsHealthComponent.h"
#include "Asteroid.h"
#include "AsteroidsScoreManager.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "WorldBoundsVolume.h"

UAsteroidsHealthComponent::UAsteroidsHealthComponent(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UAsteroidsHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAsteroidsHealthComponent, bDamageTimerActive);
	DOREPLIFETIME(UAsteroidsHealthComponent, CurrentDamageTimeDelay);
	DOREPLIFETIME(UAsteroidsHealthComponent, bIsDead);
	DOREPLIFETIME(UAsteroidsHealthComponent, PlayerCurrentHealth);
	DOREPLIFETIME(UAsteroidsHealthComponent, PlayerCurrentShields);
	DOREPLIFETIME(UAsteroidsHealthComponent, ShieldRegenTimer);
	DOREPLIFETIME(UAsteroidsHealthComponent, bShieldTimerActive);
}

void UAsteroidsHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ensureAlways(IsValid(GetOwner()) && IsValid(GetOwner()->GetClass()) && GetOwner()->GetClass()->ImplementsInterface(UWorldBoundsHandlingInterface::StaticClass())))
	{
		return;
	}

	UCapsuleComponent* OwnerCapsule = IWorldBoundsHandlingInterface::Execute_GetCapsuleComponent(GetOwner());
	if (!ensureAlways(IsValid(OwnerCapsule)))
	{
		return;
	}

	OwnerCapsule->OnComponentHit.AddDynamic(this, &UAsteroidsHealthComponent::OnHit);
}

void UAsteroidsHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!ensureAlways(IsValid(GetOwner()) && IsValid(GetOwner()->GetClass()) && GetOwner()->GetClass()->ImplementsInterface(UWorldBoundsHandlingInterface::StaticClass())))
	{
		return;
	}

	UCapsuleComponent* OwnerCapsule = IWorldBoundsHandlingInterface::Execute_GetCapsuleComponent(GetOwner());
	if (!ensureAlways(IsValid(OwnerCapsule)))
	{
		return;
	}

	OwnerCapsule->OnComponentHit.RemoveDynamic(this, &UAsteroidsHealthComponent::OnHit);
}

void UAsteroidsHealthComponent::HandleShieldDamage(const float DamageAmount)
{
	if (!ensureAlways(IsValid(GetOwner()) && GetOwner()->HasAuthority()))
	{
		return;
	}

	if (PlayerCurrentShields > 0.0f)
	{
		ShieldRegenTimer = 0.0f;
		bShieldTimerActive = true;
		PlayerCurrentShields = FMath::Clamp(PlayerCurrentShields - DamageAmount, 0.0f, PlayerMaxShields);

		// Manually call on server so the Listen Server player's UI updates instantly
		OnRep_Shields();
	}
}

void UAsteroidsHealthComponent::HandleDeath()
{
	if (!ensureAlways(IsValid(GetOwner()) && GetOwner()->HasAuthority()))
	{
		return;
	}

	if (!ensureAlways(IsValid(GetWorld())))
	{
		return;
	}

	const UAsteroidsScoreManager* ScoreManager = UAsteroidsScoreManager::GetScoreManager(*GetWorld());
	if (!ensureAlways(IsValid(ScoreManager)))
	{
		return;
	}

	if (!bIsDead)
	{
		OnPlayerDied.Broadcast();
	}

	bIsDead = true;
}

void UAsteroidsHealthComponent::HandleHealthDamage(const float DamageAmount)
{
	if (!ensureAlways(IsValid(GetOwner()) && GetOwner()->HasAuthority()))
	{
		return;
	}

	PlayerCurrentHealth = FMath::Clamp(PlayerCurrentHealth - DamageAmount, 0.0f, PlayerMaxHealth);
	if (PlayerCurrentHealth <= 0.0f)
	{
		HandleDeath();
	}
	else
	{
		ShieldRegenTimer = 0.0f;
		bDamageTimerActive = true;

		// Manually call on server for Listen Server UI updates
		OnRep_Health();
	}
}

void UAsteroidsHealthComponent::Server_DealDamage_Implementation(const float Damage)
{
	if (!ensureAlways(IsValid(GetOwner()) && GetOwner()->HasAuthority()))
	{
		return;
	}

	if (FMath::IsNearlyZero(PlayerCurrentShields))
	{
		if (!bDamageTimerActive)
		{
			HandleHealthDamage(Damage);
		}
	}
	else
	{
		HandleShieldDamage(Damage);
	}
}

void UAsteroidsHealthComponent::TickComponent(const float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (!ensureAlways(IsValid(GetOwner())))
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (bDamageTimerActive)
	{
		CurrentDamageTimeDelay += DeltaTime;
	}

	if (CurrentDamageTimeDelay >= DamageTimeDelay)
	{
		bDamageTimerActive = false;
		CurrentDamageTimeDelay = 0;
	}

	if (bShieldTimerActive)
	{
		ShieldRegenTimer += DeltaTime;
	}
	else
	{
		Server_RegenerateShields(DeltaTime);
	}

	if (ShieldRegenTimer >= ShieldRegenDelay)
	{
		ShieldRegenTimer = 0.0f;
		bShieldTimerActive = false;
	}
}

void UAsteroidsHealthComponent::Server_RegenerateShields_Implementation(const float DeltaSeconds)
{
	if (PlayerCurrentShields < PlayerMaxShields)
	{
		PlayerCurrentShields = FMath::Clamp(PlayerCurrentShields + ShieldRegenRate * DeltaSeconds, 0.0f, PlayerMaxShields);
		// Refresh Listen Server UI
		OnRep_Shields();
	}
}

void UAsteroidsHealthComponent::OnHit(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, FVector, const FHitResult&)
{
	if (!ensureAlways(IsValid(GetOwner())))
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (bIsProcessingHit)
	{
		return;
	}

	bIsProcessingHit = true;

	if (!ensureAlways(OtherActor))
	{
		return;
	}

	if (OtherActor->IsA<AAsteroid>())
	{
		Server_DealDamage(10);
	}

	bIsProcessingHit = false;
}

void UAsteroidsHealthComponent::HandleHealthPackPickedUp(const float HealthIncreaseAmount)
{
	if (!ensureAlways(IsValid(GetOwner()) && GetOwner()->HasAuthority()))
	{
		return;
	}

	PlayerCurrentHealth = FMath::Clamp(PlayerCurrentHealth + HealthIncreaseAmount, 0.0f, PlayerMaxHealth);

	// Refresh local server display state
	OnRep_Health();
}

void UAsteroidsHealthComponent::OnRep_Health() const
{
	if (PlayerMaxHealth > 0.0f)
	{
		const float HealthPercentage = PlayerCurrentHealth / PlayerMaxHealth;
		OnPlayerHealthUpdated.Broadcast(HealthPercentage);
	}
}

void UAsteroidsHealthComponent::OnRep_Shields() const
{
	if (PlayerMaxShields > 0.0f)
	{
		const float CurrentShieldPercentage = PlayerCurrentShields / PlayerMaxShields;
		OnPlayerShieldUpdated.Broadcast(CurrentShieldPercentage);
	}
}