
#pragma once

#include "OnlineKeyValuePair.h"
#include "AsyncAction_Types.generated.h"

UENUM(BlueprintType)
enum class EEIKAttributeType : uint8
{
	String  UMETA(DisplayName = "String"),
	Bool    UMETA(DisplayName = "Bool"),
	Integer UMETA(DisplayName = "Integer")
};

USTRUCT(BlueprintType)
struct FEIKAttribute
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommonUser")
	EEIKAttributeType AttributeType = EEIKAttributeType::String;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommonUser", meta = (EditCondition = "AttributeType == EEIKAttributeType::String", EditConditionHides))
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommonUser", meta = (EditCondition = "AttributeType == EEIKAttributeType::Bool", EditConditionHides))
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommonUser", meta = (EditCondition = "AttributeType == EEIKAttributeType::Integer", EditConditionHides))
	int32 IntValue = 0;

	FEIKAttribute() = default;

	// Construct from type and value
	explicit FEIKAttribute(const FString& InValue)
		: AttributeType(EEIKAttributeType::String), StringValue(InValue) {}
	explicit FEIKAttribute(bool InValue)
		: AttributeType(EEIKAttributeType::Bool), BoolValue(InValue) {}
	explicit FEIKAttribute(int32 InValue)
		: AttributeType(EEIKAttributeType::Integer), IntValue(InValue) {}

	// Convert to FVariantData
	FVariantData ToVariantData() const
	{
		FVariantData VariantData;
		switch (AttributeType)
		{
		case EEIKAttributeType::String:
			VariantData.SetValue(StringValue);
			break;
		case EEIKAttributeType::Bool:
			VariantData.SetValue(BoolValue);
			break;
		case EEIKAttributeType::Integer:
			VariantData.SetValue(IntValue);
			break;
		default:
			break;
		}
		return VariantData;
	}

	// Construct from FVariantData
	explicit FEIKAttribute(const FVariantData& VariantData)
	{
		switch (VariantData.GetType())
		{
		case EOnlineKeyValuePairDataType::Type::String:
			AttributeType = EEIKAttributeType::String;
			VariantData.GetValue(StringValue);
			break;
		case EOnlineKeyValuePairDataType::Type::Bool:
			AttributeType = EEIKAttributeType::Bool;
			VariantData.GetValue(BoolValue);
			break;
		case EOnlineKeyValuePairDataType::Type::Int32:
			AttributeType = EEIKAttributeType::Integer;
			VariantData.GetValue(IntValue);
			break;
		default:
			AttributeType = EEIKAttributeType::String;
			StringValue = TEXT("Unsupported VariantData type");
			break;
		}
	}
};