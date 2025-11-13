// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CommonGameSettings.generated.h"

#define UE_API COMMONGAME_API

class UGameUIPolicy;
/**
 *
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="CommonGame Settings"))
class UCommonGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UE_API UCommonGameSettings() {}

	// 指定它在设置面板中的分类
	UE_API virtual FName GetContainerName() const override { return FName("Project"); }
	UE_API virtual FName GetCategoryName() const override { return FName("Game"); }
	UE_API virtual FName GetSectionName() const override { return FName("CommonGameSettings"); }

	// 默认的 UI 策略类
	UPROPERTY(Config, EditAnywhere, Category="FrameworkSettings")
	TSoftClassPtr<UGameUIPolicy> DefaultUIPolicyClass;
};

#undef UE_API