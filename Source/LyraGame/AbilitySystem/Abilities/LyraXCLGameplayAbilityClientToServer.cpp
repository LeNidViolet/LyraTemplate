// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraXCLGameplayAbilityClientToServer.h"
#include "AbilitySystemComponent.h"

ULyraXCLGameplayAbilityClientToServer::ULyraXCLGameplayAbilityClientToServer(
	const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void ULyraXCLGameplayAbilityClientToServer::ActivateServerAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(ASC);

	// 注册监听 TargetData 准备好的委托
	NotifyTargetDataReadyDelegateHandle = ASC->AbilityTargetDataSetDelegate(
		Handle, ActivationInfo.GetActivationPredictionKey()).AddUObject(
		this,
		&ThisClass::NotifyTargetDataReady);
}

void ULyraXCLGameplayAbilityClientToServer::EndAbilityCleanup(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(ASC);

	// 注销监听 TargetData 准备好的委托
	ASC->AbilityTargetDataSetDelegate(Handle, ActivationInfo.GetActivationPredictionKey()).Remove(NotifyTargetDataReadyDelegateHandle);
	ASC->ConsumeClientReplicatedTargetData(Handle, ActivationInfo.GetActivationPredictionKey());

	Super::EndAbilityCleanup(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}



void ULyraXCLGameplayAbilityClientToServer::NotifyTargetDataReady(
	const FGameplayAbilityTargetDataHandle& InData,
	FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(ASC);

	// [xist] is this (from Lyra) like an "if is handle valid?" check? seems so, keeping it as such.
	if (!ASC->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false);  // do not replicate
		return;
	}

	// if commit fails, cancel ability
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);  // replicate cancellation
		return;
	}

	// true if we need to replicate this target data to the server
	const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();

	// Start a scoped prediction window
	FScopedPredictionWindow	ScopedPrediction(ASC);

	// Lyra does this memcopy operation; const cast paranoia is real. We'll keep it.
	// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
	const FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

	// if this isn't the local player on the server, then notify the server
	if (bShouldNotifyServer)
		ASC->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, ASC->ScopedPredictionKey);

	// Execute the ability we've now successfully committed
	ActivateAbilityWithTargetData(LocalTargetDataHandle, ApplicationTag);

	// We've processed the data, clear it from the RPC buffer
	ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}
