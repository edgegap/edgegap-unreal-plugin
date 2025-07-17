// Copyright Epic Games, Inc. All Rights Reserved.

#include "UCMDHelperModule.h"
#include "CoreTypes.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"
#include "Logging/LogMacros.h"
#include "Templates/SharedPointer.h"
#include "Internationalization/Text.h"
#include "Internationalization/Internationalization.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Stats/Stats.h"
#include "Misc/MonitoredProcess.h"
#include "Modules/ModuleManager.h"
#include "Async/TaskGraphInterfaces.h"
#include "Framework/Docking/TabManager.h"
#include "Editor.h"
#include "EditorAnalytics.h"
#include "IUCMDHelperModule.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Editor/EditorPerProjectUserSettings.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Logging/TokenizedMessage.h"
#include "Logging/MessageLog.h"
#include "Developer/MessageLog/Public/IMessageLogListing.h"
#include "Developer/MessageLog/Public/MessageLogModule.h"
#include "Misc/UObjectToken.h"

#include "GameProjectGenerationModule.h"
#include "AnalyticsEventAttribute.h"

#include "ShaderCompiler.h"

#define LOCTEXT_NAMESPACE "UCMDHelper"

DEFINE_LOG_CATEGORY_STATIC(UCMDHelper, Log, All);

/* Event Data
*****************************************************************************/
struct EventData
{
	FString EventName;
	bool bProjectHasCode;
	double StartTime;
	IUCMDHelperModule::UcmdTaskResultCallack ResultCallback;
};

/* FMainFrameActionCallbacks callbacks
*****************************************************************************/

class FMainFrameActionsNotificationTask
{
public:

	FMainFrameActionsNotificationTask(TWeakPtr<SNotificationItem> InNotificationItemPtr, SNotificationItem::ECompletionState InCompletionState, const FText& InText, const FText& InLinkText = FText(), bool InExpireAndFadeout = true)
		: CompletionState(InCompletionState)
		, NotificationItemPtr(InNotificationItemPtr)
		, Text(InText)
		, LinkText(InLinkText)
		, bExpireAndFadeout(InExpireAndFadeout)

	{ }

	static void HandleHyperlinkNavigate()
	{
		FMessageLog("PackagingResults").Open(EMessageSeverity::Error, true);
	}

	static void HandleDismissButtonClicked()
	{
		TSharedPtr<SNotificationItem> NotificationItem = ExpireNotificationItemPtr.Pin();
		if (NotificationItem.IsValid())
		{

			NotificationItem->SetExpireDuration(0.0f);
			NotificationItem->SetFadeOutDuration(0.0f);
			NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
			NotificationItem->ExpireAndFadeout();
			ExpireNotificationItemPtr.Reset();
		}
	}

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		if (NotificationItemPtr.IsValid())
		{
			if (CompletionState == SNotificationItem::CS_Fail)
			{
				GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
			}
			else
			{
				GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
			}

			TSharedPtr<SNotificationItem> NotificationItem = NotificationItemPtr.Pin();
			NotificationItem->SetText(Text);

			if (!LinkText.IsEmpty())
			{
				FText VLinkText(LinkText);
				const TAttribute<FText> Message = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([VLinkText]()
					{
						return VLinkText;
					}));

				NotificationItem->SetHyperlink(FSimpleDelegate::CreateStatic(&HandleHyperlinkNavigate), Message);

			}

			ExpireNotificationItemPtr = NotificationItem;
			if (bExpireAndFadeout)
			{
				NotificationItem->SetExpireDuration(6.0f);
				NotificationItem->SetFadeOutDuration(0.5f);
				NotificationItem->SetCompletionState(CompletionState);
				NotificationItem->ExpireAndFadeout();
			}
			else
			{
				// Handling the notification expiration in callback
				NotificationItem->SetCompletionState(CompletionState);
			}

			// Since the task was probably fairly long, we should try and grab the users attention if they have the option enabled.
			const UEditorPerProjectUserSettings* SettingsPtr = GetDefault<UEditorPerProjectUserSettings>();
			if (SettingsPtr->bGetAttentionOnUATCompletion)
			{
				IMainFrameModule* MainFrame = FModuleManager::LoadModulePtr<IMainFrameModule>("MainFrame");
				if (MainFrame != nullptr)
				{
					TSharedPtr<SWindow> ParentWindow = MainFrame->GetParentWindow();
					if (ParentWindow != nullptr)
					{
						ParentWindow->DrawAttention(FWindowDrawAttentionParameters(EWindowDrawAttentionRequestType::UntilActivated));
					}
				}
			}
		}
	}

	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
	ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FMainFrameActionsNotificationTask, STATGROUP_TaskGraphTasks);
	}

private:

	static TWeakPtr<SNotificationItem> ExpireNotificationItemPtr;

	SNotificationItem::ECompletionState CompletionState;
	TWeakPtr<SNotificationItem> NotificationItemPtr;
	FText Text;
	FText LinkText;
	bool bExpireAndFadeout;
};

TWeakPtr<SNotificationItem> FMainFrameActionsNotificationTask::ExpireNotificationItemPtr;

/**
* Helper class to deal with packaging issues encountered in UCMD.
**/
class FPackagingErrorHandler
{

public:


private:

	/**
	* Create a message to send to the Message Log.
	*
	* @Param MessageString - The error we wish to send to the Message Log.
	* @Param MessageType - The severity of the message, i.e. error, warning etc.
	**/
	static void AddMessageToMessageLog(FString MessageString, EMessageSeverity::Type MessageType)
	{
		if (!bSawSummary && (MessageType == EMessageSeverity::Error || MessageType == EMessageSeverity::Warning))
		{
			FAssetData AssetData;

			// Parse the warning/error into an array and check whether there is an asset on the path
			TArray<FString> MessageArray;
			MessageString.ParseIntoArray(MessageArray, TEXT(": "), true);

			FString AssetPath = MessageArray.Num() > 0 ? MessageArray[0] : TEXT("");
			if (AssetPath.Len())
			{
				// Convert from the asset's full path provided by UE_ASSET_LOG back to an AssetData, if possible
				FString LongPackageName;
				FPaths::NormalizeFilename(AssetPath);
				if (!FPaths::IsRelative(AssetPath) && FPackageName::TryConvertFilenameToLongPackageName(AssetPath, LongPackageName))
				{
					// Generate qualified asset path and query the registry
					AssetPath = LongPackageName + TEXT(".") + FPackageName::GetShortName(LongPackageName);
					FName AssetPathName(*AssetPath, FNAME_Find);
					if (!AssetPathName.IsNone())
					{
						static const FName AssetRegistryModuleName(TEXT("AssetRegistry"));
						FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryModuleName);
						AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(AssetPathName, true);
					}
				}
			}


			if (AssetData.IsValid())
			{
				// we have asset errors in the cook
				if (MessageType == EMessageSeverity::Error)
				{
					bHasAssetErrors = true;
				}

				TSharedRef<FTokenizedMessage> PackagingMsg = MessageType == EMessageSeverity::Error ? FMessageLog("PackagingResults").Error()
					: FMessageLog("PackagingResults").Warning();

				PackagingMsg->AddToken(FTextToken::Create(FText::FromString(MessageArray.Num() > 1 ? MessageArray[1] : MessageString)));

				if (AssetData.IsValid())
				{
					PackagingMsg->AddToken(FUObjectToken::Create(AssetData.GetAsset()));
				}

			}

		}

		// note: CookResults:Warning: actually outputs some unhandled errors.
		FText MsgText = FText::FromString(MessageString);

		TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(MessageType);
		Message->AddToken(FTextToken::Create(MsgText));

		FMessageLog MessageLog("PackagingResults");
		MessageLog.AddMessage(Message);

	}

	/**
	* Send Error to the Message Log.
	*
	* @Param MessageString - The error we wish to send to the Message Log.
	* @Param MessageType - The severity of the message, i.e. error, warning etc.
	**/
	static void SyncMessageWithMessageLog(FString MessageString, EMessageSeverity::Type MessageType)
	{
		DECLARE_CYCLE_STAT(TEXT("FSimpleDelegateGraphTask.SendPackageErrorToMessageLog"),
			STAT_FSimpleDelegateGraphTask_SendPackageErrorToMessageLog,
			STATGROUP_TaskGraphTasks);

		// Remove any new line terminators
		MessageString.ReplaceInline(TEXT("\r"), TEXT(""));
		MessageString.ReplaceInline(TEXT("\n"), TEXT(""));

		/**
		* Dispatch the error from packaging to the message log.
		**/
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic(&FPackagingErrorHandler::AddMessageToMessageLog, MessageString, MessageType),
			GET_STATID(STAT_FSimpleDelegateGraphTask_SendPackageErrorToMessageLog),
			nullptr, ENamedThreads::GameThread
		);
	}

	// Whether there are asset errors in the cook, which can be navigated to in the content browser.
	static bool bHasAssetErrors;
	// Whether the cook summary has been seen in the log.
	static bool bSawSummary;

public:

	/**
	* Determine if the output is an communication message we wish to process.
	*
	* @Param UCMDOutput - The current line of output from the UCMD package process.
	**/
	static bool ProcessAndHandleCookMessageOutput(FString UCMDOutput)
	{
		FString LhsUCMDOutputMsg, ParsedCookIssue;
		if (UCMDOutput.Split(TEXT("Shaders left to compile "), &LhsUCMDOutputMsg, &ParsedCookIssue))
		{
			if (GShaderCompilingManager)
			{
				if (ParsedCookIssue.IsNumeric())
				{
					int32 ShadersLeft = FCString::Atoi(*ParsedCookIssue);
					GShaderCompilingManager->SetExternalJobs(ShadersLeft);
				}
			}
			return false;
		}
		return true;
	}

	/**
	* Determine if the output is an error we wish to send to the Message Log.
	*
	* @Param UCMDOutput - The current line of output from the UCMD package process.
	**/
	static void ProcessAndHandleCookErrorOutput(FString UCMDOutput)
	{
		FString LhsUCMDOutputMsg, ParsedCookIssue;

		// we don't want to report duplicate warnings/errors to the package results log
		// so, only add messages between the cook start and the summary
		if (UCMDOutput.Contains(TEXT("Warning/Error Summary")))
		{
			bSawSummary = true;
		}

		// note: CookResults:Warning: actually outputs some unhandled errors.
		if (UCMDOutput.Split(TEXT("CookResults:Warning: "), &LhsUCMDOutputMsg, &ParsedCookIssue))
		{
			SyncMessageWithMessageLog(ParsedCookIssue, EMessageSeverity::Warning);
		}
		else if (UCMDOutput.Split(TEXT("CookResults:Error: "), &LhsUCMDOutputMsg, &ParsedCookIssue))
		{
			SyncMessageWithMessageLog(ParsedCookIssue, EMessageSeverity::Error);
		}
		else if (!bSawSummary && UCMDOutput.Split(TEXT("Warning: "), &LhsUCMDOutputMsg, &ParsedCookIssue))
		{
			SyncMessageWithMessageLog(ParsedCookIssue, EMessageSeverity::Warning);
		}
		else if (!bSawSummary && UCMDOutput.Split(TEXT("Error: "), &LhsUCMDOutputMsg, &ParsedCookIssue))
		{
			SyncMessageWithMessageLog(ParsedCookIssue, EMessageSeverity::Error);
		}

	}

	/**
	* Send the UCMD Packaging error message to the Message Log.
	*
	* @Param ErrorCode - The UCMD return code we received and wish to display the error message for.
	**/
	static void SendPackagingErrorToMessageLog(int32 ErrorCode)
	{
		SyncMessageWithMessageLog(FEditorAnalytics::TranslateErrorCode(ErrorCode), EMessageSeverity::Error);
	}

	/**
	* Get Whether the UCMD process returned asset errors via the log
	**/
	static bool GetHasAssetErrors() { return bHasAssetErrors; }

	/**
	* Clear the asset error state for the UCMD process
	**/
	static void ClearAssetErrors() { bHasAssetErrors = false; bSawSummary = false; }

};

bool FPackagingErrorHandler::bHasAssetErrors = false;
bool FPackagingErrorHandler::bSawSummary = false;

DECLARE_CYCLE_STAT(TEXT("Requesting FUCMDHelperModule::HandleUcmdProcessCompleted message dialog to present the error message"), STAT_FUCMDHelperModule_HandleUcmdProcessCompleted_DialogMessage, STATGROUP_TaskGraphTasks);


class  FMonitoredCMDProcess
	: public FMonitoredProcess
{
public:

	FMonitoredCMDProcess(const FString& InURL, const FString& InParams, bool InHidden, bool InCreatePipes)
		: FMonitoredProcess(InURL, InParams, FPaths::RootDir(), InHidden, InCreatePipes)
	{
	}



	virtual bool Launch() override
	{
		if (bIsRunning)
		{
			return false;
		}

		check(Thread == nullptr); // We shouldn't be calling this twice

		if (bCreatePipes && !FPlatformProcess::CreatePipe(ReadPipe, WritePipe))
		{
			return false;
		}

		ProcessHandle = FPlatformProcess::CreateProc(*URL, *Params, false, Hidden, Hidden, nullptr, 0, *WorkingDir, WritePipe);

		if (!ProcessHandle.IsValid())
		{
			return false;
		}

		static std::atomic<uint32> MonitoredProcessIndex{ 0 };
		const FString MonitoredProcessName = FString::Printf(TEXT("FMonitoredCMDProcess %d"), MonitoredProcessIndex.fetch_add(1));

		bIsRunning = true;
		Thread = FRunnableThread::Create(this, *MonitoredProcessName, 128 * 1024, TPri_AboveNormal);
		if (!FPlatformProcess::SupportsMultithreading())
		{
			StartTime = FDateTime::UtcNow();
		}

		return true;
	}
};


class FUCMDHelperModule : public IUCMDHelperModule
{
public:
	FUCMDHelperModule()
	{
	}

	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}

	virtual void CreateUcmdTask(const FString& CommandLine, const FText& PlatformDisplayName, const FText& TaskName, const FText& TaskShortName, const FSlateBrush* TaskIcon, bool PowerShell, UcmdTaskResultCallack ResultCallback, const FString& ResultLocation)
	{

		FString CmdExe = "";
		#if PLATFORM_WINDOWS
				if (PowerShell)
					CmdExe = TEXT("powershell");
				else
					CmdExe = TEXT("cmd.exe");
		#elif PLATFORM_LINUX
				FString CmdExe = TEXT("/bin/bash");
		#else
				FString CmdExe = TEXT("/bin/sh");
		#endif

				FString FullCommandLine = "";
		#if PLATFORM_WINDOWS
				if (PowerShell)
					FullCommandLine = FString::Printf(TEXT("-command \"%s\""), *CommandLine);
				else
					FullCommandLine = FString::Printf(TEXT("/c \"%s\""), *CommandLine);

		#else
				FString FullCommandLine = FString::Printf(TEXT("%s"), *CommandLine);
		#endif


		TSharedPtr<FMonitoredCMDProcess> UcmdProcess = MakeShareable(new FMonitoredCMDProcess(CmdExe, FullCommandLine, true, true));

		FGameProjectGenerationModule& GameProjectModule = FModuleManager::LoadModuleChecked<FGameProjectGenerationModule>(TEXT("GameProjectGeneration"));
		bool bHasCode = GameProjectModule.Get().ProjectHasCodeFiles();

		FPackagingErrorHandler::ClearAssetErrors();

		// create notification item
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("Platform"), PlatformDisplayName);
		Arguments.Add(TEXT("TaskName"), TaskName);
		FText NotificationFormat = (PlatformDisplayName.IsEmpty()) ? LOCTEXT("UcmdTaskInProgressNotificationNoPlatform", "{TaskName}...") : LOCTEXT("UcmdTaskInProgressNotification", "{TaskName} for {Platform}...");
		FNotificationInfo Info(FText::Format(NotificationFormat, Arguments));

		Info.Image = TaskIcon;
		Info.bFireAndForget = false;
		Info.FadeOutDuration = 0.0f;
		Info.ExpireDuration = 0.0f;
		Info.Hyperlink = FSimpleDelegate::CreateStatic(&FUCMDHelperModule::HandleUcmdHyperlinkNavigate);
		Info.HyperlinkText = LOCTEXT("ShowOutputLogHyperlink", "Show Output Log");
		Info.ButtonDetails.Add(
			FNotificationButtonInfo(
				LOCTEXT("UcmdTaskCancel", "Cancel"),
				LOCTEXT("UcmdTaskCancelToolTip", "Cancels execution of this task."),
				FSimpleDelegate::CreateStatic(&FUCMDHelperModule::HandleUcmdCancelButtonClicked, UcmdProcess),
				SNotificationItem::CS_Pending
			)
		);
		Info.ButtonDetails.Add(
			FNotificationButtonInfo(
				LOCTEXT("UcmdTaskDismiss", "Dismiss"),
				FText(),
				FSimpleDelegate::CreateStatic(&FMainFrameActionsNotificationTask::HandleDismissButtonClicked),
				SNotificationItem::CS_Fail
			)
		);

		TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
		TSharedPtr<SNotificationItem> OldNotification = NotificationItemPtr.Pin();
		if (OldNotification.IsValid())
		{
			OldNotification->Fadeout();
		}

		if (!NotificationItem.IsValid())
		{
			return;
		}

		FString EventName = (CommandLine.Contains(TEXT("-package")) ? TEXT("Editor.Package") : TEXT("Editor.Cook"));
		FEditorAnalytics::ReportEvent(EventName + TEXT(".Start"), PlatformDisplayName.ToString(), bHasCode);

		NotificationItem->SetCompletionState(SNotificationItem::CS_Pending);

		// launch the packager
		NotificationItemPtr = NotificationItem;

		EventData Data;
		Data.StartTime = FPlatformTime::Seconds();
		Data.EventName = EventName;
		Data.bProjectHasCode = bHasCode;
		Data.ResultCallback = ResultCallback;
		UcmdProcess->OnCanceled().BindStatic(&FUCMDHelperModule::HandleUcmdProcessCanceled, NotificationItemPtr, PlatformDisplayName, TaskShortName, Data);
		UcmdProcess->OnCompleted().BindStatic(&FUCMDHelperModule::HandleUcmdProcessCompleted, NotificationItemPtr, PlatformDisplayName, TaskShortName, Data, ResultLocation);
		UcmdProcess->OnOutput().BindStatic(&FUCMDHelperModule::HandleUcmdProcessOutput, NotificationItemPtr, PlatformDisplayName, TaskShortName);

		TWeakPtr<FMonitoredCMDProcess> UcmdProcessPtr(UcmdProcess);
		FEditorDelegates::OnShutdownPostPackagesSaved.Add(FSimpleDelegate::CreateStatic(&FUCMDHelperModule::HandleUcmdCancelButtonClicked, UcmdProcessPtr));

		UcmdProcess->Launch();
		GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileStart_Cue.CompileStart_Cue"));
	}

	static void HandleUcmdHyperlinkNavigate()
	{
		FGlobalTabmanager::Get()->TryInvokeTab(FName("OutputLog"));
	}

	static void HandleUcmdResultHyperlinkNavigate(FString ResultLocation)
	{
		if (!ResultLocation.IsEmpty())
		{
			FPlatformProcess::ExploreFolder(*(ResultLocation));
		}
	}

	static void HandleUcmdCancelButtonClicked(TSharedPtr<FMonitoredCMDProcess> PackagerProcess)
	{
		if (PackagerProcess.IsValid())
		{
			PackagerProcess->Cancel(true);
		}
	}

	static void HandleUcmdCancelButtonClicked(TWeakPtr<FMonitoredCMDProcess> PackagerProcessPtr)
	{
		TSharedPtr<FMonitoredCMDProcess> PackagerProcess = PackagerProcessPtr.Pin();
		if (PackagerProcess.IsValid())
		{
			PackagerProcess->Cancel(true);
		}
	}

	static void HandleUcmdProcessCanceled(TWeakPtr<SNotificationItem> NotificationItemPtr, FText PlatformDisplayName, FText TaskName, EventData Event)
	{
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("Platform"), PlatformDisplayName);
		Arguments.Add(TEXT("TaskName"), TaskName);

		TGraphTask<FMainFrameActionsNotificationTask>::CreateTask().ConstructAndDispatchWhenReady(
			NotificationItemPtr,
			SNotificationItem::CS_Fail,
			FText::Format(LOCTEXT("UcmdProcessFailedNotification", "{TaskName} canceled!"), Arguments)
		);

		TArray<FAnalyticsEventAttribute> ParamArray;
		const double TimeSec = FPlatformTime::Seconds() - Event.StartTime;
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("Time"), TimeSec));
		FEditorAnalytics::ReportEvent(Event.EventName + TEXT(".Canceled"), PlatformDisplayName.ToString(), Event.bProjectHasCode, ParamArray);
		if (Event.ResultCallback)
		{
			Event.ResultCallback(TEXT("Canceled"), TimeSec);
		}
		if (GShaderCompilingManager)
		{
			GShaderCompilingManager->SetExternalJobs(0);
		}
		//	FMessageLog("PackagingResults").Warning(FText::Format(LOCTEXT("UcmdProcessCanceledMessageLog", "{TaskName} for {Platform} canceled by user"), Arguments));
	}

	static void HandleUcmdProcessCompleted(int32 ReturnCode, TWeakPtr<SNotificationItem> NotificationItemPtr, FText PlatformDisplayName, FText TaskName, EventData Event, FString ResultLocation)
	{
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("Platform"), PlatformDisplayName);
		Arguments.Add(TEXT("TaskName"), TaskName);
		const double TimeSec = FPlatformTime::Seconds() - Event.StartTime;

		if (ReturnCode == 0)
		{
			if (!ResultLocation.IsEmpty())
			{
				if (TSharedPtr<SNotificationItem> SharedNotificationItemPtr = NotificationItemPtr.Pin())
				{
					SharedNotificationItemPtr->SetHyperlink(FSimpleDelegate::CreateStatic(&FUCMDHelperModule::HandleUcmdResultHyperlinkNavigate, ResultLocation), LOCTEXT("ShowOutputLocation", "Show in Explorer"));
				}

			}

			TGraphTask<FMainFrameActionsNotificationTask>::CreateTask().ConstructAndDispatchWhenReady(
				NotificationItemPtr,
				SNotificationItem::CS_Success,
				FText::Format(LOCTEXT("UcmdProcessSucceededNotification", "{TaskName} complete!"), Arguments)
			);

			TArray<FAnalyticsEventAttribute> ParamArray;
			ParamArray.Add(FAnalyticsEventAttribute(TEXT("Time"), TimeSec));
			FEditorAnalytics::ReportEvent(Event.EventName + TEXT(".Completed"), PlatformDisplayName.ToString(), Event.bProjectHasCode, ParamArray);
			if (Event.ResultCallback)
			{
				Event.ResultCallback(TEXT("Completed"), TimeSec);
			}

			//		FMessageLog("PackagingResults").Info(FText::Format(LOCTEXT("UcmdProcessSuccessMessageLog", "{TaskName} for {Platform} completed successfully"), Arguments));
		}
		else
		{
			TGraphTask<FMainFrameActionsNotificationTask>::CreateTask().ConstructAndDispatchWhenReady(
				NotificationItemPtr,
				SNotificationItem::CS_Fail,
				FText::Format(LOCTEXT("PackagerFailedNotification", "{TaskName} failed!"), Arguments),
				FPackagingErrorHandler::GetHasAssetErrors() ? LOCTEXT("ShowResultsLogHyperlink", "Show Results Log") : FText(),
				false);

			TArray<FAnalyticsEventAttribute> ParamArray;
			ParamArray.Add(FAnalyticsEventAttribute(TEXT("Time"), TimeSec));
			FEditorAnalytics::ReportEvent(Event.EventName + TEXT(".Failed"), PlatformDisplayName.ToString(), Event.bProjectHasCode, ReturnCode, ParamArray);
			if (Event.ResultCallback)
			{
				Event.ResultCallback(TEXT("Failed"), TimeSec);
			}

			// Send the error to the Message Log.
			if (TaskName.EqualTo(LOCTEXT("PackagingTaskName", "Packaging")))
			{
				FPackagingErrorHandler::SendPackagingErrorToMessageLog(ReturnCode);
			}

			// Present a message dialog if we want the error message to be prominent.
			if (FEditorAnalytics::ShouldElevateMessageThroughDialog(ReturnCode))
			{
				FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
					FSimpleDelegateGraphTask::FDelegate::CreateLambda([=]() {
						FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FEditorAnalytics::TranslateErrorCode(ReturnCode)));
						}),
					GET_STATID(STAT_FUCMDHelperModule_HandleUcmdProcessCompleted_DialogMessage),
							nullptr,
							ENamedThreads::GameThread
							);
			}

			//		FMessageLog("PackagingResults").Info(FText::Format(LOCTEXT("UcmdProcessFailedMessageLog", "{TaskName} for {Platform} failed"), Arguments));
		}
		if (GShaderCompilingManager)
		{
			GShaderCompilingManager->SetExternalJobs(0);
		}
	}

	static void HandleUcmdProcessOutput(FString Output, TWeakPtr<SNotificationItem> NotificationItemPtr, FText PlatformDisplayName, FText TaskName)
	{
		if (!Output.IsEmpty() && !Output.Equals("\r"))
		{
			bool bDisplayLog = true;
			if (TaskName.EqualTo(LOCTEXT("PackagingTaskName", "Packaging")))
			{
				// Deal with any cook messages that may have been encountered.
				bDisplayLog = FPackagingErrorHandler::ProcessAndHandleCookMessageOutput(Output);
			}
			if (bDisplayLog)
			{
				UE_LOG(UCMDHelper, Log, TEXT("%s (%s): %s"), *TaskName.ToString(), *PlatformDisplayName.ToString(), *Output);
			}

			if (TaskName.EqualTo(LOCTEXT("PackagingTaskName", "Packaging")))
			{
				// Deal with any cook errors that may have been encountered.
				FPackagingErrorHandler::ProcessAndHandleCookErrorOutput(Output);
			}
			if (TaskName.EqualTo(LOCTEXT("CookingTaskName", "Cooking")))
			{
				// Deal with any cook errors that may have been encountered.
				FPackagingErrorHandler::ProcessAndHandleCookErrorOutput(Output);
			}
		}
	}

	static void HandleUcmdLaunchFailed(TWeakPtr<SNotificationItem> NotificationItemPtr, FText PlatformDisplayName, FText TaskName, EventData Event)
	{
		GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));

		TGraphTask<FMainFrameActionsNotificationTask>::CreateTask().ConstructAndDispatchWhenReady(
			NotificationItemPtr,
			SNotificationItem::CS_Fail,
			LOCTEXT("UcmdLaunchFailedNotification", "Failed to launch Unreal Automation Tool (UCMD)!")
		);

		TArray<FAnalyticsEventAttribute> ParamArray;
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("Time"), 0.0));
		FEditorAnalytics::ReportEvent(Event.EventName + TEXT(".Failed"), PlatformDisplayName.ToString(), Event.bProjectHasCode, EAnalyticsErrorCodes::UATLaunchFailure, ParamArray);
		if (Event.ResultCallback)
		{
			Event.ResultCallback(TEXT("FailedToStart"), 0.0f);
		}
	}

private:
	TWeakPtr<SNotificationItem> NotificationItemPtr;

};

IMPLEMENT_MODULE(FUCMDHelperModule, UCMDHelper)


#undef LOCTEXT_NAMESPACE