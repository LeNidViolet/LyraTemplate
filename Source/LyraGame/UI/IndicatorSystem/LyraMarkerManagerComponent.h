// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IndicatorDescriptor.h"
#include "Components/ControllerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraMarkerManagerComponent.generated.h"

#define UE_API LYRAGAME_API

struct FOnAddMarkerParameters;
struct FOnRemoveMarkerParameters;
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
class ULyraMarkerManagerComponent : public UControllerComponent
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
	TArray<FLyraMarkerInstance*> GetMarkerInstancesForOwnerPlayer(APlayerState* PlayerState,bool bWithInvisible=false) const;


	// 是否正瞄准某个标记点(属于本地玩家)
	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	bool IsAimingAtMarker() const;

	// 获取正瞄准的标记点实例(属于本地玩家)
	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	ALyraWorldMarker* GetAimingMarkerActor() const;

protected:
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void RegisterMessageHandlers();
	void UnregisterMessageHandlers();

	FGameplayMessageListenerHandle MarkerAddEventListener;
	void HandleAddMarkerEvent(FGameplayTag Channel, const FOnAddMarkerParameters& Parameters);
	FGameplayMessageListenerHandle MarkerRemoveEventListener;
	void HandleRemoveMarkerEvent(FGameplayTag Channel, const FOnRemoveMarkerParameters& Parameters);

	// 缓存标记点实例列表
	TArray<TSharedPtr<FLyraMarkerInstance>> MarkerList;

	FTimerHandle TimerHandle;
	void ToggleMarkerPromptVisbility();
	void ShowOrHideMarkerPrompt(UIndicatorDescriptor* IndicatorDescriptor, bool bShow);

	// 标记点交互提示扫描频率
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="WorldMarker", meta=(AllowPrivateAccess="true"))
	float ScanRate = 0.2;

	// 标记点交互提示扫描半径 (像素)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="WorldMarker", meta=(AllowPrivateAccess="true"))
	float ScanRadius = 40;

	TWeakObjectPtr<UIndicatorDescriptor> LastPromptIndicatorDescriptor = nullptr;
	TWeakObjectPtr<ALyraWorldMarker> LastPromptMarkerActor = nullptr;
	bool bLastPromptVisible = false;
};

#undef UE_API