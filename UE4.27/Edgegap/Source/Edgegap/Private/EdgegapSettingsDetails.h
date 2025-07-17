// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"
#include "IUCMDHelperModule.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Brushes/SlateImageBrush.h"
#include "EdgegapSettings.h"
#include "Misc/App.h"
#include "Interfaces/IPluginManager.h"

DECLARE_LOG_CATEGORY_EXTERN(EdgegapLog, Log, All);

class UcmdTaskResultCallack;

template <typename ItemType>
class SCustomListView : public SListView< ItemType >
{
public:
	void SetListItemsSource(const TArray<ItemType>* InListItemsSource)
	{
		AsyncTask(ENamedThreads::GameThread, [this, InListItemsSource] {
			this->Private_ClearSelection();
			this->CancelScrollIntoView();
			this->ClearWidgets();
			this->RebuildList();
			this->ItemsSource = InListItemsSource;
			});
		
		TArray<ItemType> x = (*this->ItemsSource);
	}
};


struct FDeploymentStatusListItem
{
public:
	FString DeploymentIP, DeploymentStatus, RequestID, API_Key;
	bool DeploymentReady;



	FDeploymentStatusListItem() {}

	FDeploymentStatusListItem(FString InDeploymentIP, FString InDeploymentStatus, FString InRequestID, FString InAPI_Key, bool InDeploymentReady)
		: DeploymentIP(InDeploymentIP)
		, DeploymentStatus(InDeploymentStatus)
		, RequestID(InRequestID)
		, API_Key(InAPI_Key)
		, DeploymentReady(InDeploymentReady)
	{
	}
};

typedef SCustomListView< TSharedPtr< struct FDeploymentStatusListItem > > SDeploymentStatusListItemListView;


class FEdgegapSettingsDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	
	void Add_API_UI(IDetailLayoutBuilder& DetailBuilder);
	void Add_Documentation_UI(IDetailLayoutBuilder& DetailBuilder);
	void AddAppInfoUI(IDetailLayoutBuilder& DetailBuilder);
	void AddContainerUI(IDetailLayoutBuilder& DetailBuilder);
	void AddDeploymentStatusTableUI(IDetailLayoutBuilder& DetailBuilder);
	void AddDeployUI(IDetailLayoutBuilder& DetailBuilder);

	static void PackageProject(const FName InPlatformInfoName);

	static void SaveAll();
	static void AddMessageLog(const FText& Text, const FText& Detail, const FString& TutorialLink, const FString& DocumentationLink);
	static void Containerize(FString DockerFilePath, FString StartScriptPath, FString ServerBuildPath, FString RegistryURL, FString ImageRepository, FString Tag, FString PrivateUsername, FString PrivateToken);
	static void PushContainer(FString ImageName, FString RegistryURL, FString PrivateUsername, FString PrivateToken, bool LoggedIn=false);
	static void DockerLogin(FString RegistryURL, FString PrivateUsername, FString PrivateToken);
	static void CreateApp(FString AppName, FString ImagePath, FString API_key);
	static void onCreateAppComplete(FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bWasSuccessful);

	static void CreateVersion(FString AppName, FString VersionName, FString API_key, FString RegistryURL, FString ImageRepository, FString Tag, FString PrivateUsername, FString PrivateToken);
	static void onCreateVersionComplete(FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bWasSuccessful);

	static void DeployApp(FString AppName, FString VersionName, FString API_key);
	static void onDeployApp(FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bWasSuccessful);

	static void GetDeploymentsInfo(FString API_key);
	static void onGetDeploymentsInfo(FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bWasSuccessful);

	static void StopDeploy(FString RequestID, FString API_key);
	static void onStopDeploy(FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bWasSuccessful);

	static FString _ImageName, _RegistryURL, _PrivateUsername, _PrivateToken, _API_key;
	static FString _AppName, _VersionName;

	const FSlateBrush* HandleImage() const
	{
		FString PluginDir = IPluginManager::Get().FindPlugin(FString("Edgegap"))->GetBaseDir();
		FString pngfile = FPaths::Combine(FString(EDGEGAP_MODULE_PATH), FString("/Resources/SettingsIcon.png"));

		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		// Note: PNG format.  Other formats are supported
		IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

		TArray<uint8> RawFileData;
		if (FFileHelper::LoadFileToArray(RawFileData, *pngfile))
		{
			if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
			{
				TArray<uint8> UncompressedBGRA;
				if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
				{
					UTexture2D* mytex = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
					void* TextureData = mytex->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
					FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
					mytex->PlatformData->Mips[0].BulkData.Unlock();

					// Update the rendering resource from data.
					mytex->UpdateResource();

					FSlateBrush* image = new FSlateImageBrush(mytex, FVector2D(ImageWrapper->GetWidth(), ImageWrapper->GetHeight()));

					return image;
				}
			}
		}
		return new FSlateBrush();
	}

	TWeakObjectPtr<UEdgegapSettings> Settings;
	TSharedRef<ITableRow> HandleGenerateDeployStatusWidget(TSharedPtr<FDeploymentStatusListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedPtr<SDeploymentStatusListItemListView> DeploymentStatusListItemListView;
	TSharedPtr<SButton> DeploymentStatuRefreshButton, StartDeployButton, StartUploadyButton;

	UPROPERTY()
	static TArray< TSharedPtr< FDeploymentStatusListItem > >	DeployStatusOverrideListSource;

	static FEdgegapSettingsDetails* GetInstance()
	{
		return Singelton;
	}

private:
	static FEdgegapSettingsDetails* Singelton;
};



