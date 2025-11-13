// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraWorldInteractable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraWorldInteractable)

// Sets default values
ALyraWorldInteractable::ALyraWorldInteractable()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ALyraWorldInteractable::BeginPlay()
{
	Super::BeginPlay();

}

void ALyraWorldInteractable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ALyraWorldInteractable::GatherInteractionOptions(const FInteractionQuery& InteractQuery,
	FInteractionOptionBuilder& InteractionBuilder)
{
	InteractionBuilder.AddInteractionOption(Option);
}

