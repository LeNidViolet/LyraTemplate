
#pragma once

#include "LyraNotificationMessage_Marker.generated.h"

USTRUCT(BlueprintType)
struct FServerRequestPlaceMarkerParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::Zero();
};

USTRUCT(BlueprintType)
struct FServerRequestRemoveMarkerParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid MarkerId;
};

USTRUCT(BlueprintType)
struct FOnPlaceMarkerParameters
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Location = FVector::Zero();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGuid MarkerId;
};

USTRUCT(BlueprintType)
struct FOnRemoveMarkerParameters
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGuid MarkerId;
};
