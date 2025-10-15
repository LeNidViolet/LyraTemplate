// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraNameplateManagerComonpent.h"

#include "LyraIndicatorManagerComponent.h"
#include "NativeGameplayTags.h"
#include "Player/LyraPlayerController.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Message_Nameplate_Add, TEXT("Gameplay.Message.Nameplate.Add"))
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Message_Nameplate_Remove, TEXT("Gameplay.Message.Nameplate.Remove"))
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Message_Nameplate_Discover, TEXT("Gameplay.Message.Nameplate.Discover"))


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
			NameplateAddEventListener = MessageSubsystem->RegisterListener<FLyraMessageNameplateInfoAdd>(
				TAG_Message_Nameplate_Add,
				this,
				&ThisClass::HandleNameplateAddEvent);

			NameplateRemoveEventListener = MessageSubsystem->RegisterListener<FLyraMessageNameplateInfoRemove>(
				TAG_Message_Nameplate_Remove,
				this,
				&ThisClass::HandleNameplateRemoveEvent);

			FLyraMessageNameplateRequest NameplateRequest;
			NameplateRequest.NameplateManagerComonpent = this;
			MessageSubsystem->BroadcastMessage(TAG_Message_Nameplate_Discover, NameplateRequest);

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

	for (const FLyraMessageNameplateCreatedEntry& NameplateEntry  : NameplateList)
	{
		if (NameplateEntry.DescriptorObject.IsValid())
		{
			NameplateEntry.DescriptorObject.Get()->UnregisterIndicator();
		}
	}
	NameplateList.Empty();
}

void ULyraNameplateManagerComonpent::HandleNameplateAddEvent(
	FGameplayTag Channel,
	const FLyraMessageNameplateInfoAdd& Payload)
{
	// Register Nameplate Source

	UIndicatorDescriptor* Descriptor = NewObject<UIndicatorDescriptor>(this, Payload.DescriptorClass);
	Descriptor->SetDataObject(Payload.Pawn);

	FLyraMessageNameplateCreatedEntry NameplateEntry;
	NameplateEntry.Pawn = Payload.Pawn;
	NameplateEntry.DescriptorClass = Payload.DescriptorClass;
	NameplateEntry.DescriptorObject = Descriptor;
	NameplateList.Add(NameplateEntry);

	AController* Controller = Cast<AController>(GetOwner());
	if (Controller)
	{
		ULyraIndicatorManagerComponent* IndicatorManager = Controller->GetComponentByClass<ULyraIndicatorManagerComponent>();
		if (IndicatorManager)
		{
			IndicatorManager->AddIndicator(Descriptor);

			UE_LOG(LogTemp, Warning, TEXT("HandleNameplateAddEvent IndicatorManager->AddIndicator"));
		}
	}
}


void ULyraNameplateManagerComonpent::HandleNameplateRemoveEvent(
	FGameplayTag Channel,
	const FLyraMessageNameplateInfoRemove& Payload)
{
	// Unregister Nameplate Source

	for (int32 i = 0; i < NameplateList.Num(); ++i)
	{
		const FLyraMessageNameplateCreatedEntry& NameplateEntry = NameplateList[i];

		if (NameplateEntry.DescriptorObject.IsValid() &&
			Payload.Pawn == NameplateEntry.Pawn &&
			Payload.DescriptorObject == NameplateEntry.DescriptorObject.Get())
		{
			NameplateEntry.DescriptorObject.Get()->UnregisterIndicator();
			UE_LOG(LogTemp, Warning, TEXT("HandleNameplateRemoveEvent IndicatorManager->UnregisterIndicator"));

			NameplateList.RemoveAt(i);
			break;
		}
	}
}

