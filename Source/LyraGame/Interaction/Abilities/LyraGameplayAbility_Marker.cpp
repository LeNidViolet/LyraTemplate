// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraGameplayAbility_Marker.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerState.h"
#include "Player/LyraPlayerController.h"
#include "Interaction/IInteractableMarker.h"
#include "UI/IndicatorSystem/LyraMarkerManagerComponent.h"


ULyraGameplayAbility_Marker::ULyraGameplayAbility_Marker(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = ELyraAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void ULyraGameplayAbility_Marker::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ActorInfo && ActorInfo->IsLocallyControlled())
	{
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::ToggleMarkerPromptVisbility, CancelScanRate, true);
	}
}

bool ULyraGameplayAbility_Marker::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                                     const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void ULyraGameplayAbility_Marker::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->IsLocallyControlled())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool ULyraGameplayAbility_Marker::IsAimingAtMarker() const
{
	return bLastPromptVisible;
}

UIndicatorDescriptor* ULyraGameplayAbility_Marker::GetAimingMarker() const
{
	if (IsAimingAtMarker() && LocalPlayerMarkerInstance.IsValid() && LocalPlayerMarkerInstance->DescriptorObject.IsValid())
	{
		return LocalPlayerMarkerInstance->DescriptorObject.Get();
	}
	return nullptr;
}

FGuid ULyraGameplayAbility_Marker::GetLocalPlayerMarkerId(bool& bSuccess)
{
	if (!LocalPlayerMarkerInstance.IsValid() || !LocalPlayerMarkerInstance->DescriptorObject.IsValid())
	{
		ALyraPlayerController* PlayerController = GetLyraPlayerControllerFromActorInfo();
		if (PlayerController)
		{
			ULyraMarkerManagerComponent* MarkerComponent = ULyraMarkerManagerComponent::GetComponent(PlayerController);
			if (MarkerComponent)
			{
				LocalPlayerMarkerInstance = MarkerComponent->GetMarkerInstance(PlayerController->GetPlayerState<APlayerState>());
			}
		}
	}

	if (LocalPlayerMarkerInstance.IsValid() && LocalPlayerMarkerInstance->DescriptorObject.IsValid())
	{
		bSuccess = true;
		return LocalPlayerMarkerInstance->MarkerId;
	}
	bSuccess = false;
	return FGuid();
}

void ULyraGameplayAbility_Marker::ShowOrHideMarkerPrompt(const TSharedPtr<FLyraMarkerInstance>& Entry, bool bShow)
{
	if (!Entry.IsValid()) return;

	TWeakObjectPtr<UUserWidget> Widget = Entry->DescriptorObject->IndicatorWidget;
	if (!Widget.IsValid()) return;

	if (Widget->GetClass()->ImplementsInterface(UInteractableMarker::StaticClass()))
	{
		if (bShow)
			IInteractableMarker::Execute_OnShowMarkerInteractablePrompt(Widget.Get(), Entry->DescriptorObject.Get());
		else
			IInteractableMarker::Execute_OnHideMarkerInteractablePrompt(Widget.Get(), Entry->DescriptorObject.Get());
	}
}

void ULyraGameplayAbility_Marker::ToggleMarkerPromptVisbility()
{
	ALyraPlayerController* PlayerController = GetLyraPlayerControllerFromActorInfo();
	if (PlayerController)
	{
		ULyraMarkerManagerComponent* MarkerComponent = ULyraMarkerManagerComponent::GetComponent(PlayerController);
		if (MarkerComponent)
		{
			if (!LocalPlayerMarkerInstance.IsValid() || !LocalPlayerMarkerInstance->DescriptorObject.IsValid())
			{
				LocalPlayerMarkerInstance = MarkerComponent->GetMarkerInstance(PlayerController->GetPlayerState<APlayerState>());
			}

			if (LocalPlayerMarkerInstance.IsValid() && LocalPlayerMarkerInstance->DescriptorObject.IsValid())
			{
				int32 ViewportX, ViewportY;
				PlayerController->GetViewportSize(ViewportX, ViewportY);
				FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);

				FVector2D MarkerScreenPos;
				PlayerController->ProjectWorldLocationToScreen(LocalPlayerMarkerInstance->Location, MarkerScreenPos);
				const float PixelDistance = FVector2D::Distance(MarkerScreenPos, ScreenCenter);

				bool bShouldShowPrompt = PixelDistance <= CancelRadius;
				bool bNewObject = LastDescriptorObject != LocalPlayerMarkerInstance->DescriptorObject;

				if ((bShouldShowPrompt != bLastPromptVisible) || bNewObject)
				{
					if (bNewObject)
					{
						LastDescriptorObject = LocalPlayerMarkerInstance->DescriptorObject;
					}
					ShowOrHideMarkerPrompt(LocalPlayerMarkerInstance, bShouldShowPrompt);
					bLastPromptVisible = bShouldShowPrompt;
				}
			}
		}
	}
}
