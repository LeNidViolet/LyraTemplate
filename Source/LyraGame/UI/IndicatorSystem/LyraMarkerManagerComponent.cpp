// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraMarkerManagerComponent.h"

#include "LyraIndicatorManagerComponent.h"
#include "NativeGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Player/LyraPlayerController.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Message_Marker_Toggle, TEXT("Gameplay.Message.Marker.Toggle"))


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

TSharedPtr<FLyraMarkerInstance> ULyraMarkerManagerComponent::GetMarkerInstance()
{
	return MarkerInstance;
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

	if (MarkerInstance.IsValid() &&
		MarkerInstance->DescriptorObject.IsValid() &&
		MarkerInstance->DescriptorObject == DescriptorObject)
	{
		Distance = FMath::RoundToInt(FVector::Dist(TargetLocation, MarkerInstance->Location) / 100.0f);
	}

	return Distance;
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
			MarkerToggleEventListener = MessageSubsystem->RegisterListener<FLyraMessageMarkerToggle>(
				TAG_Message_Marker_Toggle,
				this,
				&ThisClass::HandleMarkerToggleEvent);

			Result = true;

			MarkerInstance = MakeShared<FLyraMarkerInstance>();
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
			if (MarkerToggleEventListener.IsValid())
			{
				MessageSubsystem->UnregisterListener(MarkerToggleEventListener);
			}
		}
	}

	RemoveExistingMarker();
}

void ULyraMarkerManagerComponent::HandleMarkerToggleEvent(FGameplayTag Channel, const FLyraMessageMarkerToggle& Payload)
{
	if (Payload.bAddMarker) HandleMarkerAdd(Payload);
	else HandleMarkerRemove(Payload);
}

void ULyraMarkerManagerComponent::HandleMarkerAdd(const FLyraMessageMarkerToggle& Payload)
{
	if (Payload.Actor != nullptr && Payload.Actor->GetRootComponent())
	{
		// The rest of the settings are coming from the **CDO**.
		UIndicatorDescriptor* Descriptor = NewObject<UIndicatorDescriptor>(this, Payload.DescriptorClass);
		Descriptor->SetSceneComponent(Payload.Actor->GetRootComponent());
		Descriptor->SetWorldPosition(Payload.Location);
		Descriptor->SetDataObject(Payload.Actor.Get());

		RemoveExistingMarker();
		MarkerInstance->Location = Payload.Location;
		MarkerInstance->DescriptorClass = Payload.DescriptorClass;
		MarkerInstance->DescriptorObject = Descriptor;

		AController* Controller = Cast<AController>(GetOwner());
		if (Controller)
		{
			ULyraIndicatorManagerComponent* IndicatorManager = Controller->GetComponentByClass<ULyraIndicatorManagerComponent>();
			if (IndicatorManager)
			{
				IndicatorManager->AddIndicator(Descriptor);
			}
		}
	}
}

void ULyraMarkerManagerComponent::HandleMarkerRemove(const FLyraMessageMarkerToggle& Payload)
{
	if (MarkerInstance.IsValid() && MarkerInstance->DescriptorObject.IsValid())
	{
		if (Payload.DescriptorObject == MarkerInstance->DescriptorObject)
		{
			RemoveExistingMarker();
		}
	}
}

void ULyraMarkerManagerComponent::RemoveExistingMarker()
{
	if (MarkerInstance.IsValid() && MarkerInstance->DescriptorObject.IsValid())
	{
		MarkerInstance->DescriptorObject->UnregisterIndicator();
		MarkerInstance->Location = FVector::ZeroVector;
		MarkerInstance->DescriptorObject.Reset();
	}
}


