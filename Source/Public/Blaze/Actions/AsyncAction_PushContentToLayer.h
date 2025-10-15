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
#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"
#include "AsyncAction_PushContentToLayer.generated.h"

class APlayerController;
class UCommonActivatableWidget;
struct FStreamableHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPushContentToLayerAsyncSignature, UCommonActivatableWidget*, UserWidget);

/**
 * Asynchronous action class for pushing a widget onto a specified UI layer.
 */
UCLASS(MinimalAPI, BlueprintType)
class UAsyncAction_PushContentToLayer : public UCancellableAsyncAction
{
    GENERATED_BODY()

public:
    BLAZE_API virtual void Activate() override;

    BLAZE_API virtual void Cancel() override;

    /**
     * Asynchronously creates an action to push a specified widget onto a UI layer.
     *
     * @param PlayerController The player controller associated with this operation. Must not be null.
     * @param LayerName The gameplay tag specifying the UI layer to place the widget on. Must be valid.
     * @param WidgetClass The widget class to be added to the specific layer. Must not be null.
     * @param bSuspendInputUntilComplete Indicates whether player input is suspended until the action is complete.
     * @return An instance of UAsyncAction_PushContentToLayer if successful, or nullptr if any of the parameters are
     * invalid.
     */
    UFUNCTION(BlueprintCallable,
              BlueprintCosmetic,
              DisplayName = "Push Content To Layer Async",
              Category = "Blaze",
              meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
    static BLAZE_API UAsyncAction_PushContentToLayer*
    PushContentToLayerAsync(APlayerController* PlayerController,
                            UPARAM(meta = (Categories = "UILayersCategory")) FGameplayTag LayerName,
                            UPARAM(meta = (AllowAbstract = false)) TSoftClassPtr<UCommonActivatableWidget> WidgetClass,
                            bool bSuspendInputUntilComplete = true);

private:
    /**
     * Event invoked after the widget has been initialized and before pushed onto the layer.
     */
    UPROPERTY(BlueprintAssignable)
    FPushContentToLayerAsyncSignature OnInitialize;

    /**
     * Event invoked after the widget has been pushed onto the layer.
     */
    UPROPERTY(BlueprintAssignable)
    FPushContentToLayerAsyncSignature AfterPush;

    /**
     * Event invoked if the push operation has been canceled.
     */
    UPROPERTY(BlueprintAssignable)
    FPushContentToLayerAsyncSignature OnCancelled;

    TWeakObjectPtr<APlayerController> PlayerController{ nullptr };

    FGameplayTag LayerName{ FGameplayTag::EmptyTag };

    TSoftClassPtr<UCommonActivatableWidget> WidgetClass{ nullptr };

    bool bSuspendInputUntilComplete{ false };

    TSharedPtr<FStreamableHandle> Handle;
};
