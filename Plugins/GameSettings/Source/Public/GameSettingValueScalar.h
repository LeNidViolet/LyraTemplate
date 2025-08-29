// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingValue.h"
#include "Math/Range.h"

#include "GameSettingValueScalar.generated.h"

#define UE_API GAMESETTINGS_API

class UObject;

UCLASS(MinimalAPI, abstract)
class UGameSettingValueScalar : public UGameSettingValue
{
	GENERATED_BODY()

public:
	UE_API UGameSettingValueScalar();

	UE_API void SetValueNormalized(double NormalizedValue);
	UE_API double GetValueNormalized() const;

	TOptional<double> GetDefaultValueNormalized() const
	{
		TOptional<double> DefaultValue = GetDefaultValue();
		if (DefaultValue.IsSet())
		{
			return FMath::GetMappedRangeValueClamped(GetSourceRange(), TRange<double>(0, 1), DefaultValue.GetValue());
		}
		return TOptional<double>();
	}

	UE_API virtual TOptional<double> GetDefaultValue() const						PURE_VIRTUAL(, return TOptional<double>(););
	UE_API virtual void SetValue(double Value, EGameSettingChangeReason Reason = EGameSettingChangeReason::Change)	PURE_VIRTUAL(, );
	UE_API virtual double GetValue() const											PURE_VIRTUAL(, return 0;);
	UE_API virtual TRange<double> GetSourceRange() const							PURE_VIRTUAL(, return TRange<double>(););
	UE_API virtual double GetSourceStep() const									PURE_VIRTUAL(, return 0.01;);
	double GetNormalizedStepSize() const
	{
		TRange<double> SourceRange = GetSourceRange();
		return GetSourceStep() / FMath::Abs(SourceRange.GetUpperBoundValue() - SourceRange.GetLowerBoundValue());
	}
	UE_API virtual FText GetFormattedText() const									PURE_VIRTUAL(, return FText::GetEmpty(););
	
	virtual FString GetAnalyticsValue() const override
	{
		return LexToString(GetValue());
	}

protected:
};

#undef UE_API
