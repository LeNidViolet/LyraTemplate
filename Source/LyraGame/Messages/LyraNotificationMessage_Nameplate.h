
#pragma once

#include "LyraNotificationMessage_Nameplate.generated.h"

class APawn;
class UIndicatorDescriptor;
class ULyraNameplateManagerComonpent;

// 添加 Nameplate 的事件参数 由 Nameplate Source Component 广播
USTRUCT(BlueprintType)
struct FOnNameplateAddParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nameplate")
	TObjectPtr<APawn> Pawn;
};

// 移除 Nameplate 的事件参数 由 Nameplate Source Component 广播
USTRUCT(BlueprintType)
struct FOnNameplateRemoveParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nameplate")
	TObjectPtr<APawn> Pawn;
};

// 请求发现 Nameplate 的事件参数 由 Nameplate Manager Component 广播, Nameplate Source Component 监听并响应
USTRUCT(BlueprintType)
struct FOnNameplateDiscoverParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nameplate")
	TObjectPtr<ULyraNameplateManagerComonpent> NameplateManagerComonpent;
};
