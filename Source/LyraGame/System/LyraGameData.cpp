// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameData.h"
#include "LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameData)

ULyraGameData::ULyraGameData()
{
}

const ULyraGameData& ULyraGameData::Get()
{
	return ULyraAssetManager::Get().GetGameData();
}

TSubclassOf<UIndicatorDescriptor> ULyraGameData::GetIndicatorClassForMarkerType(ELyraWorldMarkerType WorldMarkerType) const
{
	for (const FLyraWorldMarkerDescriptor& TypeIndicatorMapping : MarkerIndicators)
	{
		if (TypeIndicatorMapping.MarkerType == WorldMarkerType)
		{
			return TypeIndicatorMapping.IndicatorDescriptorClass;
		}
	}
	return TSubclassOf<UIndicatorDescriptor>();
}

TSubclassOf<UUserWidget> ULyraGameData::GetCalloutClassForMarkerType(ELyraWorldMarkerType WorldMarkerType) const
{
	for (const FLyraWorldMarkerCallout& MarkerCallout : MarkerCallouts)
	{
		if (MarkerCallout.MarkerType == WorldMarkerType)
		{
			return MarkerCallout.CalloutClass;
		}
	}
	return TSubclassOf<UUserWidget>();
}
