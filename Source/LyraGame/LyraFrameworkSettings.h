// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LyraFrameworkSettings.generated.h"

#define UE_API LYRAGAME_API

class UCommonGameDialog;
class UGameUIPolicy;
class ULyraPawnData;
class ULyraGameData;

/**
 * 这里设置一些Lyra框架的全局配置, 之前这些配置只能在 ini 中更改, 现在可以通过编辑器中的项目设置来更改这些配置
 * TODO 后续可能会添加新的配置进来或者把分散在各处的配置集中到这里
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Lyra Framework Settings"))
class ULyraFrameworkSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UE_API ULyraFrameworkSettings() {}

	// 指定它在设置面板中的分类
	UE_API virtual FName GetContainerName() const override { return FName("Project"); }
	UE_API virtual FName GetCategoryName() const override { return FName("Game"); }
	UE_API virtual FName GetSectionName() const override { return FName("LyraFrameworkSettings"); }

	// 默认的 Experience 名称
	UPROPERTY(Config, EditAnywhere, meta=(AllowedTypes="LyraExperienceDefinition"), Category="FrameworkSettings")
	FPrimaryAssetId DefaultExperienceId;

	// LyraGameData 用于配置全局游戏数据
	UPROPERTY(Config, EditAnywhere, Category="FrameworkSettings")
	TSoftObjectPtr<ULyraGameData> DefaultGameData;

	// 默认的 PawnData
	UPROPERTY(Config, EditAnywhere, Category="FrameworkSettings")
	TSoftObjectPtr<ULyraPawnData> DefaultPawnData;

	// 默认的确认对话框类
	UPROPERTY(Config, EditAnywhere, Category="FrameworkSettings")
	TSoftClassPtr<UCommonGameDialog> DefaultConfirmationDialogClass;

	// 默认的错误对话框类
	UPROPERTY(Config, EditAnywhere, Category="FrameworkSettings")
	TSoftClassPtr<UCommonGameDialog> DefaultErrorDialogClass;
};

#undef UE_API
