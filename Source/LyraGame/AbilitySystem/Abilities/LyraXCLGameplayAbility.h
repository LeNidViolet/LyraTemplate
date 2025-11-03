// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraGameplayAbility.h"
#include "LyraXCLGameplayAbility.generated.h"

#define UE_API LYRAGAME_API

/**
Network	Mode	Variant				Authority?		Note						Method Invoked
Standalone		local player		Yes				client is local player		ActivateLocalPlayerAbility
Client			local player		NO				remote client				ActivateLocalPlayerAbility
Listen			Server local player	Yes				client is local player		ActivateLocalPlayerAbility
Listen			Server server		Yes				server for remote client	ActivateServerAbility
Dedicated		Server server		Yes				server for remote client	ActivateServerAbility

https://x157.github.io/UE5/GameplayAbilitySystem/
*/



UCLASS(MinimalAPI)
class  ULyraXCLGameplayAbility : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
protected:

	/*
	 * 以本地玩家身份激活能力
	 * 被 ActivateAbility 调用
	 * 默认实现为空, 需要子类重载(C++或者蓝图)
	*/
	virtual void ActivateLocalPlayerAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
	{ K2_ActivateLocalPlayerAbility(); }

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Ability, DisplayName = "Activate Local Player Ability")
	void K2_ActivateLocalPlayerAbility();

	/*
	 * 以服务器身份激活能力
	 * 被 ActivateAbility 调用
	 * 默认实现为空, 需要子类重载(C++或者蓝图)
	*/
	virtual void ActivateServerAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
	{ K2_ActivateServerAbility(); }

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Ability, DisplayName = "Activate Server Ability")
	void K2_ActivateServerAbility();


	virtual void EndAbilityCleanup(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled);


	// ~ULyraGameplayAbility Interface
	virtual void NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const override;
	// ~End of ULyraGameplayAbility Interface

	// ~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	// ~End of UGameplayAbility interface

private:
};

#undef UE_API