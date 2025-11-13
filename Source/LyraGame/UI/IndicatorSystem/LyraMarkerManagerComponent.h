// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IndicatorDescriptor.h"
#include "Components/ControllerComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraMarkerManagerComponent.generated.h"

#define UE_API LYRAGAME_API

struct FOnMarkerAddParameters;
struct FOnMarkerRemoveParameters;
class ALyraWorldMarker;


USTRUCT()
struct FLyraMarkerInstance
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<ALyraWorldMarker> MarkerActor;

	UPROPERTY()
	TWeakObjectPtr<UIndicatorDescriptor> IndicatorDescriptor;
};



UCLASS(MinimalAPI)
class ULyraMarkerManagerComponent : public UControllerComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UE_API ULyraMarkerManagerComponent(const FObjectInitializer& ObjectInitializer);

	static UE_API ULyraMarkerManagerComponent* GetComponent(AController* Controller);

	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	UE_API TArray<ALyraWorldMarker*> GetMarkerActorsForOwnerPlayer(APlayerState* PlayerState, bool bWithInvisible=false) const;

	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	UE_API ALyraWorldMarker* GetMarkerActorForIndicatorDescriptor(UIndicatorDescriptor* IndicatorDescriptor) const;

	// 返回到目标位置的距离, 单位厘米
	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	UE_API int32 GetDistanceToLocation(UIndicatorDescriptor* IndicatorDescriptor, const FVector& TargetLocation) const;

	// 获取指定玩家拥有的所有标记点实例
	UE_API TArray<FLyraMarkerInstance*> GetMarkerInstancesForOwnerPlayer(APlayerState* PlayerState,bool bWithInvisible=false) const;


	// 是否正瞄准某个标记点(属于本地玩家)
	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	UE_API bool IsAimingAtMarker() const;

	// 获取正瞄准的标记点实例(属于本地玩家)
	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	UE_API ALyraWorldMarker* GetAimingMarkerActor() const;

protected:
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void OnRegister() override;


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
	void RegisterMessageHandlers();
	void UnregisterMessageHandlers();

	FGameplayMessageListenerHandle MarkerAddEventListener;
	void HandleMarkerAddEvent(FGameplayTag Channel, const FOnMarkerAddParameters& Parameters);
	FGameplayMessageListenerHandle MarkerRemoveEventListener;
	void HandleMarkerRemoveEvent(FGameplayTag Channel, const FOnMarkerRemoveParameters& Parameters);

	// 广播发现标记点消息
	void BroadcastDiscoverMessage();

	// 缓存标记点实例列表
	TArray<TSharedPtr<FLyraMarkerInstance>> MarkerList;

	// 检查 MarkerActor 是否已经存在
	bool IsMarkerActorExist(ALyraWorldMarker* MarkerActor) const;

	FTimerHandle TimerHandle;
	void ToggleMarkerPromptVisbility();
	void ShowOrHideCalloutPrompt(ALyraWorldMarker* MarkerActor, bool bShow);

	float ScanRate = 0.f;
	float ScanRadius = 0.f;

	TWeakObjectPtr<UIndicatorDescriptor> LastPromptIndicatorDescriptor = nullptr;
	TWeakObjectPtr<ALyraWorldMarker> LastPromptMarkerActor = nullptr;
	bool bLastPromptVisible = false;
};

#undef UE_API