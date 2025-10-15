// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LyraMarkerManagerComponent.h"
#include "LyraNameplateManagerComonpent.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "IndicatorLibrary.generated.h"

#define UE_API LYRAGAME_API

class AController;
class ULyraIndicatorManagerComponent;
class UObject;
struct FFrame;

UCLASS(MinimalAPI)
class UIndicatorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UE_API UIndicatorLibrary();
	
	/**  */
	UFUNCTION(BlueprintCallable, Category = Indicator)
	static UE_API ULyraIndicatorManagerComponent* GetIndicatorManagerComponent(AController* Controller);

	UFUNCTION(BlueprintCallable, Category = Indicator)
	static UE_API ULyraMarkerManagerComponent* GetMarkerManagerComponent(AController* Controller);

	UFUNCTION(BlueprintCallable, Category = Indicator)
	static UE_API ULyraNameplateManagerComonpent* GetNameplateManagerComponent(AController* Controller);
};

#undef UE_API
