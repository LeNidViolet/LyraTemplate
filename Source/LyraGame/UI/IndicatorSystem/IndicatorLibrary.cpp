// Copyright Epic Games, Inc. All Rights Reserved.

#include "IndicatorLibrary.h"

#include "LyraIndicatorManagerComponent.h"
#include "LyraNameplateManagerComonpent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IndicatorLibrary)

class AController;

UIndicatorLibrary::UIndicatorLibrary()
{
}

ULyraIndicatorManagerComponent* UIndicatorLibrary::GetIndicatorManagerComponent(AController* Controller)
{
	return ULyraIndicatorManagerComponent::GetComponent(Controller);
}

ULyraMarkerManagerComponent* UIndicatorLibrary::GetMarkerManagerComponent(AController* Controller)
{
	return ULyraMarkerManagerComponent::GetComponent(Controller);
}

ULyraNameplateManagerComonpent* UIndicatorLibrary::GetNameplateManagerComponent(AController* Controller)
{
	return ULyraNameplateManagerComonpent::GetComponent(Controller);
}

