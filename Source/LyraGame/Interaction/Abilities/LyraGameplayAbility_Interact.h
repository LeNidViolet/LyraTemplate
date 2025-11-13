// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Interaction/InteractionOption.h"

#include "LyraGameplayAbility_Interact.generated.h"

class UIndicatorDescriptor;
class UObject;
class UUserWidget;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;

/**
 * ULyraGameplayAbility_Interact
 *
 * Gameplay ability used for character interacting
 * 主要用来扫描附近的可交互对象，并将它们身上的能力赋予角色, 然后与其他组件交互, 可视化这些可交互对象
 */
UCLASS(Abstract)
class ULyraGameplayAbility_Interact : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:

	ULyraGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;


	UFUNCTION(BlueprintCallable)
	void UpdateInteractions(const TArray<FInteractionOption>& InteractiveOptions);

	UFUNCTION(BlueprintCallable)
	void TriggerInteraction();

protected:
	UPROPERTY(BlueprintReadWrite)
	TArray<FInteractionOption> CurrentOptions;

	UPROPERTY()
	TArray<TObjectPtr<UIndicatorDescriptor>> Indicators;

	// 以下配置从GameData中读取

	// 暂时没有使用它
	FGameplayTag TriggerTag;

	// CollisionProfile Name
	FName InteractTraceProfileName;

	// 扫描交互对象的频率和范围
	float InteractScanRate = 0.f;
	float InteractScanRange = 0.f;

	// 要使用的 Indicator Descriptor 类
	TSubclassOf<UIndicatorDescriptor> IndicatorClass;

private:

	void CreateWaitInputPressTask();

	UFUNCTION()
	void OnInputPressedCallback(float TimeWaited);
};
