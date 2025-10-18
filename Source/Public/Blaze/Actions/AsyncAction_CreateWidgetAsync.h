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
#include "UObject/SoftObjectPtr.h"
#include "AsyncAction_CreateWidgetAsync.generated.h"

class APlayerController;
struct FStreamableHandle;
class UUserWidget;
class UWorld;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateWidgetAsyncOnSuccessSignature, UUserWidget*, UserWidget);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCreateWidgetAsyncOnCancelledSignature);

/**
 * Asynchronous action class for loading and creating a widget.
 */
UCLASS(MinimalAPI, BlueprintType)
class UAsyncAction_CreateWidgetAsync final : public UCancellableAsyncAction
{
    GENERATED_BODY()

public:
    BLAZE_API virtual void Activate() override;

    BLAZE_API virtual void Cancel() override;

    /**
     * Event invoked after the widget has been created.
     */
    UPROPERTY(BlueprintAssignable)
    FCreateWidgetAsyncOnSuccessSignature OnComplete;

    /**
     * Event invoked if the operation has been canceled.
     */
    UPROPERTY(BlueprintAssignable)
    FCreateWidgetAsyncOnCancelledSignature OnCancelled;

    /**
     * Factory method to initiate an asynchronous process for creating a widget.
     *
     * This call returns immediately and does not block while the widget class is loaded.
     * Use the return value to add callbacks for completion, or cancellation of the load operation.
    *
     * @param WorldContextObject The context object that provides the world for the operation.
     * @param WidgetClass The class of the widget to be created, specified as a soft reference.
     * @param OwningPlayer The player controller that will own the created widget.
     * @param bSuspendInputUntilComplete Controls whether player input is suspended until the widget creation process is
    complete.
     * @return A new instance of UAsyncAction_CreateWidgetAsync to manage the asynchronous operation.
     */
    UFUNCTION(BlueprintCallable,
              BlueprintCosmetic,
              DisplayName = "Create Widget Async",
              Category = "Blaze",
              meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
    static BLAZE_API UAsyncAction_CreateWidgetAsync* CreateWidgetAsync(UObject* WorldContextObject,
                                                                       UPARAM(meta = (AllowAbstract = false))
                                                                           TSoftClassPtr<UUserWidget> WidgetClass,
                                                                       APlayerController* OwningPlayer,
                                                                       bool bSuspendInputUntilComplete = true);

private:
    TWeakObjectPtr<UWorld> World{ nullptr };
    TSoftClassPtr<UUserWidget> WidgetClass{ nullptr };
    TWeakObjectPtr<APlayerController> OwningPlayer{ nullptr };
    bool bSuspendInputUntilComplete{ false };

    TSharedPtr<FStreamableHandle> Handle;
};
