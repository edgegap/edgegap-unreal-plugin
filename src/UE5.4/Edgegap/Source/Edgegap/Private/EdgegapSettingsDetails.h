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
#include "Interfaces/ITargetPlatform.h"
#include "Interfaces/ITargetPlatformModule.h"
#include "DetailWidgetRow.h"

DECLARE_LOG_CATEGORY_EXTERN(EdgegapLog, Log, All);

DECLARE_DELEGATE_OneParam(FOnIsTokenVerifiedChanged, bool);

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
typedef TSharedPtr<IImageWrapper> IImageWrapperPtr;
typedef PlatformInfo::FTargetPlatformInfo FPlatformInfo;


class SImagePreview : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SImagePreview) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, FString ImagePath)
	{
		// Load the image from the specified path
		FSlateBrush ImageBrush;
		ImageBrush.SetResourceObject(LoadObject<UObject>(nullptr, *ImagePath));

		// Create an image widget to display the loaded image
		ChildSlot
			[
				SNew(SImage)
				.Image(&ImageBrush)
			];
	}
};

class FEdgegapSettingsDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static void PackageProject(const FName IniPlatformName);

	static void SaveAll();
	static void AddMessageLog(const FText& Text, const FText& Detail, const FString& TutorialLink, const FString& DocumentationLink);
	static void Containerize(FString DockerFilePath, FString StartScriptPath, FString ServerBuildPath, FString RegistryURL, FString ImageRepository, FString Tag, FString PrivateUsername, FString PrivateToken);
	static void PushContainer(FString ImageName, FString RegistryURL, FString PrivateUsername, FString PrivateToken, bool LoggedIn=false);
	static void DockerLogin(FString RegistryURL, FString PrivateUsername, FString PrivateToken);

	void Request_VerifyToken();
	void Request_CreateApplication(TSharedPtr<SButton> InCreateApplication_SBtn);

	void Request_RegistryCredentials();

	static void CreateVersion(FString AppName, FString VersionName, FString API_key, FString RegistryURL, FString ImageRepository, FString Tag, FString PrivateUsername, FString PrivateToken);
	static void onCreateVersionComplete(FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bWasSuccessful);

	void Request_DeployApp(FString AppName, FString VersionName, FString API_key, TSharedPtr<SButton> InCreateNewDeployment_SBtn);

	void Request_GetDeploymentsInfo(FString API_key, TSharedPtr<SButton> InRefreshBtn);
	static void Callback_GetDeploymentsInfo(FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bWasSuccessful);

	void Request_StopDeploy(FString RequestID, FString API_key);

	static FString _ImageName, _RegistryURL, _PrivateUsername, _PrivateToken, _API_key;
	static FString _AppName, _VersionName;
	static FString _RecentTag;

	FString GetApplicationImageFilename(const bool bInIsGameOverride = false)
	{
		const FString& PlatformName = FModuleManager::GetModuleChecked<ITargetPlatformModule>("WindowsTargetPlatform").GetTargetPlatforms()[0]->PlatformName();

		FString Filename = FString(EDGEGAP_MODULE_PATH) / FString("Resources/ApplicationImage.png");
		return FPaths::ConvertRelativePathToFull(Filename);
	}

	FString GetBannerImageFilePath()
	{
		const FString& PlatformName = FModuleManager::GetModuleChecked<ITargetPlatformModule>("WindowsTargetPlatform").GetTargetPlatforms()[0]->PlatformName();

		FString Filename = FString(EDGEGAP_MODULE_PATH) / FString("Resources/BannerImage.png");
		return FPaths::ConvertRelativePathToFull(Filename);
	}

	const FSlateBrush* HandleImage(FString& InPath) const
	{
		FString pngfile = InPath;

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
					void* TextureData = mytex->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
					FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
					mytex->GetPlatformData()->Mips[0].BulkData.Unlock();

					// Update the rendering resource from data.
					mytex->UpdateResource();

					FSlateBrush* image = new FSlateImageBrush(mytex, FVector2D(ImageWrapper->GetWidth(), ImageWrapper->GetHeight()));

					return image;
				}
			}
		}
		return new FSlateBrush();
	}
	FSlateBrush* LoadImage(const FString& InImagePath);

	const FSlateBrush* GetImageBrush() const
	{
		return SavedImageBrush.IsValid() ? SavedImageBrush.Get() : FAppStyle::Get().GetBrush("ExternalImagePicker.BlankImage");
	}

	static FString MakeImageName(const FString InRegistry, const FString InImageRepository, const FString InAppName, const FString InTag)
	{
		const FString ImageName = FString::Printf(TEXT("%s/%s/%s:%s"), *InRegistry, *InImageRepository, *InAppName.ToLower(), *InTag);
		return ImageName;
	}

	static FString ExpandSequencePathTokens(const FString& InPath)
	{
		return InPath
			.Replace(TEXT("{engine_dir}"), *FPaths::ConvertRelativePathToFull(FPaths::EngineDir()))
			.Replace(TEXT("{project_dir}"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()))
			;
	}

	static FString SanitizeTokenizedSequencePath(const FString& InPath)
	{
		FString SanitizedPickedPath = InPath.TrimStartAndEnd().Replace(TEXT("\""), TEXT(""));

		const FString ProjectAbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());

		// Replace supported tokens
		FString ExpandedPath = ExpandSequencePathTokens(SanitizedPickedPath);

		// Relative paths are always w.r.t. the project root.
		if (FPaths::IsRelative(ExpandedPath))
		{
			ExpandedPath = FPaths::Combine(ProjectAbsolutePath, SanitizedPickedPath);
		}

		// Chop trailing file path, in case the user picked a file instead of a folder
		if (FPaths::FileExists(ExpandedPath))
		{
			ExpandedPath = FPaths::GetPath(ExpandedPath);
			SanitizedPickedPath = FPaths::GetPath(SanitizedPickedPath);
		}

		// If the user picked the absolute path of a directory that is inside the project, use relative path.
		// Unless the user has a token in the beginning.
		if (!InPath.Len() || InPath[0] != '{') // '{' indicates that the path begins with a token
		{
			FString PathRelativeToProject;

			if (IsPathUnderBasePath(ExpandedPath, ProjectAbsolutePath, PathRelativeToProject))
			{
				SanitizedPickedPath = PathRelativeToProject;
			}
		}

		return SanitizedPickedPath;
	}

	static bool IsPathUnderBasePath(const FString& InPath, const FString& InBasePath, FString& OutRelativePath)
	{
		OutRelativePath = InPath;

		return
			FPaths::MakePathRelativeTo(OutRelativePath, *InBasePath)
			&& !OutRelativePath.StartsWith(TEXT(".."));
	}

	TWeakObjectPtr<UEdgegapSettings> Settings;
	TSharedRef<ITableRow> HandleGenerateDeployStatusWidget(TSharedPtr<FDeploymentStatusListItem> InItem, const TSharedRef<STableViewBase>& InOwnerTable);
	TSharedPtr<SDeploymentStatusListItemListView> DeploymentStatusListItemListView;
	TSharedPtr<SButton> CreateApplication_SBtn, CreateBuilAndPushBtn_SBtn, DeploymentStatuRefresh_SBtn, StartDeployButton, CreateNewDeployment_SBtn;

	FDetailWidgetRow* AppNameWidgetRow;
	FDetailWidgetRow* AppImageWidgetRow;

	UPROPERTY()
	static TArray< TSharedPtr< FDeploymentStatusListItem > >	DeployStatusOverrideListSource;

	static FEdgegapSettingsDetails* GetInstance()
	{
		return Singelton;
	}

protected:
	TSharedPtr<SImage> ImageWidget;
	TSharedPtr<IPropertyHandle> ImagePathHandle;

private:
	/** Delegate handler for before an icon is copied */
	bool HandlePreExternalIconCopy(const FString& InChosenImage);

	/** Delegate handler to get the path to start picking from */
	FString GetPickerPath();

	/** Delegate handler to set the path to start picking from */
	bool HandlePostExternalIconCopy(const FString& InChosenImage);

public:
	FOnIsTokenVerifiedChanged OnIsTokenVerifiedChanged;

private:
	TSharedPtr<FSlateDynamicImageBrush> SavedImageBrush;

private:
	static FEdgegapSettingsDetails* Singelton;
};



