// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IndicatorDescriptor.h"
#include "Components/ControllerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraNameplateManagerComonpent.generated.h"

#define UE_API LYRAGAME_API


struct FOnNameplateRemoveParameters;
struct FOnNameplateAddParameters;
class ULyraNameplateManagerComonpent;

USTRUCT()
struct FNameplateCreatedEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<APawn> Pawn;
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

	UFUNCTION(BlueprintPure, Category = "Component")
	static UE_API ULyraNameplateManagerComonpent* GetComponent(const AController* Controller);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void RegisterMessageHandlers();
	void UnregisterMessageHandlers();

	FGameplayMessageListenerHandle NameplateAddEventListener;
	void HandleNameplateAddEvent(FGameplayTag Channel, const FOnNameplateAddParameters& Parameters);

	FGameplayMessageListenerHandle NameplateRemoveEventListener;
	void HandleNameplateRemoveEvent(FGameplayTag Channel, const FOnNameplateRemoveParameters& Parameters);

	TArray<FNameplateCreatedEntry> NameplateList;
};

#undef UE_API