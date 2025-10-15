// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "LyraToastMessage.generated.h"



USTRUCT(BlueprintType)
struct FLyraToastMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Toast Message")
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Toast Message")
	int32 NumberValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Toast Message")
	bool BooleanValue = false;
};


