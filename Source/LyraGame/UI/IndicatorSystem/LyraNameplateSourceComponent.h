// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "LyraNameplateManagerComonpent.h"
#include "LyraNameplateSourceComponent.generated.h"


#define UE_API LYRAGAME_API

struct FOnNameplateDiscoverParameters;

UCLASS(MinimalAPI)
class ULyraNameplateSourceComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UE_API ULyraNameplateSourceComponent(const FObjectInitializer& ObjectInitializer);

	/** Returns the pawn extension component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "Component")
	static UE_API ULyraNameplateSourceComponent* GetComponent(const AActor* Actor);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void RegisterMessageHandlers();
	void UnregisterMessageHandlers();
	void BroadcastNameplateAddMessage();
	void BroadcastNameplateRemoveMessage();

	FGameplayMessageListenerHandle NameplateDiscoverListenerHandle;
	void HandleNameplateDiscoverRequest(FGameplayTag Channel, const FOnNameplateDiscoverParameters& Parameters);
};

#undef UE_API