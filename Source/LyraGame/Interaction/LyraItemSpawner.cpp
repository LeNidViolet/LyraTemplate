// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraItemSpawner.h"
#include "Engine/World.h"
#include "LyraWorldCollectable.h"
#include "TimerManager.h"
#include "Inventory/LyraInventoryItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraItemSpawner)

ALyraItemSpawner::ALyraItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // Spawner should be replicated to ensure consistent state
	SetReplicateMovement(true);
	SetCanBeDamaged(false);
}

void ALyraItemSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnItem();
	}
}

void ALyraItemSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (RespawnTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
	}

	if (SpawnedActor)
	{
		SpawnedActor->OnDestroyed.RemoveDynamic(this, &ThisClass::OnItemDestroyed);
		SpawnedActor->Destroy();
		SpawnedActor = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void ALyraItemSpawner::SpawnItem()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ItemDefinition)
	{
		return;
	}

	if (SpawnedActor)
	{
		return;
	}

	// 计算生成位置
	FVector ForwardDir = GetActorForwardVector();
	FVector SpawnLocation = GetActorLocation() + ForwardDir * SpawnForwardOffset;

	FRotator SpawnRotation = SpawnRotationOffset;
	// 随机旋转，让掉落物看起来自然一点
	if (bRandomizeRotation)
	{
		SpawnRotation += FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f);
	}

	SpawnedActor = ALyraWorldCollectable::SpawnCollectableForDefinition(
		this,
		ItemDefinition,
		SpawnCount,
		SpawnLocation,
		SpawnRotation,
		nullptr, // Instigator
		nullptr // OwnerActor
	);

	if (SpawnedActor)
	{
		SpawnedActor->OnDestroyed.AddUniqueDynamic(this, &ThisClass::OnItemDestroyed);
		CurrentSpawnCount++;
	}
}

void ALyraItemSpawner::OnItemDestroyed(AActor* DestroyedActor)
{
	if (SpawnedActor == DestroyedActor)
	{
		SpawnedActor = nullptr;

		// 是否持续生成 ?
		if (!bContinuousRespawn)
		{
			return;
		}

		// 检查最大生成数量限制
		if (MaxSpawnCount > 0 && CurrentSpawnCount >= MaxSpawnCount)
		{
			return;
		}

		// Start respawn timer
		if (RespawnInterval > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(
				RespawnTimerHandle,
				this,
				&ThisClass::SpawnItem,
				RespawnInterval,
				false);
		}
		else
		{
			// Immediate respawn (next frame to avoid recursion issues if any)
			GetWorld()->GetTimerManager().SetTimerForNextTick(
				this,
				&ThisClass::SpawnItem);
		}
	}
}
