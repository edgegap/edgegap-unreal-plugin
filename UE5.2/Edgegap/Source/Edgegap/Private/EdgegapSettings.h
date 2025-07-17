#pragma once

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "APIToken/APITokenSettings.h"
#include "Engine/DeveloperSettings.h"
#include "EdgegapSettings.generated.h"

UCLASS(config=EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Edgegap Plugin"))
class UEdgegapSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UEdgegapSettings();

	//~ Begin UObject Interface
#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateSettings, UEdgegapSettings const*);
	static FOnUpdateSettings OnSettingsChange;

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif
	//~ End UObject Interface

	UPROPERTY(Config, EditAnywhere, Category = "API", DisplayName = "API Token")
	FAPITokenSettings APIToken;

	UPROPERTY(Config, EditAnywhere, Category = "Application Info", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Application Name")
	FText ApplicationName;

	UPROPERTY(Config, EditAnywhere, Category = "Application Info", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Application Image")
	FFilePath ImagePath;

	UPROPERTY(Config, EditAnywhere, Category = "Container Registry")
	bool bUseCustomContainerRegistry = false;

	UPROPERTY(Config, EditAnywhere, Category = "Container Registry", Meta = (EditCondition = "bUseCustomContainerRegistry"), DisplayName = "Registry URL")
	FString Registry;

	UPROPERTY(Config, EditAnywhere, Category = "Container Registry", Meta = (EditCondition = "bUseCustomContainerRegistry"), DisplayName = "Repository")
	FString ImageRepository;

	UPROPERTY(Config, EditAnywhere, Category = "Container Registry", Meta = (EditCondition = "bUseCustomContainerRegistry"), DisplayName = "Username")
	FString PrivateRegistryUsername;

	UPROPERTY(Config, EditAnywhere, Category = "Container Registry", Meta = (EditCondition = "bUseCustomContainerRegistry"), DisplayName = "Token")
	FString PrivateRegistryToken;

	UPROPERTY(Config)
	FString Tag;

	UPROPERTY(Config)
	FString VersionName;

	//@TODO: Check the best way to handle verification and toggle edit conditions accordingly
	UPROPERTY(Config, EditAnywhere)
	bool bIsTokenVerified = false;
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif
