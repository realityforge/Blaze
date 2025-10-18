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
#include "Blaze/Actions/AsyncAction_PushContentToLayer.h"
#include "Blaze/BlazeFunctionLibrary.h"
#include "Blaze/BlazePrimaryLayout.h"
#include "Engine/Engine.h"
#include "Engine/StreamableManager.h"
#include "UObject/Stack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_PushContentToLayer)

UAsyncAction_PushContentToLayer*
UAsyncAction_PushContentToLayer::PushContentToLayerAsync(APlayerController* PlayerController,
                                                         const FGameplayTag LayerName,
                                                         const TSoftClassPtr<UCommonActivatableWidget> WidgetClass,
                                                         const bool bSuspendInputUntilComplete)
{
    if (!PlayerController)
    {
#if WITH_EDITOR
        FFrame::KismetExecutionMessage(TEXT("PushContentToLayerAsync was supplied a null PlayerController"),
                                       ELogVerbosity::Error);
#else
        UE_LOGFMT(LogBlaze, Error, "PushContentToLayerAsync was supplied a null PlayerController");
#endif

        return nullptr;
    }
    else if (WidgetClass.IsNull())
    {
#if WITH_EDITOR
        FFrame::KismetExecutionMessage(TEXT("PushContentToLayerAsync was supplied a null WidgetClass"),
                                       ELogVerbosity::Error);
#else
        UE_LOGFMT(LogBlaze, Error, "PushContentToLayerAsync was supplied a null WidgetClass");
#endif
        return nullptr;
    }
    else if (!LayerName.IsValid())
    {
#if WITH_EDITOR
        FFrame::KismetExecutionMessage(TEXT("PushContentToLayerAsync was supplied an invalid LayerName"),
                                       ELogVerbosity::Error);
#else
        UE_LOGFMT(LogBlaze, Error, "PushContentToLayerAsync was supplied an invalid LayerName");
#endif
        return nullptr;
    }
    else if (const auto World =
                 GEngine->GetWorldFromContextObject(PlayerController, EGetWorldErrorMode::LogAndReturnNull))
    {
        const auto Action = NewObject<UAsyncAction_PushContentToLayer>();
        Action->PlayerController = PlayerController;
        Action->LayerName = LayerName;
        Action->WidgetClass = WidgetClass;
        Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
        Action->RegisterWithGameInstance(World);
        return Action;
    }
    else
    {
        return nullptr;
    }
}

void UAsyncAction_PushContentToLayer::Cancel()
{
    Super::Cancel();

    if (Handle.IsValid())
    {
        Handle->CancelHandle();
        Handle.Reset();
    }
}

void UAsyncAction_PushContentToLayer::Activate()
{
    if (const auto Layout = UBlazeFunctionLibrary::GetPrimaryLayout(PlayerController.Get()))
    {
        TWeakObjectPtr Self(this);

        Handle = Layout->PushWidgetToLayerAsync<UCommonActivatableWidget>(
            LayerName,
            bSuspendInputUntilComplete,
            WidgetClass,
            [Self](auto State, auto Widget) {
                if (Self.IsValid())
                {
                    if (EBlazePushWidgetToLayerState::Initialize == State)
                    {
                        Self->OnInitialize.Broadcast(Widget);
                    }
                    else if (EBlazePushWidgetToLayerState::AfterPush == State)
                    {
                        Self->AfterPush.Broadcast(Widget);
                        Self->SetReadyToDestroy();
                    }
                    else if (EBlazePushWidgetToLayerState::Canceled == State)
                    {
                        Self->OnCancelled.Broadcast(Widget);
                        Self->SetReadyToDestroy();
                    }
                }
            });
    }
    else
    {
        SetReadyToDestroy();
    }
}
