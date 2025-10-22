// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraNameplateManagerComonpent.h"

#include "LyraIndicatorManagerComponent.h"
#include "NativeGameplayTags.h"
#include "LyraGameplayTags.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Messages/LyraNotificationMessage_Nameplate.h"
#include "Player/LyraPlayerController.h"



// Sets default values for this component's properties
ULyraNameplateManagerComonpent::ULyraNameplateManagerComonpent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

ULyraNameplateManagerComonpent* ULyraNameplateManagerComonpent::GetComponent(AController* Controller)
{
	if (Controller)
	{
		return Controller->FindComponentByClass<ULyraNameplateManagerComonpent>();
	}

	return nullptr;
}


// Called when the game starts
void ULyraNameplateManagerComonpent::BeginPlay()
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

void ULyraNameplateManagerComonpent::EndPlay(const EEndPlayReason::Type EndPlayReason)
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


bool ULyraNameplateManagerComonpent::Initialize()
{
	bool Result = false;

	UGameInstance* GameInstance = GetOwner()->GetGameInstance();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			NameplateAddEventListener = MessageSubsystem->RegisterListener<FOnAddNameplateParameters>(
				LyraGameplayTags::Gameplay_Message_Nameplate_Add,
				this,
				&ThisClass::HandleAddNameplateEvent);

			NameplateRemoveEventListener = MessageSubsystem->RegisterListener<FOnRemoveNameplateParameters>(
			LyraGameplayTags::Gameplay_Message_Nameplate_Remove,
				this,
				&ThisClass::HandleRemoveNameplateEvent);

			FClientRequestNameplateParameters NameplateRequest;
			NameplateRequest.NameplateManagerComonpent = this;
			MessageSubsystem->BroadcastMessage(LyraGameplayTags::Gameplay_Message_Nameplate_Discover, NameplateRequest);

			Result = true;
		}
	}

	return Result;
}

void ULyraNameplateManagerComonpent::Deinitialize()
{
	UGameInstance* GameInstance = GetOwner()->GetGameInstance();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			if (NameplateAddEventListener.IsValid())
			{
				MessageSubsystem->UnregisterListener(NameplateAddEventListener);
			}
			if (NameplateRemoveEventListener.IsValid())
			{
				MessageSubsystem->UnregisterListener(NameplateRemoveEventListener);
			}
		}
	}

	for (const FNameplateCreatedEntry& NameplateEntry  : NameplateList)
	{
		if (NameplateEntry.DescriptorObject.IsValid())
		{
			NameplateEntry.DescriptorObject.Get()->UnregisterIndicator();
		}
	}
	NameplateList.Empty();
}

void ULyraNameplateManagerComonpent::HandleAddNameplateEvent(
	FGameplayTag Channel,
	const FOnAddNameplateParameters& Parameters)
{
	// Register Nameplate Source

	UIndicatorDescriptor* Descriptor = NewObject<UIndicatorDescriptor>(this, Parameters.DescriptorClass);
	ACharacter* Character = Cast<ACharacter>(Parameters.Pawn);
	if (Character)
	{
		Descriptor->SetSceneComponent(Character->GetCapsuleComponent());
	}
	else
	{
		Descriptor->SetSceneComponent(Parameters.Pawn->GetRootComponent());
	}


	Descriptor->SetDataObject(Parameters.Pawn);

	FNameplateCreatedEntry NameplateEntry;
	NameplateEntry.Pawn = Parameters.Pawn;
	NameplateEntry.DescriptorClass = Parameters.DescriptorClass;
	NameplateEntry.DescriptorObject = Descriptor;
	NameplateList.Add(NameplateEntry);

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


void ULyraNameplateManagerComonpent::HandleRemoveNameplateEvent(
	FGameplayTag Channel,
	const FOnRemoveNameplateParameters& Parameters)
{
	// Unregister Nameplate Source

	for (int32 i = 0; i < NameplateList.Num(); ++i)
	{
		const FNameplateCreatedEntry& NameplateEntry = NameplateList[i];

		if (NameplateEntry.DescriptorObject.IsValid() &&
			Parameters.Pawn == NameplateEntry.Pawn)
		{
			NameplateEntry.DescriptorObject.Get()->UnregisterIndicator();
			NameplateList.RemoveAt(i);
			break;
		}
	}
}

