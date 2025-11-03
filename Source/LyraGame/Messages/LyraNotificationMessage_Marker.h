
#pragma once

#include "LyraNotificationMessage_Marker.generated.h"

class ALyraWorldMarker;

USTRUCT(BlueprintType)
struct FOnAddMarkerParameters
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<ALyraWorldMarker> MarkerActor;
};

USTRUCT(BlueprintType)
struct FOnRemoveMarkerParameters
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 MarkerId = -1;
};
