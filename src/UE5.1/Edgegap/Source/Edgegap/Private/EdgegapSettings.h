#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
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

	UPROPERTY(Config, EditAnywhere, Category = "Ports")
	TArray<FVersionPortMapping> Ports;

	UPROPERTY(Config, EditAnywhere, Category = "API", DisplayName = "API Token")
	FString API_Key;


};