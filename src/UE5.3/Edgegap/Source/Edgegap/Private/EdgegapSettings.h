#pragma once

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "APIToken/APITokenSettings.h"
#include "Engine/DeveloperSettings.h"
#include "EdgegapSettings.generated.h"

UENUM(BlueprintType)
enum class EPortProtocol : uint8
{
	UDP UMETA(DisplayName = "UDP"),
	TCP UMETA(DisplayName = "TCP"),
	TCP_UDP UMETA(DisplayName = "TCP/UDP"),
	HTTP UMETA(DisplayName = "HTTP"),
	HTTPS UMETA(DisplayName = "HTTPS"),
	WS UMETA(DisplayName = "WS"),
	WSS UMETA(DisplayName = "WSS")
};

USTRUCT(BlueprintType)
struct FVersionPortMapping
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Port;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPortProtocol Protocol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool toCheck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool tlsUpgrade;

	FVersionPortMapping(FString _Name = "", int _Port = 0, EPortProtocol _Protocol = EPortProtocol::TCP_UDP, bool _toCheck = false, bool _tlsUpgrade = false)
	{
		Name = _Name;
		Port = _Port;
		Protocol = _Protocol;
		toCheck = _toCheck;
		tlsUpgrade = _tlsUpgrade;
	}

	static FString GetProtocolString(FVersionPortMapping Port)
	{
		switch (Port.Protocol)
		{
		case EPortProtocol::UDP: return "UDP";
		case EPortProtocol::TCP: return "TCP";
		case EPortProtocol::TCP_UDP: return "TCP/UDP";
		case EPortProtocol::HTTP: return "HTTP";
		case EPortProtocol::HTTPS: return "HTTPS";
		case EPortProtocol::WS: return "WS";
		case EPortProtocol::WSS: return "WSS";
		default: return "TCP/UDP";
		}
	}
};

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

	UPROPERTY(Config, EditAnywhere, Category = "Ports")
	TArray<FVersionPortMapping> Ports;

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
