// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraXCLGameplayAbilityClientToServer.h"
#include "LyraGameplayAbility_Inventory_Swap.generated.h"

#define UE_API LYRAGAME_API

/**
 *
 */
UCLASS(MinimalAPI)
class ULyraGameplayAbility_Inventory_Swap : public ULyraXCLGameplayAbilityClientToServer
{
	GENERATED_BODY()

public:
	UE_API ULyraGameplayAbility_Inventory_Swap(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// 以本地玩家身份激活能力
	UE_API virtual void ActivateLocalPlayerAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 客户端服务端都会调用到这里
	UE_API virtual void ActivateAbilityWithTargetData_Implementation(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag) override;

	UE_API virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

private:
	bool MakeTargetData(const FGameplayEventData* TriggerEventData);
};

#undef UE_API