#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "EdgegapSettings.generated.h"


UCLASS(config = Edgegap, defaultconfig)
class UEdgegapSettings : public UObject
{
	GENERATED_BODY()

public:

	UEdgegapSettings();

	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
	//~ End UObject Interface


	UPROPERTY(Config, EditAnywhere, Category = "Application Info")
	FText ApplicationName;

	/* Path to app image */
	UPROPERTY(Config, EditAnywhere, Category = "Application Info")
	FFilePath ImagePath;


	UPROPERTY(Config, EditAnywhere, Category = "Version")
	FString VersionName;

	UPROPERTY(Config, EditAnywhere, Category = "Container")
	FString Registry;

	UPROPERTY(Config, EditAnywhere, Category = "Container")
	FString ImageRepository;

	UPROPERTY(Config, EditAnywhere, Category = "Container")
	FString Tag;

	UPROPERTY(Config, EditAnywhere, Category = "Container")
	FString PrivateRegistryUsername;

	UPROPERTY(Config, EditAnywhere, Category = "Container")
	FString PrivateRegistryToken;

	UPROPERTY(Config, EditAnywhere, Category = "API", DisplayName = "API Token")
	FString API_Key;


};