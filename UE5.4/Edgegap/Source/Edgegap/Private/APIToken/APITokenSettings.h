#pragma once

#include "APITokenSettings.generated.h"

USTRUCT(BlueprintType)
struct FAPITokenSettings
{
	GENERATED_BODY()

public:
	// API Token
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "API Token")
	FString APIToken;
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif
