// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraGameplayAbility_Inventory_Drop.h"

#include "LyraLogChannels.h"
#include "Interaction/LyraWorldCollectable.h"
#include "Inventory/LyraInventoryItemInstance.h"
#include "Inventory/LyraInventoryManagerComponent.h"
#include "TargetData/LyraGameplayAbilityTargetData_Inventory.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameplayAbility_Inventory_Drop)


ULyraGameplayAbility_Inventory_Drop::ULyraGameplayAbility_Inventory_Drop(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 禁止客户端结束服务端能力
	bServerRespectsRemoteAbilityCancellation = false;
}


void ULyraGameplayAbility_Inventory_Drop::ActivateLocalPlayerAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateLocalPlayerAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 本地要做的事情就是准备好数据并发送给服务器
	bool bOk = MakeTargetData(TriggerEventData);
	if (!bOk)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

bool ULyraGameplayAbility_Inventory_Drop::MakeTargetData(const FGameplayEventData* TriggerEventData)
{
	// 要丢弃的 ItemInstance / 数量 都在 TargetData 里
	if (!TriggerEventData->TargetData.IsValid(0)) return false;

	// 提取数据
	const FGameplayAbilityTargetData* ClientTargetData = TriggerEventData->TargetData.Get(0);
	const FLyraGameplayAbilityTargetData_Inventory_Drop* ClientDropTargetData = static_cast<const FLyraGameplayAbilityTargetData_Inventory_Drop*>(ClientTargetData);
	if (!ClientDropTargetData) return false;
	if (!ClientDropTargetData->ItemInstance) return false;
	if (ClientDropTargetData->DropCount <= 0) return false;
	const AActor* Instigator = TriggerEventData->Instigator.Get();
	if (!Instigator) return false;

	FVector DropLocation = ClientDropTargetData->DropLocation;
	if (DropLocation.IsZero())
	{
		DropLocation = Instigator->GetActorLocation();
	}

	FLyraGameplayAbilityTargetData_Inventory_Drop* ServerDropTargetData = new FLyraGameplayAbilityTargetData_Inventory_Drop(*ClientDropTargetData);
	ServerDropTargetData->DropLocation = DropLocation;

	const FGameplayAbilityTargetDataHandle TargetDataHandle(ServerDropTargetData);
	NotifyTargetDataReady(TargetDataHandle, FGameplayTag());

	return true;
}

void ULyraGameplayAbility_Inventory_Drop::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// 初始化操作
}

void ULyraGameplayAbility_Inventory_Drop::ActivateAbilityWithTargetData_Implementation(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag)
{
	// 这里不要调用父类实现 父类只是 unimplemented 会崩溃

	// 客户端 服务端 都会执行到这里, 使用客户端提供的数据进行一致的逻辑处理

	bool bSuccess = false;
	do
	{
		AController* Controller = GetControllerFromActorInfo();
		const FGameplayAbilityTargetData* BaseTargetData = TargetDataHandle.Get(0);
		const FLyraGameplayAbilityTargetData_Inventory_Drop* TargetData = static_cast<const FLyraGameplayAbilityTargetData_Inventory_Drop*>(BaseTargetData);
		if (!TargetData || !Controller) { break; }
		if (!TargetData->ItemInstance || TargetData->DropCount <= 0) { break; }
		if (TargetData->DropLocation.IsZero()) { break; }

		ULyraInventoryManagerComponent* InventoryManager = Controller->FindComponentByClass<ULyraInventoryManagerComponent>();
		if(!InventoryManager) { break; }

		if (CurrentActorInfo->IsNetAuthority())
		{
			ULyraInventoryItemInstance* DroppedItemInstance = nullptr;
			int32 DroppedStackCount = 0;
			bool bOk = InventoryManager->DropItem(
				TargetData->ItemInstance.Get(),
				TargetData->DropCount,
				DroppedItemInstance,
				DroppedStackCount);
			if (!bOk) { break; }
			ensure(DroppedItemInstance);
			ensure(DroppedStackCount == TargetData->DropCount);

			// SpawnCollectableForInstance 内部将会完成 DroppedItemInstance Owner 以及复制队列的转移
			ALyraWorldCollectable* NewCollectable = ALyraWorldCollectable::SpawnCollectableForInstance(
				Controller,
				DroppedItemInstance,
				DroppedStackCount,
				TargetData->DropLocation,
				FRotator::ZeroRotator,
				Controller->GetPawn(),
				nullptr);
			if (!NewCollectable)
			{
				// TODO 如果生成失败, 则需要将物品放回到背包中 ??
				UE_LOG(LogLyraInventory, Error, TEXT("LyraInventory ====> ULyraGameplayAbility_Inventory_Drop::ActivateAbilityWithTargetData_Implementation: Failed to spawn dropped collectable actor"));
				InventoryManager->AddItemInstance(DroppedItemInstance, nullptr, DroppedStackCount);
				break;
			}
		}

		bSuccess = true;
	}
	while (false);

	if (bSuccess)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

