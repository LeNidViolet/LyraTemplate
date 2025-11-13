#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraCalloutHostWidget.generated.h"


struct FOnCalloutDisplayParameters;
class UVerticalBox;

UCLASS()
class ULyraCalloutHostWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	ULyraCalloutHostWidget(const FObjectInitializer& ObjectInitializer);

	//~UUserWidget interface
	virtual void NativeTick(const FGeometry& MyGeometry,
	                        float InDeltaTime) override;
	virtual int32 NativePaint(const FPaintArgs& Args,
	                          const FGeometry& AllottedGeometry,
	                          const FSlateRect& MyCullingRect,
	                          FSlateWindowElementList& OutDrawElements,
	                          int32 LayerId, const FWidgetStyle& InWidgetStyle,
	                          bool bParentEnabled) const override;
	//~End of UUserWidget interface

	UFUNCTION(BlueprintCallable, Category = "Callout")
	void SetTargetActor(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "Callout")
	void SetTargetActorWorldOffset(FVector2D InOffset);

	UFUNCTION(BlueprintCallable, Category = "Callout")
	void SetCalloutWidgetClass(TSubclassOf<UUserWidget> InCalloutWidgetClass);

	UFUNCTION(BlueprintCallable, Category = "Callout")
	void SetCalloutWidgetScreenOffset(FVector2D InOffset);

protected:
	//~UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~End of UUserWidget interface

	// 需要在蓝图绑定
	UPROPERTY(BlueprintReadOnly, Category = "Callout", meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox;

	// 使用此小部件类型作为信息框
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Callout")
	TSubclassOf<UUserWidget> CalloutWidgetClass;

	// 要跟踪的目标Actor
	UPROPERTY(BlueprintReadOnly, Category = "Callout")
	TWeakObjectPtr<AActor> TargetActor;

private:

	bool GetTargetScreenPosition(FVector2D& OutScreenPos) const;

	// 每一帧计算得到的目标位置
	FVector2D TargetActorPixelPos;
	bool bIsTargetOnScreen = false;
	bool bShouldShowCallout = false;

	UPROPERTY()
	TObjectPtr<UUserWidget> CalloutWidget;

	float LineThickness = 0.f;
	FLinearColor LineColor = FLinearColor::White;
	FVector2D CalloutScreenOffset = FVector2D::ZeroVector;
	FVector2D ActorScreenOffset = FVector2D::ZeroVector;

	FGameplayMessageListenerHandle DisplayCalloutEventListener;
	void RegisterMessageHandlers();
	void UnregisterMessageHandlers();
	void HandleCalloutDisplayEvent(FGameplayTag Channel, const FOnCalloutDisplayParameters& Parameters);
};
