
#pragma once

#include "LyraNotificationMessage_Nameplate.generated.h"

class APawn;
class UIndicatorDescriptor;
class ULyraNameplateManagerComonpent;

USTRUCT(BlueprintType)
struct FOnAddNameplateParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NameplateInfo")
	TObjectPtr<APawn> Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NameplateInfo")
	TSubclassOf<UIndicatorDescriptor> DescriptorClass;
};

USTRUCT(BlueprintType)
struct FOnRemoveNameplateParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NameplateInfo")
	TObjectPtr<APawn> Pawn;
};

USTRUCT(BlueprintType)
struct FClientRequestNameplateParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NameplateRequest")
	TObjectPtr<ULyraNameplateManagerComonpent> NameplateManagerComonpent;
};
