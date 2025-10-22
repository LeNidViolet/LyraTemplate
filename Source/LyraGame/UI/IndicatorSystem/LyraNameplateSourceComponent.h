// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "LyraNameplateManagerComonpent.h"
#include "LyraNameplateSourceComponent.generated.h"


#define UE_API LYRAGAME_API

struct FClientRequestNameplateParameters;

UCLASS(MinimalAPI)
class ULyraNameplateSourceComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UE_API ULyraNameplateSourceComponent(const FObjectInitializer& ObjectInitializer);

	/** Returns the pawn extension component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "Lyra|Pawn")
	static ULyraNameplateSourceComponent* FindNameplateSourceComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<ULyraNameplateSourceComponent>() : nullptr); }


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Pawn")
	TSubclassOf<UIndicatorDescriptor> DescriptorClass;

	void HandleNameplateDiscoverRequest(FGameplayTag Channel, const FClientRequestNameplateParameters& Parameters);
};

#undef UE_API