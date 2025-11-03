// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraGameplayAbilityTargetDataStatics.generated.h"


#define UE_API LYRAGAME_API

struct FGameplayAbilityTargetDataHandle;
struct FLyraGameplayAbilityTargetData_Marker;


UCLASS(MinimalAPI)
class ULyraGameplayAbilityTargetDataStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TargetData|WorldMarker")
	static UE_API bool TargetData_Marker_HasAimingMarker(const FLyraGameplayAbilityTargetData_Marker& TargetData);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TargetData|WorldMarker")
	static UE_API int32 TargetData_Marker_GetAimingMarkerId(const FLyraGameplayAbilityTargetData_Marker& TargetData);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TargetData|WorldMarker")
	static UE_API AActor* TargetData_Marker_GetTargetActor(const FLyraGameplayAbilityTargetData_Marker& TargetData);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TargetData|WorldMarker")
	static UE_API FVector TargetData_Marker_GetTargetLocation(const FLyraGameplayAbilityTargetData_Marker& TargetData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TargetData|WorldMarker")
	static UE_API bool TargetHandle_Marker_GetHasAimingMarker(const FGameplayAbilityTargetDataHandle& TargetHandle);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TargetData|WorldMarker")
	static UE_API int32 TargetHandle_Marker_GetAimingMarkerId(const FGameplayAbilityTargetDataHandle& TargetHandle);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TargetData|WorldMarker")
	static UE_API AActor* TargetHandle_Marker_GetTargetActor(const FGameplayAbilityTargetDataHandle& TargetHandle);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TargetData|WorldMarker")
	static UE_API FVector TargetHandle_Marker_GetTargetLocation(const FGameplayAbilityTargetDataHandle& TargetHandle);
};

#undef UE_API