// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraNameplateSourceComponent.h"
#include "NativeGameplayTags.h"
#include "LyraGameplayTags.h"
#include "Messages/LyraNotificationMessage_Nameplate.h"
#include "GameFramework/GameplayMessageSubsystem.h"




// Sets default values for this component's properties
ULyraNameplateSourceComponent::ULyraNameplateSourceComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}



// Called when the game starts
void ULyraNameplateSourceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
	{
		if (DescriptorClass)
		{
			FOnAddNameplateParameters Parameters;
			Parameters.Pawn = GetPawn<APawn>();
			Parameters.DescriptorClass = DescriptorClass;

			UGameplayMessageSubsystem::Get(GetWorld()).BroadcastMessage(
				LyraGameplayTags::Gameplay_Message_Nameplate_Add,
				Parameters
				);

			UGameplayMessageSubsystem::Get(GetWorld()).RegisterListener<FClientRequestNameplateParameters>(
			LyraGameplayTags::Gameplay_Message_Nameplate_Discover,
				this,
				&ThisClass::HandleNameplateDiscoverRequest
				);
		}
	}
}

void ULyraNameplateSourceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
	{
		FOnRemoveNameplateParameters Parameters;
		Parameters.Pawn = GetPawn<APawn>();

		UGameplayMessageSubsystem::Get(GetWorld()).BroadcastMessage(
		LyraGameplayTags::Gameplay_Message_Nameplate_Remove,
			Parameters
			);
	}

	Super::EndPlay(EndPlayReason);
}

void ULyraNameplateSourceComponent::HandleNameplateDiscoverRequest(FGameplayTag Channel,
	const FClientRequestNameplateParameters& Parameters)
{
	if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
	{
		if (DescriptorClass)
		{
			FOnAddNameplateParameters AddParameters;
			AddParameters.Pawn = GetPawn<APawn>();
			AddParameters.DescriptorClass = DescriptorClass;

			UGameplayMessageSubsystem::Get(GetWorld()).BroadcastMessage(
				LyraGameplayTags::Gameplay_Message_Nameplate_Add,
				AddParameters
				);
		}
	}
}


