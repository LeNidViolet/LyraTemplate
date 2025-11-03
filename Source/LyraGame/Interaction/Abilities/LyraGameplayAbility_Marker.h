// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraXCLGameplayAbilityClientToServer.h"
#include "LyraGameplayAbility_Marker.generated.h"

#define UE_API LYRAGAME_API



UCLASS(MinimalAPI)
class ULyraGameplayAbility_Marker : public ULyraXCLGameplayAbilityClientToServer
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Ability")
	void MakeTargetData(const FGameplayTag& ApplicationTag);
};


#undef UE_API