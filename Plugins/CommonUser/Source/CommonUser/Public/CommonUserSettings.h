// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CommonUserSettings.generated.h"

#define UE_API COMMONUSER_API

/**
 *
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="CommonUser Settings"))
class UCommonUserSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UE_API UCommonUserSettings() {}

	// 指定它在设置面板中的分类
	UE_API virtual FName GetContainerName() const override { return FName("Project"); }
	UE_API virtual FName GetCategoryName() const override { return FName("Game"); }
	UE_API virtual FName GetSectionName() const override { return FName("CommonUserSettings"); }

	// 是否覆盖服务器地址
	UPROPERTY(Config, EditAnywhere, Category="Connection")
	bool bOverrideServerAddress = false;

	// 主服务器地址
	UPROPERTY(Config, EditAnywhere, Category="Connection")
	FString ServerAddress = TEXT("127.0.0.1:7777");

	// 信标服务器地址
	UPROPERTY(Config, EditAnywhere, Category="Connection")
	FString ServerAddressBeacon = TEXT("127.0.0.1:15000");
};

#undef UE_API
