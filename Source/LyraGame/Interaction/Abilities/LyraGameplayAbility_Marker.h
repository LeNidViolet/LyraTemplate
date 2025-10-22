// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "LyraGameplayAbility_Marker.generated.h"

#define UE_API LYRAGAME_API

struct FLyraMarkerInstance;
class UIndicatorDescriptor;

/**
 *
 */
UCLASS(Abstract, MinimalAPI)
class ULyraGameplayAbility_Marker : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:

	ULyraGameplayAbility_Marker(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	FTimerHandle TimerHandle;
	void ToggleMarkerPromptVisbility();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Marker")
	float CancelScanRate = 0.2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Marker")
	float CancelRadius = 40;

	UFUNCTION(BlueprintCallable, Category="Marker")
	bool IsAimingAtMarker() const;

	UFUNCTION(BlueprintCallable, Category="Marker")
	UIndicatorDescriptor* GetAimingMarker() const;

	UFUNCTION(BlueprintCallable, Category="Marker")
	FGuid GetLocalPlayerMarkerId(bool& bSuccess);

private:

	void ShowOrHideMarkerPrompt(const TSharedPtr<FLyraMarkerInstance>& Entry, bool bShow);


	TSharedPtr<FLyraMarkerInstance> LocalPlayerMarkerInstance;
	bool bLastPromptVisible = false;
	TWeakObjectPtr<UIndicatorDescriptor> LastDescriptorObject;
};

#undef UE_API