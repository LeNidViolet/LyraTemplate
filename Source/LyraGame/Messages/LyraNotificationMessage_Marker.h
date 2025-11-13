
#pragma once

#include "LyraNotificationMessage_Marker.generated.h"

class ALyraWorldMarker;

USTRUCT(BlueprintType)
struct FOnMarkerAddParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ALyraWorldMarker> MarkerActor;
};

USTRUCT(BlueprintType)
struct FOnMarkerRemoveParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MarkerId = -1;
};

USTRUCT(BlueprintType)
struct FOnMarkerDiscoverParameters
{
	GENERATED_BODY()
};