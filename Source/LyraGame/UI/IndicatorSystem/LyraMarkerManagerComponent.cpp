// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraMarkerManagerComponent.h"

#include "LyraIndicatorManagerComponent.h"
#include "NativeGameplayTags.h"
#include "LyraGameplayTags.h"
#include "Messages/LyraNotificationMessage_Marker.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Player/LyraPlayerController.h"



// Sets default values for this component's properties
ULyraMarkerManagerComponent::ULyraMarkerManagerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

ULyraMarkerManagerComponent* ULyraMarkerManagerComponent::GetComponent(AController* Controller)
{
	if (Controller)
	{
		return Controller->FindComponentByClass<ULyraMarkerManagerComponent>();
	}

	return nullptr;
}

TSharedPtr<FLyraMarkerInstance> ULyraMarkerManagerComponent::GetMarkerInstance(APlayerState* PlayerState)
{
	for (TSharedPtr<FLyraMarkerInstance>& MarkerInstance : MarkerList)
	{
		if (MarkerInstance->PlayerState == PlayerState)
		{
			return MarkerInstance;
		}
	}
	return nullptr;
}

// Called when the game starts
void ULyraMarkerManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	ALyraPlayerController* PC = Cast<ALyraPlayerController>(GetOwner());
	if (PC)
	{
		if (PC->IsLocalPlayerController())
		{
			if (Initialize())
			{
				return ;
			}
		}
	}

	DestroyComponent();

}

void ULyraMarkerManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ALyraPlayerController* PC = Cast<ALyraPlayerController>(GetOwner());
	if (PC)
	{
		if (PC->IsLocalPlayerController())
		{
			Deinitialize();
			return ;
		}
	}
	Super::EndPlay(EndPlayReason);
}

int32 ULyraMarkerManagerComponent::GetDistanceToLocation(UIndicatorDescriptor* DescriptorObject, const FVector& TargetLocation)
{
	int32 Distance = -1;

	for (TSharedPtr<FLyraMarkerInstance>& MarkerInstance : MarkerList)
	{
		if (MarkerInstance->DescriptorObject.IsValid() && MarkerInstance->DescriptorObject == DescriptorObject)
		{
			Distance = FMath::RoundToInt(FVector::Dist(TargetLocation, MarkerInstance->Location) / 100.0f);
			break;
		}
	}

	return Distance;
}

FGuid ULyraMarkerManagerComponent::GetMarkerId(UIndicatorDescriptor* DescriptorObject)
{
	for (TSharedPtr<FLyraMarkerInstance>& MarkerInstance : MarkerList)
	{
		if (MarkerInstance->DescriptorObject.IsValid() && MarkerInstance->DescriptorObject == DescriptorObject)
		{
			return MarkerInstance->MarkerId;
		}
	}

	return FGuid();
}

APlayerState* ULyraMarkerManagerComponent::GetMarkerPlayerState(UIndicatorDescriptor* DescriptorObject)
{
	for (TSharedPtr<FLyraMarkerInstance>& MarkerInstance : MarkerList)
	{
		if (MarkerInstance->DescriptorObject.IsValid() && MarkerInstance->DescriptorObject == DescriptorObject)
		{
			return MarkerInstance->PlayerState.Get();
		}
	}

	return nullptr;
}

bool ULyraMarkerManagerComponent::Initialize()
{
	bool Result = false;
	UGameInstance* GameInstance = GetOwner()->GetGameInstance();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			MarkerAddEventListener = MessageSubsystem->RegisterListener<FOnPlaceMarkerParameters>(
				LyraGameplayTags::Gameplay_Message_Marker_Add,
				this,
				&ThisClass::HandlePlaceMarkerEvent);

			MarkerRemoveEventListener = MessageSubsystem->RegisterListener<FOnRemoveMarkerParameters>(
				LyraGameplayTags::Gameplay_Message_Marker_Remove,
				this,
				&ThisClass::HandleRemoveMarkerEvent);

			Result = true;
		}
	}

	return Result;
}

void ULyraMarkerManagerComponent::Deinitialize()
{
	UGameInstance* GameInstance = GetOwner()->GetGameInstance();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			if (MarkerAddEventListener.IsValid())
			{
				MessageSubsystem->UnregisterListener(MarkerAddEventListener);
			}
			if (MarkerRemoveEventListener.IsValid())
			{
				MessageSubsystem->UnregisterListener(MarkerRemoveEventListener);
			}
		}
	}
}

void ULyraMarkerManagerComponent::HandlePlaceMarkerEvent(FGameplayTag Channel, const FOnPlaceMarkerParameters& Parameters)
{
	if (Parameters.PlayerState && DescriptorClass)
	{
		// The rest of the settings are coming from the **CDO**.
		UIndicatorDescriptor* Descriptor = NewObject<UIndicatorDescriptor>(this, DescriptorClass);
		Descriptor->SetActor(Parameters.PlayerState.Get());
		Descriptor->SetWorldPosition(Parameters.Location);

		TSharedPtr<FLyraMarkerInstance> MarkerInstance = MakeShared<FLyraMarkerInstance>();
		MarkerInstance->PlayerState = Parameters.PlayerState;
		MarkerInstance->MarkerId = Parameters.MarkerId;
		MarkerInstance->Location = Parameters.Location;
		MarkerInstance->DescriptorObject = Descriptor;
		MarkerList.Add(MarkerInstance);

		AController* Controller = Cast<AController>(GetOwner());
		check(Controller);

		ULyraIndicatorManagerComponent* IndicatorManager = Controller->GetComponentByClass<ULyraIndicatorManagerComponent>();
		if (IndicatorManager)
		{
			IndicatorManager->AddIndicator(Descriptor);
		}
	}
}

void ULyraMarkerManagerComponent::HandleRemoveMarkerEvent(FGameplayTag Channel, const FOnRemoveMarkerParameters& Parameters)
{
	int32 FoundIndex = MarkerList.IndexOfByPredicate([Parameters](const TSharedPtr<FLyraMarkerInstance>& MarkerInstance)
	{
		return MarkerInstance->PlayerState == Parameters.PlayerState && MarkerInstance->MarkerId == Parameters.MarkerId;
	});
	
	if (MarkerList.IsValidIndex(FoundIndex))
	{
		TSharedPtr<FLyraMarkerInstance>& MarkerInstance = MarkerList[FoundIndex];
		if (MarkerInstance->DescriptorObject.IsValid())
		{
			MarkerInstance->DescriptorObject->UnregisterIndicator();
		}
		MarkerInstance->PlayerState = nullptr;
		MarkerInstance->DescriptorObject = nullptr;
		MarkerList.RemoveAt(FoundIndex);
	}
}
