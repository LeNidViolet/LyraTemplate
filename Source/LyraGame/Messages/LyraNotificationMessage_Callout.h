#pragma once

#include "LyraNotificationMessage_Callout.generated.h"

class AActor;
class UUserWidget;


// 需要显示或者隐藏 callout 的事件参数
USTRUCT(BlueprintType)
struct FOnCalloutDisplayParameters
{
	GENERATED_BODY()

	// 显示还是隐藏
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShow = true;

	// 要显示标注的Actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> Actor;

	// Actor端点世界空间偏移
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D ActorScreenOffset = FVector2D::ZeroVector;

	// Widget类
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> CalloutWidgetClass;

	// Widget端点屏幕空间偏移
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D CalloutWidgetScreenOffset = FVector2D::ZeroVector;
};

