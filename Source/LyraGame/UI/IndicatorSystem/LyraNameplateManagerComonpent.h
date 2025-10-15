// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IndicatorDescriptor.h"
#include "Components/ControllerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraNameplateManagerComonpent.generated.h"

#define UE_API LYRAGAME_API

class ULyraNameplateManagerComonpent;

USTRUCT(BlueprintType)
struct FLyraMessageNameplateInfoAdd
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NameplateInfo")
	TObjectPtr<APawn> Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NameplateInfo")
	TSubclassOf<UIndicatorDescriptor> DescriptorClass;
};

USTRUCT(BlueprintType)
struct FLyraMessageNameplateInfoRemove
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NameplateInfo")
	TObjectPtr<APawn> Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NameplateInfo")
	TObjectPtr<UIndicatorDescriptor> DescriptorObject;
};

USTRUCT(BlueprintType)
struct FLyraMessageNameplateRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NameplateRequest")
	TObjectPtr<ULyraNameplateManagerComonpent> NameplateManagerComonpent;
};

USTRUCT()
struct FLyraMessageNameplateCreatedEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<APawn> Pawn;
	UPROPERTY()
	TSubclassOf<UIndicatorDescriptor> DescriptorClass;
	UPROPERTY()
	TWeakObjectPtr<UIndicatorDescriptor> DescriptorObject;
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
	void HandleNameplateAddEvent(FGameplayTag Channel, const FLyraMessageNameplateInfoAdd& Payload);

	FGameplayMessageListenerHandle NameplateRemoveEventListener;
	void HandleNameplateRemoveEvent(FGameplayTag Channel, const FLyraMessageNameplateInfoRemove& Payload);

	TArray<FLyraMessageNameplateCreatedEntry> NameplateList;
};

#undef UE_API