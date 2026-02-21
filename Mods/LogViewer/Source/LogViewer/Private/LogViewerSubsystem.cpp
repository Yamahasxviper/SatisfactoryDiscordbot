// Copyright Coffee Stain Studios. All Rights Reserved.

#include "LogViewerSubsystem.h"

#include "HttpServerModule.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"
#include "IHttpRouter.h"
#include "HttpPath.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// ─────────────────────────────────────────────────────────────────────────────
// USubsystem lifetime
// ─────────────────────────────────────────────────────────────────────────────

bool ULogViewerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Only create this subsystem on dedicated servers.
	return IsRunningDedicatedServer();
}

void ULogViewerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Config = FLogViewerConfig::LoadOrCreate();

	FHttpServerModule* HttpServerModule =
		FModuleManager::LoadModulePtr<FHttpServerModule>("HTTPServer");

	if (!HttpServerModule)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("LogViewer: HTTPServer module is not available. Log viewer will not start."));
		return;
	}

	TSharedPtr<IHttpRouter> Router = HttpServerModule->GetHttpRouter(Config.Port);
	if (!Router.IsValid())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("LogViewer: Failed to create HTTP router on port %d."), Config.Port);
		return;
	}

	// Bind  GET /logs  →  HTML page
	HtmlRouteHandle = Router->BindRoute(
		FHttpPath(TEXT("/logs")),
		EHttpServerRequestVerbs::VERB_GET,
		[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) -> bool
		{
			return HandleHtmlRequest(Request, OnComplete);
		});

	// Bind  GET /logs/raw  →  plain text
	RawRouteHandle = Router->BindRoute(
		FHttpPath(TEXT("/logs/raw")),
		EHttpServerRequestVerbs::VERB_GET,
		[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) -> bool
		{
			return HandleRawRequest(Request, OnComplete);
		});

	HttpServerModule->StartAllListeners();

	if (Config.AuthToken.IsEmpty())
	{
		UE_LOG(LogTemp, Log,
		       TEXT("LogViewer: HTTP log viewer started on port %d. "
		            "Browse to http://<server-ip>:%d/logs"),
		       Config.Port, Config.Port);
	}
	else
	{
		UE_LOG(LogTemp, Log,
		       TEXT("LogViewer: HTTP log viewer started on port %d (auth token required). "
		            "Browse to http://<server-ip>:%d/logs?token=<AuthToken>"),
		       Config.Port, Config.Port);
	}
}

void ULogViewerSubsystem::Deinitialize()
{
	// Only unbind our own routes – never call StopAllListeners() as it would
	// shut down the Satisfactory Server API and any other HTTP listeners.
	FHttpServerModule* HttpServerModule =
		FModuleManager::GetModulePtr<FHttpServerModule>("HTTPServer");

	if (HttpServerModule)
	{
		TSharedPtr<IHttpRouter> Router = HttpServerModule->GetHttpRouter(Config.Port);
		if (Router.IsValid())
		{
			if (HtmlRouteHandle.IsValid())
			{
				Router->UnbindRoute(HtmlRouteHandle);
				HtmlRouteHandle.Reset();
			}
			if (RawRouteHandle.IsValid())
			{
				Router->UnbindRoute(RawRouteHandle);
				RawRouteHandle.Reset();
			}
		}
	}

	Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// HTTP route handlers
// ─────────────────────────────────────────────────────────────────────────────

bool ULogViewerSubsystem::HandleHtmlRequest(const FHttpServerRequest& Request,
                                             const FLogViewerHttpResultCallback& OnComplete)
{
	if (!IsAuthorized(Request))
	{
		TUniquePtr<FHttpServerResponse> Response =
			FHttpServerResponse::Error(EHttpServerResponseCodes::Denied,
			                           TEXT("text/plain"),
			                           TEXT("401 Unauthorized: missing or invalid token."));
		OnComplete(MoveTemp(Response));
		return true;
	}

	FString Excerpt;
	FString Error;
	ReadLogExcerpt(Excerpt, Error);

	const FString Html = BuildHtmlPage(Excerpt, Error);

	TUniquePtr<FHttpServerResponse> Response =
		FHttpServerResponse::Create(Html, TEXT("text/html; charset=utf-8"));
	OnComplete(MoveTemp(Response));
	return true;
}

bool ULogViewerSubsystem::HandleRawRequest(const FHttpServerRequest& Request,
                                            const FLogViewerHttpResultCallback& OnComplete)
{
	if (!IsAuthorized(Request))
	{
		TUniquePtr<FHttpServerResponse> Response =
			FHttpServerResponse::Error(EHttpServerResponseCodes::Denied,
			                           TEXT("text/plain"),
			                           TEXT("401 Unauthorized: missing or invalid token."));
		OnComplete(MoveTemp(Response));
		return true;
	}

	FString Excerpt;
	FString Error;
	ReadLogExcerpt(Excerpt, Error);

	const FString Body = Error.IsEmpty()
		? Excerpt
		: FString::Printf(TEXT("Error reading log: %s"), *Error);

	TUniquePtr<FHttpServerResponse> Response =
		FHttpServerResponse::Create(Body, TEXT("text/plain; charset=utf-8"));
	OnComplete(MoveTemp(Response));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

bool ULogViewerSubsystem::IsAuthorized(const FHttpServerRequest& Request) const
{
	if (Config.AuthToken.IsEmpty())
	{
		return true; // No auth required.
	}

	const FString* TokenParam = Request.QueryParams.Find(TEXT("token"));
	return TokenParam && *TokenParam == Config.AuthToken;
}

bool ULogViewerSubsystem::ReadLogExcerpt(FString& OutExcerpt, FString& OutError) const
{
	const FString LogFilePath = FPaths::ProjectLogDir() / TEXT("FactoryGame.log");

	FString LogContent;
	if (!FFileHelper::LoadFileToString(LogContent, *LogFilePath))
	{
		OutError = FString::Printf(
			TEXT("Could not read log file at '%s'. "
			     "The file may not exist yet or the server may not have write "
			     "permission to that directory."), *LogFilePath);
		UE_LOG(LogTemp, Warning, TEXT("LogViewer: %s"), *OutError);
		return false;
	}

	if (Config.LogLineCount <= 0)
	{
		// Return the entire log.
		OutExcerpt = MoveTemp(LogContent);
		return true;
	}

	TArray<FString> Lines;
	LogContent.ParseIntoArrayLines(Lines, /*bCullEmpty=*/false);

	const int32 TotalLines  = Lines.Num();
	const int32 LinesToTake = FMath::Min(Config.LogLineCount, TotalLines);
	const int32 StartIndex  = TotalLines - LinesToTake;

	for (int32 i = StartIndex; i < TotalLines; ++i)
	{
		OutExcerpt += Lines[i];
		OutExcerpt += TEXT("\n");
	}

	return true;
}

FString ULogViewerSubsystem::BuildHtmlPage(const FString& LogExcerpt,
                                            const FString& Error) const
{
	// Escape HTML entities so log content cannot inject markup.
	auto HtmlEscape = [](const FString& Raw) -> FString
	{
		FString Out = Raw;
		Out.ReplaceInline(TEXT("&"),  TEXT("&amp;"),  ESearchCase::CaseSensitive);
		Out.ReplaceInline(TEXT("<"),  TEXT("&lt;"),   ESearchCase::CaseSensitive);
		Out.ReplaceInline(TEXT(">"),  TEXT("&gt;"),   ESearchCase::CaseSensitive);
		Out.ReplaceInline(TEXT("\""), TEXT("&quot;"), ESearchCase::CaseSensitive);
		return Out;
	};

	const FString LogFilePath = FPaths::ProjectLogDir() / TEXT("FactoryGame.log");
	const FString EscapedPath = HtmlEscape(LogFilePath);

	FString BodyContent;
	if (!Error.IsEmpty())
	{
		BodyContent = FString::Printf(
			TEXT("<p class=\"err\">&#9888; %s</p>"), *HtmlEscape(Error));
	}
	else
	{
		const int32 LineCount = Config.LogLineCount > 0 ? Config.LogLineCount : 0;
		const FString Subtitle = LineCount > 0
			? FString::Printf(TEXT("Last %d lines of"), LineCount)
			: TEXT("Full contents of");

		BodyContent = FString::Printf(
			TEXT("<p>%s <code>%s</code> &mdash; auto-refreshes every 30&nbsp;s.</p>\n"
			     "<pre>%s</pre>"),
			*Subtitle, *EscapedPath, *HtmlEscape(LogExcerpt));
	}

	return FString::Printf(TEXT(R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta http-equiv="refresh" content="30">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Satisfactory Server Log</title>
  <style>
    *{box-sizing:border-box;margin:0;padding:0}
    body{background:#0d1117;color:#c9d1d9;font-family:ui-monospace,SFMono-Regular,Menlo,monospace;font-size:13px;padding:1rem}
    h1{color:#58a6ff;font-size:1.4rem;margin-bottom:.5rem}
    p{color:#8b949e;margin-bottom:.75rem;font-family:sans-serif;font-size:.9rem}
    code{background:#161b22;padding:.1em .4em;border-radius:4px;font-family:inherit}
    pre{background:#161b22;border:1px solid #30363d;border-radius:6px;padding:1rem;
        overflow-x:auto;white-space:pre-wrap;word-break:break-all;line-height:1.5}
    .err{color:#f85149;font-family:sans-serif;font-size:.9rem}
    nav{margin-bottom:1rem}
    nav a{color:#58a6ff;text-decoration:none;margin-right:1rem;font-family:sans-serif;font-size:.85rem}
    nav a:hover{text-decoration:underline}
  </style>
</head>
<body>
  <h1>&#128196; Satisfactory Server Log</h1>
  <nav>
    <a href="logs">&#128260; Refresh</a>
    <a href="logs/raw">&#128462; Raw text</a>
  </nav>
  %s
</body>
</html>)"),
		*BodyContent);
}
