// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraXCLGameplayAbilityClientToServer.h"
#include "Interaction/LyraWorldMarker.h"
#include "LyraGameplayAbility_Marker.generated.h"

#define UE_API LYRAGAME_API


UCLASS(MinimalAPI)
class ULyraGameplayAbility_Marker : public ULyraXCLGameplayAbilityClientToServer
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Marker")
	UE_API bool MakeTargetData(const FGameplayTag& ApplicationTag);

protected:
	// 以本地玩家身份激活能力
	UE_API virtual void ActivateLocalPlayerAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 客户端服务端都会调用到这里
	UE_API virtual void ActivateAbilityWithTargetData_Implementation(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag) override;

	UE_API virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

private:
	// 以下配置参数从GameData中获取

	// 射线检测通道
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldStatic;

	// 射线检测距离
	float TraceDistance = 50000.f;

	// 将要创建的Marker类类型
	TSubclassOf<ALyraWorldMarker> MarkerClass = ALyraWorldMarker::StaticClass();

	// 目标标记类型
	ELyraWorldMarkerType MarkerType = ELyraWorldMarkerType::Waypoint;

	// 额外传送的标签 暂时未用
	FGameplayTag ApplicationTag;
};


#undef UE_API