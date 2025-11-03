// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraXCLGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "LyraLogChannels.h"


void ULyraXCLGameplayAbility::EndAbilityCleanup(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
}

void ULyraXCLGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{

	// 如果在蓝图中实现了激活函数, 则调用蓝图实现的激活函数, 这会取代下面的实现逻辑
	if (bHasBlueprintActivateFromEvent && TriggerEventData)
	{
		K2_ActivateAbilityFromEvent(*TriggerEventData);
		return;
	}
	if (bHasBlueprintActivate)
	{
		K2_ActivateAbility();
		return;
	}

	// 激活本地玩家版本的能力 (or server with a player on it???)
	if (ActorInfo->IsLocallyControlled())
	{
		ActivateLocalPlayerAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	}
	// 激活服务器版本的能力
	else if (ActorInfo->IsNetAuthority())
	{
		ActivateServerAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	}
	else
	{
		// 不应该发生的情况, 结束能力
		UE_LOG(LogLyraAbilitySystem, Warning, TEXT("ActivateAbility: Unexpected network role for ability [%s]. Ending ability."), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
	}

	// 这里不激活父类的实现
}










bool ULyraXCLGameplayAbility::CommitAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	FGameplayTagContainer* OptionalRelevantTags)
{
	return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
}

void ULyraXCLGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	EndAbilityCleanup(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility,  bWasCancelled);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// if (IsEndAbilityValid(Handle, ActorInfo))
	// {
	// 	if (ScopeLockCount == 0)
	// 	{
	// 		EndAbilityCleanup(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility,  bWasCancelled);
	// 		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	// 	}
	// 	else
	// 	{
	//
	// 	}
	// }
}

void ULyraXCLGameplayAbility::OnGiveAbility(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
}

void ULyraXCLGameplayAbility::OnRemoveAbility(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);
}

void ULyraXCLGameplayAbility::NativeOnAbilityFailedToActivate(
	const FGameplayTagContainer& FailedReason) const
{
	Super::NativeOnAbilityFailedToActivate(FailedReason);
}
