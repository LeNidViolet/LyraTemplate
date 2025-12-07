// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraGameplayAbility_Inventory_Swap.h"

#include "Inventory/LyraInventoryManagerComponent.h"
#include "TargetData/LyraGameplayAbilityTargetData_Inventory.h"

ULyraGameplayAbility_Inventory_Swap::ULyraGameplayAbility_Inventory_Swap(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 禁止客户端结束服务端能力
	bServerRespectsRemoteAbilityCancellation = false;
}

void ULyraGameplayAbility_Inventory_Swap::ActivateLocalPlayerAbility(
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

bool ULyraGameplayAbility_Inventory_Swap::MakeTargetData(const FGameplayEventData* TriggerEventData)
{
	// 要丢弃的 ItemInstance / 数量 都在 TargetData 里
	if (!TriggerEventData->TargetData.IsValid(0)) return false;

	// 提取数据
	const FGameplayAbilityTargetData* ClientTargetData = TriggerEventData->TargetData.Get(0);
	const FLyraGameplayAbilityTargetData_Inventory_Swap* ClientSwapTargetData = static_cast<const FLyraGameplayAbilityTargetData_Inventory_Swap*>(ClientTargetData);
	if (!ClientSwapTargetData) return false;
	if (!ClientSwapTargetData->ItemInstance) return false;
	if (ClientSwapTargetData->SourceSlotIndex == INDEX_NONE) return false;
	if (ClientSwapTargetData->TargetSlotIndex == INDEX_NONE) return false;

	const AActor* Instigator = TriggerEventData->Instigator.Get();
	if (!Instigator) return false;

	FLyraGameplayAbilityTargetData_Inventory_Swap* ServerSwapTargetData = new FLyraGameplayAbilityTargetData_Inventory_Swap(*ClientSwapTargetData);

	const FGameplayAbilityTargetDataHandle TargetDataHandle(ServerSwapTargetData);
	NotifyTargetDataReady(TargetDataHandle, FGameplayTag());

	return true;
}

void ULyraGameplayAbility_Inventory_Swap::ActivateAbilityWithTargetData_Implementation(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag)
{
	// 这里不要调用父类实现 父类只是 unimplemented 会崩溃

	// 客户端 服务端 都会执行到这里, 使用客户端提供的数据进行一致的逻辑处理

	bool bSuccess = false;
	do
	{
		AController* Controller = GetControllerFromActorInfo();
		const FGameplayAbilityTargetData* BaseTargetData = TargetDataHandle.Get(0);
		const FLyraGameplayAbilityTargetData_Inventory_Swap* TargetData = static_cast<const FLyraGameplayAbilityTargetData_Inventory_Swap*>(BaseTargetData);
		if (!TargetData || !Controller) { break; }
		if (!TargetData->ItemInstance) { break; }
		if (TargetData->SourceSlotIndex == TargetData->TargetSlotIndex) { break; }

		ULyraInventoryManagerComponent* InventoryManager = Controller->FindComponentByClass<ULyraInventoryManagerComponent>();
		if(!InventoryManager) { break; }

		if (CurrentActorInfo->IsNetAuthority())
		{
			bool bOk = InventoryManager->SwapItem(
				TargetData->ItemInstance.Get(),
				TargetData->SourceSlotIndex,
				TargetData->TargetSlotIndex);
			if (!bOk) { break; }
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


void ULyraGameplayAbility_Inventory_Swap::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// 初始化操作
}
