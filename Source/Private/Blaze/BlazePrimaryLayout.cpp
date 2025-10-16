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
#include "Blaze/BlazePrimaryLayout.h"
#include "Blaze/BlazeFunctionLibrary.h"
#include "Blaze/BlazeLogging.h"
#include "CommonActivatableWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlazePrimaryLayout)

UBlazePrimaryLayout::UBlazePrimaryLayout(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}

void UBlazePrimaryLayout::BP_RegisterLayer(const FGameplayTag LayerTag,
                                           UCommonActivatableWidgetContainerBase* LayerWidget)
{
    if (!LayerTag.IsValid())
    {
#if WITH_EDITOR
        FFrame::KismetExecutionMessage(TEXT("BP_RegisterLayer was supplied an invalid LayerName"),
                                       ELogVerbosity::Error);
#else
        UE_LOGFMT(LogBlaze, Error, "BP_RegisterLayer was supplied an invalid LayerName");
#endif
    }
    else if (!IsValid(LayerWidget))
    {
#if WITH_EDITOR
        FFrame::KismetExecutionMessage(TEXT("BP_RegisterLayer was supplied an invalid LayerWidget"),
                                       ELogVerbosity::Error);
#else
        UE_LOGFMT(LogBlaze, Error, "BP_RegisterLayer was supplied an invalid LayerWidget");
#endif
    }
    else if (Layers.Contains(LayerTag))
    {
#if WITH_EDITOR
        FFrame::KismetExecutionMessage(TEXT("BP_RegisterLayer attempted to register a Layer with "
                                            "a name that already exists"),
                                       ELogVerbosity::Error);
#else
        UE_LOGFMT(LogBlaze,
                  Error,
                  "BP_RegisterLayer attempted to register a Layer with "
                  "a name that already exists");
#endif
    }
    else
    {
        RegisterLayer(LayerTag, LayerWidget);
    }
}

void UBlazePrimaryLayout::RegisterLayer(const FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
    // Avoid attempting to add widgets during designer as it would make it
    // hard to design in the editor if layers were being added
    if (!IsDesignTime())
    {
        if (ensureAlways(LayerWidget) && ensureAlways(LayerTag.IsValid()) && ensureAlways(!Layers.Contains(LayerTag)))
        {
            LayerWidget->SetTransitionDuration(0.0);
            Layers.Add(LayerTag, LayerWidget);
        }
    }
}

TSharedPtr<FStreamableHandle> UBlazePrimaryLayout::PushWidgetToLayerStackAsync_Internal(
    const FGameplayTag& LayerName,
    const bool bSuspendInputUntilComplete,
    const TSoftClassPtr<UCommonActivatableWidget>& WidgetClass,
    TFunction<void(EBlazePushWidgetToLayerState, UCommonActivatableWidget*)> CallbackFunc)
{
    static const FName NAME_PushWidgetToLayer("PushWidgetToLayer");
    const auto SuspendInputToken = bSuspendInputUntilComplete
        ? UBlazeFunctionLibrary::SuspendInputForPlayer(GetOwningPlayer(), NAME_PushWidgetToLayer)
        : NAME_None;

    auto Handle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
        WidgetClass.ToSoftObjectPath(),
        FStreamableDelegate::CreateWeakLambda(this, [this, LayerName, WidgetClass, CallbackFunc, SuspendInputToken] {
            if (const auto ResolvedClass = WidgetClass.Get())
            {
                UBlazeFunctionLibrary::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);
                const TFunctionRef<void(UCommonActivatableWidget&)> Func = [CallbackFunc](auto& WidgetToInit) {
                    CallbackFunc(EBlazePushWidgetToLayerState::Initialize, &WidgetToInit);
                };
                if (const auto Widget = PushWidgetToLayer<UCommonActivatableWidget>(LayerName, ResolvedClass, Func))
                {
                    CallbackFunc(EBlazePushWidgetToLayerState::AfterPush, Widget);
                }
                else
                {
                    UE_LOGFMT(LogBlaze,
                              Warning,
                              "PushWidgetToLayerAsync"
                              "((Layout=[{Layout}] Layer=[{LayerName}] WidgetClass=[{WidgetClass}])) "
                              "failed because the layer was not available or widget creation failed. "
                              "World=[{WorldName}]",
                              GetName(),
                              LayerName.GetTagName(),
                              GetNameSafe(ResolvedClass),
                              GetNameSafe(GetWorld()));
                    CallbackFunc(EBlazePushWidgetToLayerState::Canceled, nullptr);
                }
            }
            else
            {
                UBlazeFunctionLibrary::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);
                CallbackFunc(EBlazePushWidgetToLayerState::Canceled, nullptr);
            }
        }));

    Handle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this, [this, CallbackFunc, SuspendInputToken]() {
        UBlazeFunctionLibrary::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);
        CallbackFunc(EBlazePushWidgetToLayerState::Canceled, nullptr);
    }));

    return Handle;
}

void UBlazePrimaryLayout::RemoveWidgetFromLayer(const FGameplayTag LayerName,
                                                UCommonActivatableWidget* ActivatableWidget) const
{
    check(LayerName.IsValid());
    check(ActivatableWidget);
    if (const auto Layer = GetLayer(LayerName))
    {
        Layer->RemoveWidget(*ActivatableWidget);
    }
    else
    {
        UE_LOGFMT(LogBlaze,
                  Warning,
                  "RemoveWidgetFromLayer((LayerName=[{LayerName}] ActivatableWidget=[{ActivatableWidget}]) "
                  "ignored as no such Layer. "
                  "World=[{WorldName}]",
                  LayerName.GetTagName(),
                  GetNameSafe(ActivatableWidget),
                  GetNameSafe(GetWorld()));
    }
}

UCommonActivatableWidgetContainerBase* UBlazePrimaryLayout::GetLayer(const FGameplayTag LayerName) const
{
    check(LayerName.IsValid());
    return Layers.FindRef(LayerName);
}
