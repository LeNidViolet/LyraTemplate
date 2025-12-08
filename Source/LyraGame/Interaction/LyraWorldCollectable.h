// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LyraWorldInteractable.h"
#include "GameFramework/Actor.h"
#include "Inventory/IPickupable.h"

#include "LyraWorldCollectable.generated.h"

#define UE_API LYRAGAME_API

class UBoxComponent;
class UProjectileMovementComponent;
class UObject;
struct FInteractionQuery;


UENUM(BlueprintType)
enum class ECollectableState : uint8
{
	// 初始状态 / 可拾取
	ECS_Active,

	// 正在被拾取 (防止其他人同时捡)
	ECS_PickedUp,

	// 冷却中 (刚扔出来，暂时不能捡，防止误触)
	ECS_CoolingDown,

	// 已失效 (已被捡走，等待销毁)
	ECS_Finished
};

USTRUCT()
struct FLyraWorldCollectableStateData
{
	// Collectable 同步数据

	GENERATED_BODY()

	UPROPERTY()
	ECollectableState CollectableState = ECollectableState::ECS_Active;

	UPROPERTY()
	TWeakObjectPtr<APawn> LastInstigatorPawn;
};


// 数据发生变化时广播
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCollectableUpdated);
// 落地时广播(每次)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCollectableLanded);
// 掉落时广播(每次)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollectableDropped, const APawn*, InstigatorPawn);
// 开始拾取时广播(每次)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollectableStarted, const APawn*, InstigatorPawn);
// 销毁前广播
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCollectableFinished);

/**
 * 可拾取物品包装类, 有两种方式使用它
 * 1. 直接在关卡中放置 LyraWorldCollectable 实例, 并在蓝图中设置 StaticInventory 属性来指定物品
 * 2. 通过 SpawnCollectable 静态函数动态生成物品实例
 */
UCLASS(BlueprintType, Blueprintable)
class ALyraWorldCollectable : public ALyraWorldInteractable, public IPickupable
{
	GENERATED_BODY()

public:

	UE_API ALyraWorldCollectable();

	// 设置/获取底层物品数据, 将会触发 OnRep 回调通知客户端
	virtual FInventoryPickup GetPickupInventory() const override;
	void SetPickupInventory(const FInventoryPickup& InInventoryPickup);

	// 被拾取 将更改状态为 PickedUp
	UFUNCTION(BlueprintCallable, Category="Collectable")
	void NotifyPickedUp(UAbilitySystemComponent* ASC, const APawn* InstigatorPawn);

	// 被丢弃 将更改状态为 CoolingDown
	UFUNCTION(BlueprintCallable, Category="Collectable")
	void NotifyDropped(UAbilitySystemComponent* ASC, const APawn* InstigatorPawn);

	// 即将被销毁
	UFUNCTION(BlueprintCallable, Category="Collectable")
	void NotifyFinished(UAbilitySystemComponent* ASC, const APawn* InstigatorPawn);

	UPROPERTY(BlueprintAssignable, Category="Collectable")
	FOnCollectableUpdated OnUpdated;

	UPROPERTY(BlueprintAssignable, Category="Collectable")
	FOnCollectableLanded OnLanded;

	UPROPERTY(BlueprintAssignable, Category="Collectable")
	FOnCollectableDropped OnDropped;

	UPROPERTY(BlueprintAssignable, Category="Collectable")
	FOnCollectableStarted OnPickupStarted;

	UPROPERTY(BlueprintAssignable, Category="Collectable")
	FOnCollectableFinished OnFinished;


	UFUNCTION(BlueprintCallable, Category="Collectable")
	void SetActive(bool bNewActive);


	UFUNCTION(BlueprintCallable, Category="Collectable")
	void ResetStaticMesh();

	// 是否可以被拾取
	UFUNCTION(BlueprintCallable, Category="Collectable")
	virtual bool CanBePickedUp() const;

	/*
	 * 生成拾取物品世界实例
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Collectable", meta=(WorldContext="WorldContextObject"))
	static UE_API ALyraWorldCollectable* SpawnCollectableForDefinition(
		UObject* WorldContextObject,
		TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition,
		int32 SpawnCount,
		const FVector& Location,
		const FRotator& Rotation,
		APawn* InstigatorPawn = nullptr,
		AActor* OwnerActor = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Collectable", meta=(WorldContext="WorldContextObject"))
	static UE_API ALyraWorldCollectable* SpawnCollectableForInstance(
		UObject* WorldContextObject,
		ULyraInventoryItemInstance* ItemInstance,
		int32 SpawnCount,
		const FVector& Location,
		const FRotator& Rotation,
		APawn* InstigatorPawn = nullptr,
		AActor* OwnerActor = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Collectable", meta=(WorldContext="WorldContextObject"))
	static UE_API ALyraWorldCollectable* SpawnCollectable(
		UObject* WorldContextObject,
		FInventoryPickup InventoryPickup,
		const FVector& Location,
		const FRotator& Rotation,
		APawn* InstigatorPawn = nullptr,
		AActor* OwnerActor = nullptr);


protected:

	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintImplementableEvent, Category="Collectable")
	UE_API void K2_OnUpdated();

	UFUNCTION(BlueprintImplementableEvent, Category="Collectable")
	UE_API void K2_OnLanded();

	UFUNCTION(BlueprintImplementableEvent, Category="Collectable")
	UE_API void K2_OnDropped(const APawn* InstigatorPawn);

	UFUNCTION(BlueprintImplementableEvent, Category="Collectable")
	UE_API void K2_OnPickupStarted(const APawn* InstigatorPawn);

	UFUNCTION(BlueprintImplementableEvent, Category="Collectable")
	UE_API void K2_OnCoolingDown();

	UFUNCTION(BlueprintImplementableEvent, Category="Collectable")
	UE_API void K2_OnFinished();

	// 可以在蓝图中设置物品目标
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_StaticInventory, Category="Collectable")
	FInventoryPickup StaticInventory;
	UFUNCTION()
	UE_API void OnRep_StaticInventory();


	// 当前状态
	UPROPERTY(ReplicatedUsing=OnRep_CollectableStateData)
	FLyraWorldCollectableStateData CollectableStateData;
	UFUNCTION()
	UE_API void OnRep_CollectableStateData();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_HasLanded, Category="Collectable")
	bool bHasLanded = false;
	UFUNCTION()
	UE_API void OnRep_HasLanded();



	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileComp = nullptr;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComp = nullptr;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> CollisionComp = nullptr;

	UFUNCTION()
	UE_API void HandleProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
	UE_API void SetProjectileActive(bool bNewActive);


	// Drop 冷却等待时长 (主要是为了等待拾取 timeline 播放完毕)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Collectable")
	float DropCooldownDuration = 0.7f;

	// 销毁等待时长
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Collectable")
	float LifeAfterFinished = 2.0f;

	// 拾取时播放的GameplayCue标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Collectable", meta=(Categories="GameplayCue"))
	FGameplayTag GameplayCueTagCollect;

	// 丢弃时播放的GameplayCue标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Collectable", meta=(Categories="GameplayCue"))
	FGameplayTag GameplayCueTagDrop;

private:
	void OnCoolingDownTimeout(UAbilitySystemComponent* ASC, const APawn* InstigatorPawn);

	void FitCollisionToMesh();
};

#undef UE_API