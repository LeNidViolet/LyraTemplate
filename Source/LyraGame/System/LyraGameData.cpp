// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameData.h"
#include "LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameData)

ULyraGameData::ULyraGameData()
{
}

const ULyraGameData& ULyraGameData::ULyraGameData::Get()
{
	return ULyraAssetManager::Get().GetGameData();
}

TSubclassOf<UIndicatorDescriptor> ULyraGameData::GetIndicatorDescriptorClassForMarkerType(
	ELyraWorldMarkerType MarkerType) const
{
	for (const FLyraWorldMarkerTypeIndicatorMapping& TypeIndicatorMapping : TypeIndicatorMappings)
	{
		if (TypeIndicatorMapping.MarkerType == MarkerType)
		{
			return TypeIndicatorMapping.IndicatorDescriptorClass;
		}
	}
	return TSubclassOf<UIndicatorDescriptor>();
}
