// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoreMinimal.h"
//#include "EdgegapTestLib.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(EdgegapTestLog, Log, All);

/*
UCLASS()
class UEdgegapTestLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Edgegap")
	static void Package(const FName InPlatformInfoName);

	UFUNCTION(BlueprintCallable, Category = "Edgegap")
	static void Containerize(FString DockerFilePath, FString ServerBuildPath, FString RegistryURL, FString ImageRepository, FString Tag, FString PrivateUsername, FString PrivateToken);

	UFUNCTION(BlueprintCallable, Category = "Edgegap")
	static void PushContainer(FString ImageName, FString RegistryURL, FString PrivateUsername, FString PrivateToken, bool LoggedIn);
	
	UFUNCTION(BlueprintCallable, Category = "Edgegap")
	static void DockerLogin(FString RegistryURL, FString PrivateUsername, FString PrivateToken);


};
*/