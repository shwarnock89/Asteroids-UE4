// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NativeGameplayTags.h"

#include "AsteroidsHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerHealthUpdated, const float, PlayerHealthPercentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerShieldUpdated, const float, PlayerCurrentShields);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(FireComponentTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SmokeComponentTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ExplosionComponentTag);

UCLASS()
class UAsteroidsHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UFUNCTION(Server, Reliable)
	void Server_HandleHealthPackPickedUp(const float HealthIncreaseAmount);

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnPlayerDied OnPlayerDied;

private:

	UAsteroidsHealthComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnPlayerHealthUpdated OnPlayerHealthUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnPlayerShieldUpdated OnPlayerShieldUpdated;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float PlayerMaxHealth = 100.0f;

	UPROPERTY(Replicated)
	bool bIsDead = false;

	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float PlayerCurrentHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Shields)
	float PlayerCurrentShields = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float PlayerMaxShields = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float ShieldRegenDelay = 6.0f;

	UPROPERTY(Replicated)
	float ShieldRegenTimer = 0.0f;

	UPROPERTY(Replicated)
	bool bShieldTimerActive = false;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float ShieldRegenRate = 10.0f;

	UPROPERTY(Replicated)
	bool bDamageTimerActive = false;

	UPROPERTY(Replicated)
	bool bIsProcessingHit = false;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float DamageTimeDelay = 1.0f;

	UPROPERTY(Replicated)
	float CurrentDamageTimeDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
	TObjectPtr<UParticleSystem> FireSystem = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
	TObjectPtr<UParticleSystem> SmokeSystem = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
	TObjectPtr<UParticleSystem> ExplosionSystem = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TWeakObjectPtr<UParticleSystemComponent> FireComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TWeakObjectPtr<UParticleSystemComponent> SmokeComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TWeakObjectPtr<UParticleSystemComponent> ExplosionComponent = nullptr;

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_Shields();

	UFUNCTION(Server, Reliable)
	void Server_DealDamage(float Damage);

	UFUNCTION(Server, Reliable)
	void Server_RegenerateShields(const float DeltaSeconds);

	UFUNCTION()
	void HandleDamage(AActor* DamagedActor, const float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	void HandleShieldDamage(const float DamageAmount);
	void HandleHealthDamage(const float DamageAmount);
	void HandleDeath();
};
