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
#include "Blaze/Actions/AsyncAction_CreateWidgetAsync.h"
#include "Blaze/BlazeFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "UObject/Stack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_CreateWidgetAsync)

UAsyncAction_CreateWidgetAsync*
UAsyncAction_CreateWidgetAsync::CreateWidgetAsync(UObject* WorldContextObject,
                                                  const TSoftClassPtr<UUserWidget> WidgetClass,
                                                  APlayerController* OwningPlayer,
                                                  const bool bSuspendInputUntilComplete)
{
    if (WidgetClass.IsNull())
    {
#if WITH_EDITOR
        FFrame::KismetExecutionMessage(TEXT("CreateWidgetAsync was passed an invalid WidgetClass"),
                                       ELogVerbosity::Error);
#else
        UE_LOGFMT(LogBlaze, Error, "CreateWidgetAsync was passed an invalid WidgetClass");
#endif
        return nullptr;
    }
    else if (!OwningPlayer || !IsValid(OwningPlayer))
    {
#if WITH_EDITOR
        FFrame::KismetExecutionMessage(TEXT("CreateWidgetAsync was passed an invalid OwningPlayer"),
                                       ELogVerbosity::Error);
#else
        UE_LOGFMT(LogBlaze, Error, "CreateWidgetAsync was passed an invalid OwningPlayer");
#endif
        return nullptr;
    }
    else
    {
        const auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

        const auto Action = NewObject<UAsyncAction_CreateWidgetAsync>();
        Action->WidgetClass = WidgetClass;
        Action->OwningPlayer = OwningPlayer;
        Action->World = World;
        Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
        Action->RegisterWithGameInstance(World);

        return Action;
    }
}

void UAsyncAction_CreateWidgetAsync::Cancel()
{
    Super::Cancel();

    if (Handle.IsValid())
    {
        Handle->CancelHandle();
        Handle.Reset();
    }
}

void UAsyncAction_CreateWidgetAsync::Activate()
{
    static const FName NAME_CreateWidgetAsync = FName("CreatingWidgetAsync");
    const auto PlayerController = OwningPlayer.Get();
    const auto SuspendInputToken = bSuspendInputUntilComplete
        ? UBlazeFunctionLibrary::SuspendInputForPlayer(PlayerController, NAME_CreateWidgetAsync)
        : NAME_None;

    TWeakObjectPtr Self(this);

    Handle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
        WidgetClass.ToSoftObjectPath(),
        FStreamableDelegate::CreateWeakLambda(
            this,
            [Self, PlayerController, SuspendInputToken] {
                if (PlayerController)
                {
                    UBlazeFunctionLibrary::ResumeInputForPlayer(PlayerController, SuspendInputToken);
                }
                if (Self.IsValid())
                {
                    if (const auto ResolvedClass = Self->WidgetClass.Get())
                    {
                        // This next line deliberately re-resolves OwningPlayer
                        // ... which will return null if no longer valid
                        const auto Widget =
                            UWidgetBlueprintLibrary::Create(Self->World.Get(), ResolvedClass, Self->OwningPlayer.Get());
                        Self->OnComplete.Broadcast(Widget);
                    }
                    else
                    {
                        Self->OnCancelled.Broadcast();
                    }
                    Self->SetReadyToDestroy();
                }
            }),
        FStreamableManager::AsyncLoadHighPriority);

    Handle->BindCancelDelegate(
        FStreamableDelegate::CreateWeakLambda(this, [Self, PlayerController, SuspendInputToken]() {
            if (PlayerController)
            {
                UBlazeFunctionLibrary::ResumeInputForPlayer(PlayerController, SuspendInputToken);
            }
            if (Self.IsValid())
            {
                Self->OnCancelled.Broadcast();
            }
        }));
}
