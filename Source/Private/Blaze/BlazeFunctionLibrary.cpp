/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Blaze/BlazeFunctionLibrary.h"
#include "Blaze/BlazeLogging.h"
#include "Blaze/BlazePrimaryLayout.h"
#include "Blaze/BlazePrimaryLayoutManager.h"
#include "Blaze/BlazeSubsystem.h"
#include "CommonActivatableWidget.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlazeFunctionLibrary)

// Running counter so we can create a unique name for a SuspendToken.
// See SuspendInputForPlayer
static int32 InputSuspensions{ 0 };

static ULocalPlayer* GetLocalPlayerFromController(const APlayerController* PlayerController)
{
    return PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
}

UCommonActivatableWidget*
UBlazeFunctionLibrary::PushContentToLayer(const ULocalPlayer* LocalPlayer,
                                          const FGameplayTag LayerName,
                                          const TSoftClassPtr<UCommonActivatableWidget> WidgetClass,
                                          const bool bAsync)
{
    if (!LocalPlayer || WidgetClass.IsNull() || !LayerName.IsValid())
    {
        UE_LOGFMT(LogBlaze,
                  Error,
                  "PushContentToLayer{Type}"
                  "(LocalPlayer=[{LocalPlayer}] LayerName=[{LayerName}] WidgetClass=[{WidgetClass}]) "
                  "failed due to invalid parameters",
                  bAsync ? "Async" : "",
                  GetNameSafe(LocalPlayer),
                  LayerName.GetTagName(),
                  WidgetClass.ToString());
        return nullptr;
    }
    else if (const auto Layout = GetPrimaryLayout(LocalPlayer))
    {
        UE_LOGFMT(LogBlaze,
                  Log,
                  "PushContentToLayer{Type}"
                  "(LocalPlayer=[{LocalPlayer}] LayerName=[{LayerName}] WidgetClass=[{WidgetClass}])",
                  bAsync ? "Async" : "",
                  GetNameSafe(LocalPlayer),
                  LayerName.GetTagName(),
                  WidgetClass.ToString());
        if (bAsync)
        {
            Layout->PushWidgetToLayerAsync(LayerName, true, WidgetClass);
            return nullptr;
        }
        else if (const auto LoadedClass = WidgetClass.LoadSynchronous())
        {
            return Layout->PushWidgetToLayer(LayerName, TSubclassOf<UCommonActivatableWidget>(LoadedClass));
        }
        else
        {
            UE_LOGFMT(LogBlaze,
                      Error,
                      "PushContentToLayer{Type}"
                      "(LocalPlayer=[{LocalPlayer}] LayerName=[{LayerName}] WidgetClass=[{WidgetClass}]) "
                      "failed as LocalPlayer has no PrimaryLayout",
                      bAsync ? "Async" : "",
                      GetNameSafe(LocalPlayer),
                      LayerName.GetTagName(),
                      WidgetClass.ToString());
            return nullptr;
        }
    }
    else
    {
        UE_LOGFMT(LogBlaze,
                  Error,
                  "PushContentToLayer{Type}"
                  "(LocalPlayer=[{LocalPlayer}] LayerName=[{LayerName}] WidgetClass=[{WidgetClass}]) "
                  "failed as LocalPlayer has no PrimaryLayout",
                  bAsync ? "Async" : "",
                  GetNameSafe(LocalPlayer),
                  LayerName.GetTagName(),
                  WidgetClass.ToString());
        return nullptr;
    }
}

UBlazePrimaryLayoutManager* UBlazeFunctionLibrary::GetPrimaryLayoutManager(const UObject* WorldContextObject)
{
    if (const auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        const auto GameInstance = World->GetGameInstance();
        return nullptr != GameInstance ? GetPrimaryLayoutManager(GameInstance) : nullptr;
    }
    else
    {
        return nullptr;
    }
}

UBlazePrimaryLayoutManager* UBlazeFunctionLibrary::GetPrimaryLayoutManager(const UGameInstance* GameInstance)
{
    const auto Subsystem = UGameInstance::GetSubsystem<UBlazeSubsystem>(GameInstance);
    return Subsystem ? Subsystem->GetPrimaryLayoutManager() : nullptr;
}

UBlazePrimaryLayout* UBlazeFunctionLibrary::GetPrimaryLayout(const APlayerController* PlayerController)
{
    return PlayerController ? GetPrimaryLayout(Cast<ULocalPlayer>(PlayerController->Player)) : nullptr;
}

UBlazePrimaryLayout* UBlazeFunctionLibrary::GetPrimaryLayout(const ULocalPlayer* LocalPlayer)
{
    const auto GameInstance = LocalPlayer ? LocalPlayer->GetGameInstance() : nullptr;
    const auto Manager = GameInstance ? GetPrimaryLayoutManager(GameInstance) : nullptr;
    return Manager ? Manager->GetPrimaryLayout(LocalPlayer) : nullptr;
}

UCommonActivatableWidget*
UBlazeFunctionLibrary::PushContentToLayer(APlayerController* PlayerController,
                                          const FGameplayTag LayerName,
                                          const TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
    return PushContentToLayer(GetLocalPlayerFromController(PlayerController), LayerName, WidgetClass);
}

UCommonActivatableWidget*
UBlazeFunctionLibrary::PushContentToLayer(const ULocalPlayer* LocalPlayer,
                                          const FGameplayTag LayerName,
                                          const TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
    return PushContentToLayer(LocalPlayer, LayerName, TSoftClassPtr<UCommonActivatableWidget>(WidgetClass), false);
}

void UBlazeFunctionLibrary::PushContentToLayerAsync(const ULocalPlayer* LocalPlayer,
                                                    const FGameplayTag LayerName,
                                                    const TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
    PushContentToLayer(LocalPlayer, LayerName, WidgetClass, true);
}

void UBlazeFunctionLibrary::PopContentFromLayer(const FGameplayTag LayerName,
                                                UCommonActivatableWidget* ActivatableWidget)
{
    if (ActivatableWidget)
    {
        if (const auto LocalPlayer = ActivatableWidget->GetOwningLocalPlayer())
        {
            if (const auto Layout = GetPrimaryLayout(LocalPlayer))
            {
                Layout->RemoveWidgetFromLayer(LayerName, ActivatableWidget);
            }
            else
            {
                UE_LOGFMT(LogBlaze,
                          Display,
                          "PopContentFromLayer(ActivatableWidget=[{ActivatableWidget}]) "
                          "failed as LocalPlayer=[{LocalPlayer}] has no PrimaryLayout",
                          GetNameSafe(ActivatableWidget),
                          GetNameSafe(LocalPlayer));
            }
        }
        else
        {
            UE_LOGFMT(LogBlaze,
                      Error,
                      "PopContentFromLayer(ActivatableWidget=[{ActivatableWidget}]) "
                      "failed as widget has no OwningLocalPlayer.",
                      GetNameSafe(ActivatableWidget));
        }
    }
    else
    {
        UE_LOGFMT(LogBlaze,
                  Error,
                  "PopContentFromLayer(ActivatableWidget=[{ActivatableWidget}]) "
                  "failed as ActivatableWidget invalid",
                  GetNameSafe(ActivatableWidget));
    }
}

FName UBlazeFunctionLibrary::SuspendInputForPlayer(const APlayerController* PlayerController,
                                                   const FName SuspendReasonBase)
{
    return SuspendInputForPlayer(GetLocalPlayerFromController(PlayerController), SuspendReasonBase);
}

FName UBlazeFunctionLibrary::SuspendInputForPlayer(const ULocalPlayer* LocalPlayer, const FName SuspendReasonBase)
{
    if (const auto CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
    {
        // FNames in unreal are optimized to store both a string part (a.k.a. "base name") and a
        // number part (an integer suffix). So "MyName_1" is stored internally as base="MyName", Number=1
        // Special handling avoids string concatenation and associated memory and processing bloat. This is just
        // an efficient way of creating a unique name per input suspension
        InputSuspensions++;
        FName SuspendToken{ SuspendReasonBase };
        SuspendToken.SetNumber(InputSuspensions);

        CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, true);
        CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, true);
        CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, true);

        return SuspendToken;
    }
    else
    {
        UE_LOGFMT(LogBlaze,
                  Warning,
                  "SuspendInputForPlayer((LocalPlayer=[{LocalPlayer}] SuspendReasonBase=[{SuspendReasonBase}]) "
                  "failed as unable to locate CommonInputSubsystem",
                  GetNameSafe(LocalPlayer),
                  SuspendReasonBase);
        return NAME_None;
    }
}

void UBlazeFunctionLibrary::ResumeInputForPlayer(const APlayerController* PlayerController, const FName SuspendToken)
{
    ResumeInputForPlayer(GetLocalPlayerFromController(PlayerController), SuspendToken);
}

void UBlazeFunctionLibrary::ResumeInputForPlayer(const ULocalPlayer* LocalPlayer, const FName SuspendToken)
{
    if (NAME_None != SuspendToken)
    {
        if (const auto CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
        {
            CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, false);
            CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, false);
            CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, false);
        }
        else
        {
            UE_LOGFMT(LogBlaze,
                      Warning,
                      "ResumeInputForPlayer((LocalPlayer=[{LocalPlayer}] SuspendToken=[{SuspendToken}]) "
                      "failed as unable to locate CommonInputSubsystem",
                      GetNameSafe(LocalPlayer),
                      SuspendToken);
        }
    }
    else
    {
        UE_LOGFMT(LogBlaze,
                  Warning,
                  "ResumeInputForPlayer((LocalPlayer=[{LocalPlayer}] SuspendToken=[{SuspendToken}]) "
                  "ignored as SuspendToken is invalid",
                  GetNameSafe(LocalPlayer),
                  SuspendToken);
    }
}
