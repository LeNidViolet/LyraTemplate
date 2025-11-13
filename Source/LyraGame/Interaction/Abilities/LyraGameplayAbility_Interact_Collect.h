// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "LyraGameplayAbility_Interact_Collect.generated.h"

#define UE_API LYRAGAME_API

/**
 *
 */
UCLASS(MinimalAPI)
class ULyraGameplayAbility_Interact_Collect : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UE_API ULyraGameplayAbility_Interact_Collect(const FObjectInitializer& ObjectInitializer);

protected:
	UE_API virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};

#undef UE_API
