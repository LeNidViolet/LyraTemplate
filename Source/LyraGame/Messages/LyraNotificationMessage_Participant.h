#pragma once

#include "LyraNotificationMessage_Participant.generated.h"

UENUM(Blueprintable)
enum class EOnSessionParticipantEventType : uint8
{
	Unknown = 0,
	Left UMETA(DisplayName = "Participant Left"),
	Join UMETA(DisplayName = "Participant Join")
};

USTRUCT(BlueprintType)
struct FOnSessionParticipantEventParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOnSessionParticipantEventType EventType = EOnSessionParticipantEventType::Unknown;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ParticipantName;
};

