// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ControllerComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "LyraIndicatorManagerComponent.generated.h"

#define UE_API LYRAGAME_API

class AController;
class UIndicatorDescriptor;
class UObject;
struct FFrame;

/**
 * @class ULyraIndicatorManagerComponent
 */
UCLASS(MinimalAPI, BlueprintType, Blueprintable)
class ULyraIndicatorManagerComponent : public UControllerComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UE_API ULyraIndicatorManagerComponent(const FObjectInitializer& ObjectInitializer);

	static UE_API ULyraIndicatorManagerComponent* GetComponent(AController* Controller);

	UFUNCTION(BlueprintCallable, Category = Indicator)
	UE_API void AddIndicator(UIndicatorDescriptor* IndicatorDescriptor);
	
	UFUNCTION(BlueprintCallable, Category = Indicator)
	UE_API void RemoveIndicator(UIndicatorDescriptor* IndicatorDescriptor);

	DECLARE_EVENT_OneParam(ULyraIndicatorManagerComponent, FIndicatorEvent, UIndicatorDescriptor* Descriptor)
	FIndicatorEvent OnIndicatorAdded;
	FIndicatorEvent OnIndicatorRemoved;

	const TArray<UIndicatorDescriptor*>& GetIndicators() const { return Indicators; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRegister() override;


public:
	//~ Begin IGameFrameworkInitStateInterface interface
	// 定义 Feature Name (用于其他组件查找)
	static const FName NAME_ActorFeatureName;
	UE_API virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }

	// 核心：状态检查与转换
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

private:
	UPROPERTY()
	TArray<TObjectPtr<UIndicatorDescriptor>> Indicators;
};

#undef UE_API
