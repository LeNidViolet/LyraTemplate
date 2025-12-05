// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LyraItemSpawner.generated.h"

class ULyraInventoryItemDefinition;
class ALyraWorldCollectable;
class UObject;

#define UE_API LYRAGAME_API

/**
 * 物品生成器
 */

UCLASS(Blueprintable)
class ALyraItemSpawner : public AActor
{
	GENERATED_BODY()

public:
	ALyraItemSpawner();

protected:
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 指定要生成的物品类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition;

	// 每次生成几个(会打包到一起)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 SpawnCount = 1;

	// 是否不断生成物品
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	bool bContinuousRespawn = true;

	// 时间间隔，在物品被拾取后重新生成, 0 表示立即生成
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (EditCondition = "bContinuousRespawn"))
	float RespawnInterval = 2.0f;

	// 最多生成数量, 0 表示无限制
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (EditCondition = "bContinuousRespawn"))
	int32 MaxSpawnCount = 0;

	// 生成时, 向前的偏移位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float SpawnForwardOffset = 80.f;

	// 生成时, 施加的旋转偏移
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	FRotator SpawnRotationOffset = FRotator::ZeroRotator;

	// 生成时, 是否随机旋转 (Z 轴)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	bool bRandomizeRotation = true;

	// Spawns the item if it doesn't exist.
	void SpawnItem();

	// Called when the spawned item is destroyed (picked up).
	UFUNCTION()
	void OnItemDestroyed(AActor* DestroyedActor);

private:
	// Pointer to the currently spawned item.
	UPROPERTY()
	TObjectPtr<AActor> SpawnedActor;

	// Timer handle for respawning.
	FTimerHandle RespawnTimerHandle;

	// Current number of spawned items.
	int32 CurrentSpawnCount = 0;
};

#undef UE_API