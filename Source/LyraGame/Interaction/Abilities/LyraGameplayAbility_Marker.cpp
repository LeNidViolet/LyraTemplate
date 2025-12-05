// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraGameplayAbility_Marker.h"

#include "Character/LyraCharacter.h"
#include "GameModes/Session/LyraDSPlayerState.h"
#include "System/LyraGameData.h"
#include "TargetData/LyraGameplayAbilityTargetData_Marker.h"
#include "UI/IndicatorSystem/LyraMarkerManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameplayAbility_Marker)

void ULyraGameplayAbility_Marker::ActivateLocalPlayerAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateLocalPlayerAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 本地要做的事情就是准备好数据并发送给服务器
	bool bOk = MakeTargetData(ApplicationTag);
	if (!bOk)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}


bool ULyraGameplayAbility_Marker::MakeTargetData(const FGameplayTag& ApplicationTag)
{
	check(IsLocallyControlled());

	AController* Controller = GetControllerFromActorInfo();
	if (!Controller)
	{
		return false;
	}
	ULyraMarkerManagerComponent* MarkerManagerComponent = ULyraMarkerManagerComponent::GetComponent(Controller);
	if (!MarkerManagerComponent)
	{
		return false;
	}

	bool bHasValidData = false;

	// 检查一下是否瞄准了存在的标记点
	FLyraGameplayAbilityTargetData_Marker* TargetData = new FLyraGameplayAbilityTargetData_Marker();
	TargetData->bHasAimingMarker = MarkerManagerComponent->IsAimingAtMarker();
	if (TargetData->bHasAimingMarker)
	{
		ALyraWorldMarker* MarkerActor = MarkerManagerComponent->GetAimingMarkerActor();
		if (MarkerActor)
		{
			TargetData->AimingMarkerId = MarkerActor->GetMarkerId();
			bHasValidData = true;
		}
		else
		{
			TargetData->bHasAimingMarker = false;
		}
	}


	// 如果没有瞄准标记点, 则准备添加标记点的数据
	if (!bHasValidData)
	{
		ALyraCharacter* Character = GetLyraCharacterFromActorInfo();

		FVector CameraLocation;
		FRotator CameraRotation;
		// 获取当前摄像机的视角位置和旋转
		Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

		// 射线从摄像机位置开始
		FVector TraceStart = CameraLocation;
		// 射线的方向是摄像机的朝向
		FVector Forward = CameraRotation.Vector();
		// 计算射线的结束位置
		FVector TraceEnd = TraceStart + Forward * TraceDistance;

		FCollisionQueryParams QueryParams;
		if (Character)
		{
			QueryParams.AddIgnoredActor(Character);
		}
		QueryParams.bTraceComplex = false;

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			TraceChannel,
			QueryParams
			);
		if (bHit)
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor)
			{
				TargetData->TargetActor = HitActor;
				TargetData->TargetLocation = HitResult.Location;
				bHasValidData = true;
			}
			else
			{
				TargetData->TargetActor = nullptr;
				TargetData->TargetLocation = FVector(0, 0, 0);
			}
		}
	}

	if (!bHasValidData)
	{
		// 没有有效数据
		delete TargetData;
	}
	else
	{
		const FGameplayAbilityTargetDataHandle TargetDataHandle(TargetData);
		NotifyTargetDataReady(TargetDataHandle, ApplicationTag);
	}

	return bHasValidData;
}


void ULyraGameplayAbility_Marker::ActivateAbilityWithTargetData_Implementation(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag)
{
	// 这里不要调用父类实现 父类只是 unimplemented 会崩溃

	// 客户端 服务端 都会执行到这里, 使用客户端提供的数据进行一致的逻辑处理

	AController* Controller = GetControllerFromActorInfo();

	const FGameplayAbilityTargetData* BaseTargetData = TargetDataHandle.Get(0);
	const FLyraGameplayAbilityTargetData_Marker* TargetData = static_cast<const FLyraGameplayAbilityTargetData_Marker*>(BaseTargetData);
	if (!TargetData || !Controller)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (TargetData->bHasAimingMarker)
	{
		// 移除标记
		if (TargetData->AimingMarkerId != ALyraWorldMarker::InvalidId())
		{
			ALyraDSPlayerState* PlayerState = Controller->GetPlayerState<ALyraDSPlayerState>();
			if (PlayerState)
			{
				// 这里服务端会移除真正的标记点, 客户端的移除操作会失败, 因为客户端只保存着预测的标记点
				// 客户端的标记点移除会经过 MarkerActor 的销毁事件同步到客户端, 然后进行广播通知
				PlayerState->RemoveWorldMarkerForId(TargetData->AimingMarkerId);
			}
		}
	}
	else
	{
		// 添加标记
		if (TargetData->TargetActor)
		{
			ALyraWorldMarker::SpawnLyraWorldMarkerActor(
				Controller,
				MarkerClass,
				TargetData->TargetActor.Get(),
				TargetData->TargetLocation,
				MarkerType,
				Controller->PlayerState,
				/*bAsPredictedMarker=*/!GetCurrentActorInfo()->IsNetAuthority());
		}
	}

	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void ULyraGameplayAbility_Marker::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	TraceChannel = ULyraGameData::Get().MarkerTraceChannel;
	TraceDistance = ULyraGameData::Get().MarkerTraceDistance;
	MarkerClass = ULyraGameData::Get().MarkerClass;
	MarkerType = ULyraGameData::Get().MarkerType;
}
