// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraXCLGameplayAbility.h"
#include "LyraXCLGameplayAbilityClientToServer.generated.h"

#define UE_API LYRAGAME_API


/**
 * 此类实现了客户端将 TargetData 打包发送给服务端
 * 该技能将在客户端和服务端运行(服务端依赖于客户端的 TargetData)
 * 要使用此类必须实现:
 *	 1. ActivateLocalPlayerAbility (客户端) 创建 TargetData 并调用 NotifyTargetDataReady
 *	 2. ActivateAbilityWithTargetData (客户端 + 服务端) 使用 TargetData 执行技能逻辑
 *
 * 大体的执行流程是, 技能在客户端和服务器端都激活(ULyraXCLGameplayAbility)
 * 在客户端, 通过覆盖 ActivateLocalPlayerAbility 来构建数据并发送 !!!必须调用NotifyTargetDataReady!!!
 * 在服务端, 通过监听 AbilityTargetDataSetDelegate 来接收数据, 委托回调就是 NotifyTargetDataReady (已经在 ActivateServerAbility 实现)
 * 在两端, 数据准备好后都会调用 ActivateAbilityWithTargetData 来处理数据, 以完成数据与逻辑的统一 (需要覆盖实现 ActivateAbilityWithTargetData)
 */
UCLASS(MinimalAPI)
class ULyraXCLGameplayAbilityClientToServer : public ULyraXCLGameplayAbility
{
	GENERATED_BODY()

public:
	ULyraXCLGameplayAbilityClientToServer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ~LyraXCLGameplayAbility interface
	virtual void ActivateServerAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbilityCleanup(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	// ~End of LyraXCLGameplayAbility interface

protected:

	/**
	 * 取得 TargetData 之后, 此方法被调用
	 * 必须覆盖此方法以处理 TargetData
	 * 此方法将在客户端和服务器上调用
	 ***/
	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void ActivateAbilityWithTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);
	virtual void ActivateAbilityWithTargetData_Implementation(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag)
	{ unimplemented(); }


	/**
	 * 当TargetData准备好后调用此函数(由客户端发起)
	 * 在 DedicatedServer, 通过 RPC 来调用
	 *
	**/
	void NotifyTargetDataReady(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

private:
	FDelegateHandle NotifyTargetDataReadyDelegateHandle;
};

#undef UE_API