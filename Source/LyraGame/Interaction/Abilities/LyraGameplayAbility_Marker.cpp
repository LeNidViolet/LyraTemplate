// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraGameplayAbility_Marker.h"

#include "Character/LyraCharacter.h"
#include "Interaction/LyraWorldMarker.h"
#include "TargetData/LyraGameplayAbilityTargetData_Marker.h"
#include "UI/IndicatorSystem/LyraMarkerManagerComponent.h"


void ULyraGameplayAbility_Marker::MakeTargetData(const FGameplayTag& ApplicationTag)
{
	check(IsLocallyControlled());

	AController* Controller = GetControllerFromActorInfo();
	check(Controller);

	ULyraMarkerManagerComponent* MarkerManagerComponent = ULyraMarkerManagerComponent::GetComponent(Controller);
	check(MarkerManagerComponent);


	// 检查一下是否瞄准了存在的标记点
	FLyraGameplayAbilityTargetData_Marker* TargetData = new FLyraGameplayAbilityTargetData_Marker();
	TargetData->bHasAimingMarker = MarkerManagerComponent->IsAimingAtMarker();
	if (TargetData->bHasAimingMarker)
	{
		ALyraWorldMarker* MarkerActor = MarkerManagerComponent->GetAimingMarkerActor();
		if (IsValid(MarkerActor))
		{
			TargetData->AimingMarkerId = MarkerActor->GetMarkerId();
		}
		else
		{
			TargetData->bHasAimingMarker = false;
		}
	}

	// 如果没有瞄准标记点, 则准备添加标记点的数据
	if (!TargetData->bHasAimingMarker)
	{
		const float TraceRange = 50000.0f;

		ALyraCharacter* Character = GetLyraCharacterFromActorInfo();
		check(Character);

		FVector CameraLocation;
		FRotator CameraRotation;
		// 获取当前玩家的视角位置和旋转
		Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

		// 射线从摄像机位置开始
		FVector TraceStart = CameraLocation;
		// 射线的方向是摄像机的朝向
		FVector Forward = CameraRotation.Vector();
		// 计算射线的结束位置
		FVector TraceEnd = TraceStart + Forward * TraceRange;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Character);
		QueryParams.bTraceComplex = false;

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			QueryParams
			);

		if (bHit)
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor)
			{
				TargetData->TargetActor = HitActor;
				TargetData->TargetLocation = HitResult.Location;
			}
			else
			{
				TargetData->TargetActor = nullptr;
				TargetData->TargetLocation = FVector(0, 0, 0);
			}
		}
	}

	const FGameplayAbilityTargetDataHandle TargetDataHandle(TargetData);

	NotifyTargetDataReady(TargetDataHandle, ApplicationTag);
}
