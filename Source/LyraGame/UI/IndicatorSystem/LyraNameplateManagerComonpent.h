// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IndicatorDescriptor.h"
#include "Components/ControllerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraNameplateManagerComonpent.generated.h"

#define UE_API LYRAGAME_API


struct FOnRemoveNameplateParameters;
struct FOnAddNameplateParameters;
class ULyraNameplateManagerComonpent;

USTRUCT()
struct FNameplateCreatedEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<APawn> Pawn;
	UPROPERTY()
	TSubclassOf<UIndicatorDescriptor> DescriptorClass;
	UPROPERTY()
	TWeakObjectPtr<UIndicatorDescriptor> IndicatorDescriptor;
};


UCLASS(MinimalAPI)
class ULyraNameplateManagerComonpent : public UControllerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UE_API ULyraNameplateManagerComonpent(const FObjectInitializer& ObjectInitializer);

	static UE_API ULyraNameplateManagerComonpent* GetComponent(AController* Controller);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool Initialize();
	void Deinitialize();

	FGameplayMessageListenerHandle NameplateAddEventListener;
	void HandleAddNameplateEvent(FGameplayTag Channel, const FOnAddNameplateParameters& Parameters);

	FGameplayMessageListenerHandle NameplateRemoveEventListener;
	void HandleRemoveNameplateEvent(FGameplayTag Channel, const FOnRemoveNameplateParameters& Parameters);

	TArray<FNameplateCreatedEntry> NameplateList;
};

#undef UE_API