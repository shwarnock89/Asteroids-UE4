#pragma once

#include "MessageStruct.generated.h"

UENUM(BlueprintType)
namespace EMessageTypes
{
	enum Type
	{
		Float,
		Int,
		String,
		None
	};
}

USTRUCT(BlueprintType)
struct FMessage
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	float floatMessage;

	UPROPERTY(BlueprintReadWrite)
	int intMessage;

	UPROPERTY(BlueprintReadOnly)
	FString stringMessage;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EMessageTypes::Type> typeMessage;

	UPROPERTY(BlueprintReadOnly)
	ESizes asteroidSizeMessage;

	UPROPERTY(BlueprintReadOnly)
	FVector currentPosMessage;

	UPROPERTY(BlueprintReadWrite)
	bool isNewHighScore;
};
