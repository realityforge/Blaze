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

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/SoftObjectPtr.h"
#include "BlazeFunctionLibrary.generated.h"

class APlayerController;
struct FGameplayTag;
class UBlazePrimaryLayout;
class UBlazePrimaryLayoutManager;
class UCommonActivatableWidget;
class ULocalPlayer;
template <typename T>
class TSubclassOf;

/**
 * Blueprint function library exposing useful functions used within Blaze.
 */
UCLASS(MinimalAPI)
class UBlazeFunctionLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Retrieves the primary game layout associated with the specified PlayerController.
     *
     * @param PlayerController The PlayerController for which the primary game layout is requested.
     * @return A pointer to the primary game layout for the associated local player, or nullptr if it cannot be found.
     */
    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Blaze")
    static BLAZE_API UBlazePrimaryLayout* GetPrimaryLayout(const APlayerController* PlayerController);

    /**
     * Retrieves the primary game layout associated with the specified local player.
     *
     * @param LocalPlayer The local player for which the primary game layout is requested.
     * @return A pointer to the primary game layout for the given local player, or nullptr if it cannot be found.
     */
    static BLAZE_API UBlazePrimaryLayout* GetPrimaryLayout(const ULocalPlayer* LocalPlayer);

    /**
     * Adds a widget to the specified UI layer synchronously.
     *
     * This call blocks until the widget class is fully loaded and added to the layer.
     * For non-blocking, asynchronous loading, use the `UAsyncAction_PushContentToLayer` class instead.
     *
     * @param PlayerController The player controller representing the player.
     * @param LayerName The tag identifying the target layer to which the widget should be added.
     * @param WidgetClass The class of the widget to be activated and added to the specified layer.
     * @return A pointer to the added widget instance upon successful execution, or nullptr if the process fails.
     */
    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Blaze")
    static BLAZE_API UCommonActivatableWidget*
    PushContentToLayer(APlayerController* PlayerController,
                       UPARAM(meta = (Categories = "UILayersCategory")) FGameplayTag LayerName,
                       UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> WidgetClass);

    /**
     * Pushes a specified widget class to a designated layer for a given local player by delegating to another method.
     * This allows the management of UI content at various layers.
     *
     * @param LocalPlayer The local player.
     * @param LayerName The tag identifying the target layer to which the widget should be added.
     * @param WidgetClass The class of the widget to be activated and added to the specified layer.
     * @return A pointer to the added widget instance upon successful execution, or nullptr if the process fails.
     */
    static BLAZE_API UCommonActivatableWidget*
    PushContentToLayer(const ULocalPlayer* LocalPlayer,
                       const FGameplayTag LayerName,
                       const TSubclassOf<UCommonActivatableWidget> WidgetClass);

    /**
     * Removes a specified activatable widget from the specified UI layer it is currently displayed within.
     *
     * @param LayerName The tag identifying the target layer from which the widget should be remove.
     * @param ActivatableWidget The widget to remove from its current UI layer.
     */
    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Blaze")
    static BLAZE_API void PopContentFromLayer(const FGameplayTag LayerName,
                                              UCommonActivatableWidget* ActivatableWidget);

private:
    /**
     * Retrieves the primary layout manager associated with the specified world context.
     *
     * @param WorldContextObject The context object used to locate the world and its associated game instance.
     * @return A pointer to the primary layout manager if found, or nullptr if it cannot be located.
     */
    static UBlazePrimaryLayoutManager* GetPrimaryLayoutManager(const UObject* WorldContextObject);

    /**
     * Suspends input for a specific player controller by delegating to another method that handles the associated local
     * player. This method effectively disables all input types (MouseAndKeyboard, Gamepad, Touch) for the local player
     * derived from the provided player controller.
     *
     * @param PlayerController The player controller for which the input should be suspended.
     * @param SuspendReasonBase A base name used to generate a unique suspend token. This provides identification
     *                          for the suspension.
     * @return The generated suspend token identifying the input suspension. Returns NAME_None if the operation fails.
     */
    static FName SuspendInputForPlayer(const APlayerController* PlayerController, FName SuspendReasonBase);

    /**
     * Suspends input for a specific local player by adding input type filters associated with a generated suspend
     * token. This method interacts with the CommonInputSubsystem to disable all input types (MouseAndKeyboard, Gamepad,
     * Touch) for the specified player.
     *
     * @param LocalPlayer The local player for whom the input should be suspended.
     * @param SuspendReasonBase A base name used to generate a unique suspend token. This provides identification
     *                          for the suspension.
     * @return The generated suspend token identifying the input suspension.
     */
    static FName SuspendInputForPlayer(const ULocalPlayer* LocalPlayer, FName SuspendReasonBase);

    /**
     * Resumes input for a specific player controller by delegating to another method that handles the associated local
     * player. The function ensures input for the specified player is re-enabled by interacting with relevant systems.
     *
     * The suspend token is used to identify the input suspension that should be resumed.
     *
     * @param PlayerController The player controller for which the input should be resumed.
     * @param SuspendToken The token that identifies the input suspension to be resumed.
     */
    static void ResumeInputForPlayer(const APlayerController* PlayerController, FName SuspendToken);

    /**
     * Resumes input for a specific local player by removing input type filters associated with the given suspend token.
     * This method interacts with the CommonInputSubsystem to ensure that all input types (MouseAndKeyboard, Gamepad,
     * Touch) are re-enabled for the specified player.
     *
     * @param LocalPlayer The local player for whom the input should be resumed.
     * @param SuspendToken The token that identifies the input suspension to be resumed.
     */
    static void ResumeInputForPlayer(const ULocalPlayer* LocalPlayer, FName SuspendToken);

    static UBlazePrimaryLayoutManager* GetPrimaryLayoutManager(const UGameInstance* GameInstance);

    friend class UBlazePrimaryLayout;
};
