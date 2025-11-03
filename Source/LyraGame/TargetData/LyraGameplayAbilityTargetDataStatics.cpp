// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraGameplayAbilityTargetDataStatics.h"
#include "LyraGameplayAbilityTargetData_Marker.h"

bool ULyraGameplayAbilityTargetDataStatics::TargetData_Marker_HasAimingMarker(const FLyraGameplayAbilityTargetData_Marker& TargetData)
{
	return TargetData.bHasAimingMarker;
}

int32 ULyraGameplayAbilityTargetDataStatics::TargetData_Marker_GetAimingMarkerId(const FLyraGameplayAbilityTargetData_Marker& TargetData)
{
	return TargetData.AimingMarkerId;
}

AActor* ULyraGameplayAbilityTargetDataStatics::TargetData_Marker_GetTargetActor(const FLyraGameplayAbilityTargetData_Marker& TargetData)
{
	return TargetData.TargetActor.Get();
}

FVector ULyraGameplayAbilityTargetDataStatics::TargetData_Marker_GetTargetLocation(
	const FLyraGameplayAbilityTargetData_Marker& TargetData)
{
	return TargetData.TargetLocation;
}

bool ULyraGameplayAbilityTargetDataStatics::TargetHandle_Marker_GetHasAimingMarker(const FGameplayAbilityTargetDataHandle& TargetHandle)
{
	const FGameplayAbilityTargetData* BaseTargetData = TargetHandle.Get(0);
	const FLyraGameplayAbilityTargetData_Marker* TargetData = static_cast<const FLyraGameplayAbilityTargetData_Marker*>(BaseTargetData);

	return TargetData ? TargetData_Marker_HasAimingMarker(*TargetData) : false;
}

int32 ULyraGameplayAbilityTargetDataStatics::TargetHandle_Marker_GetAimingMarkerId(const FGameplayAbilityTargetDataHandle& TargetHandle)
{
	const FGameplayAbilityTargetData* BaseTargetData = TargetHandle.Get(0);
	const FLyraGameplayAbilityTargetData_Marker* TargetData = static_cast<const FLyraGameplayAbilityTargetData_Marker*>(BaseTargetData);

	return TargetData ? TargetData_Marker_GetAimingMarkerId(*TargetData) : -1;
}

AActor* ULyraGameplayAbilityTargetDataStatics::TargetHandle_Marker_GetTargetActor(const FGameplayAbilityTargetDataHandle& TargetHandle)
{
	const FGameplayAbilityTargetData* BaseTargetData = TargetHandle.Get(0);
	const FLyraGameplayAbilityTargetData_Marker* TargetData = static_cast<const FLyraGameplayAbilityTargetData_Marker*>(BaseTargetData);

	return TargetData ? TargetData_Marker_GetTargetActor(*TargetData) : nullptr;
}

FVector ULyraGameplayAbilityTargetDataStatics::TargetHandle_Marker_GetTargetLocation(const FGameplayAbilityTargetDataHandle& TargetHandle)
{
	const FGameplayAbilityTargetData* BaseTargetData = TargetHandle.Get(0);
	const FLyraGameplayAbilityTargetData_Marker* TargetData = static_cast<const FLyraGameplayAbilityTargetData_Marker*>(BaseTargetData);

	return TargetData ? TargetData_Marker_GetTargetLocation(*TargetData) : FVector::ZeroVector;
}
