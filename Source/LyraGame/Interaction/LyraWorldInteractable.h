// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IInteractableTarget.h"
#include "GameFramework/Actor.h"
#include "LyraWorldInteractable.generated.h"

#define UE_API LYRAGAME_API


/**
 * 可交互物品基类
 */
UCLASS(Abstract, Blueprintable)
class ALyraWorldInteractable : public AActor, public IInteractableTarget
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALyraWorldInteractable();

	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere)
	FInteractionOption Option;
};

#undef UE_API