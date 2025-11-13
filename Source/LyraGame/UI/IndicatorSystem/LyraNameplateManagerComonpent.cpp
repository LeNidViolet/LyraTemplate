// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraNameplateManagerComonpent.h"

#include "LyraIndicatorManagerComponent.h"
#include "NativeGameplayTags.h"
#include "LyraGameplayTags.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Messages/LyraNotificationMessage_Nameplate.h"
#include "Player/LyraPlayerController.h"
#include "System/LyraGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraNameplateManagerComonpent)

// Sets default values for this component's properties
ULyraNameplateManagerComonpent::ULyraNameplateManagerComonpent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

ULyraNameplateManagerComonpent* ULyraNameplateManagerComonpent::GetComponent(const AController* Controller)
{
	return Controller ? Controller->FindComponentByClass<ULyraNameplateManagerComonpent>() : nullptr;
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
			RegisterMessageHandlers();
			return ;
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
			UnregisterMessageHandlers();
		}
	}

	// 注销清理所有Nameplate对象
	for (const FNameplateCreatedEntry& NameplateEntry  : NameplateList)
	{
		if (NameplateEntry.IndicatorDescriptor.IsValid())
		{
			NameplateEntry.IndicatorDescriptor.Get()->UnregisterIndicator();
		}
	}
	NameplateList.Empty();

	Super::EndPlay(EndPlayReason);
}


void ULyraNameplateManagerComonpent::HandleNameplateAddEvent(
	FGameplayTag Channel,
	const FOnNameplateAddParameters& Parameters)
{
	// Register Nameplate Source

	TSubclassOf<UIndicatorDescriptor> IndicatorClass = ULyraGameData::Get().NameplateIndicatorClass;

	UIndicatorDescriptor* Descriptor = NewObject<UIndicatorDescriptor>(this, IndicatorClass);
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
	NameplateEntry.IndicatorDescriptor = Descriptor;
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


void ULyraNameplateManagerComonpent::HandleNameplateRemoveEvent(
	FGameplayTag Channel,
	const FOnNameplateRemoveParameters& Parameters)
{
	// Unregister Nameplate Source

	for (int32 i = 0; i < NameplateList.Num(); ++i)
	{
		const FNameplateCreatedEntry& NameplateEntry = NameplateList[i];

		if (NameplateEntry.IndicatorDescriptor.IsValid() &&
			Parameters.Pawn == NameplateEntry.Pawn)
		{
			NameplateEntry.IndicatorDescriptor.Get()->UnregisterIndicator();
			NameplateList.RemoveAt(i);
			break;
		}
	}
}

void ULyraNameplateManagerComonpent::RegisterMessageHandlers()
{
	UGameInstance* GameInstance = GetGameInstance<UGameInstance>();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			if (!NameplateAddEventListener.IsValid())
			{
				NameplateAddEventListener = MessageSubsystem->RegisterListener<FOnNameplateAddParameters>(
					LyraGameplayTags::Gameplay_Message_Nameplate_Add,
					this,
					&ThisClass::HandleNameplateAddEvent);
			}
			if (!NameplateRemoveEventListener.IsValid())
			{
				NameplateRemoveEventListener = MessageSubsystem->RegisterListener<FOnNameplateRemoveParameters>(
					LyraGameplayTags::Gameplay_Message_Nameplate_Remove,
					this,
					&ThisClass::HandleNameplateRemoveEvent);
			}

			// 请求局内的 Nameplate Source Component 回复 Nameplate Add 消息
			FOnNameplateDiscoverParameters NameplateRequest;
			NameplateRequest.NameplateManagerComonpent = this;
			MessageSubsystem->BroadcastMessage(LyraGameplayTags::Gameplay_Message_Nameplate_Discover, NameplateRequest);
		}
	}
}

void ULyraNameplateManagerComonpent::UnregisterMessageHandlers()
{
	if (NameplateAddEventListener.IsValid())
	{
		NameplateAddEventListener.Unregister();
	}
	if (NameplateRemoveEventListener.IsValid())
	{
		NameplateRemoveEventListener.Unregister();
	}
}

