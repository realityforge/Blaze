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

#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "BlazePrimaryLayout.generated.h"

class UCommonActivatableWidget;
struct FStreamableHandle;

/**
 * The state of the async operation to push a widget onto a Layer.
 */
enum class EBlazePushWidgetToLayerState : uint8
{
    // State when the widget push operation is canceled or the Widget Class failed to resolve.
    Canceled,
    // State after the widget has been initialized and before pushed onto the layer.
    Initialize,
    // State after the widget has been pushed onto the layer.
    AfterPush
};

/**
 * @brief The primary UI layout for a player.
 *
 * UBlazePrimaryLayout acts as the root widget responsible for managing and displaying
 * all UI layers associated with a single player's experience. It provides
 * a consistent structure for stacking, transitioning, and maintaining persistent
 * and transient UI elements such as HUDs, menus, dialogs, and overlays.
 *
 * Each local player in a split-screen or multiplayer session owns their own instance of
 * UBlazePrimaryLayout, ensuring an independent UI state and interaction for each player.
 *
 * Typical responsibilities include:
 * - Hosting the player's primary HUD and other core in-game UI widgets.
 * - Managing layered UI such as menus, notifications, and popups.
 * - Handling layout transitions, focus management, and input routing.
 * - Supporting per-player layouts in split-screen or shared environments.
 *
 * Games should subclass UBlazePrimaryLayout to define layers, specialized
 * layer behavior, transitions, and/or presentation rules for different game modes.
 *
 * @see UCommonActivatableWidget
 * @see UPrimaryLayoutManager
 */
UCLASS(MinimalAPI, Abstract, meta = (DisableNativeTick))
class UBlazePrimaryLayout : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    BLAZE_API explicit UBlazePrimaryLayout(const FObjectInitializer& ObjectInitializer);

    template <typename T = UCommonActivatableWidget>
    TSharedPtr<FStreamableHandle> PushWidgetToLayerAsync(
        const FGameplayTag LayerName,
        const bool bSuspendInputUntilComplete,
        const TSoftClassPtr<UCommonActivatableWidget> WidgetClass,
        const TFunction<void(EBlazePushWidgetToLayerState, T*)> CallbackFunc = [](auto, auto) {});

    template <typename T = UCommonActivatableWidget>
    T* PushWidgetToLayer(
        const FGameplayTag LayerName,
        const UClass* WidgetClass,
        const TFunctionRef<void(T&)> InitInstanceFunc = [](auto&) {});

    /**
     * Finds a widget in the specified layer by its gameplay tag and removes it if it exists.
     *
     * @param LayerName The gameplay tag identifying the desired UI layer.
     * @param ActivatableWidget The widget to remove from the specified layer.
     */
    BLAZE_API void RemoveWidgetFromLayer(const FGameplayTag LayerName,
                                         UCommonActivatableWidget* ActivatableWidget) const;

    /**
     * Retrieves the widget container associated with the specified gameplay layer.
     *
     * @param LayerName The gameplay tag identifying the desired layer.
     * @return A pointer to the widget container corresponding to the provided layer name, or nullptr if no match is
     * found.
     */
    BLAZE_API UCommonActivatableWidgetContainerBase* GetLayer(const FGameplayTag LayerName) const;

protected:
    /** Register a layer that widgets can be pushed onto. */
    UFUNCTION(BlueprintCallable, Category = "Blaze")
    void BP_RegisterLayer(UPARAM(meta = (Categories = "UILayersCategory")) FGameplayTag LayerTag,
                          UCommonActivatableWidgetContainerBase* LayerWidget);

    /** Register a layer that widgets can be pushed onto. */
    BLAZE_API void RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);

private:
    /**
     * A mapping that records registered layers for the primary layout.
     * Layers are identified by a `FGameplayTag` and represented by a `UCommonActivatableWidgetContainerBase` object.
     */
    UPROPERTY(Transient, meta = (Categories = "UILayersCategory"))
    TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;

    /**
     * Asynchronously pushes a widget onto a specified layer stack and manages input suspension if needed.
     *
     * @param LayerName The name of the layer onto which the widget will be pushed.
     * @param bSuspendInputUntilComplete Determines whether player input is suspended until the operation completes.
     * @param WidgetClass The soft class pointer to the activatable widget to be added to the layer stack.
     * @param CallbackFunc Callback function that provides the state of the async operation and a reference to the
     * widget. The state indicates various stages such as initialization, completion, or cancellation.
     * @return A shared pointer to the streamable handle managing the asset loading process tied to the async operation.
     */
    BLAZE_API TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync_Internal(
        const FGameplayTag& LayerName,
        bool bSuspendInputUntilComplete,
        const TSoftClassPtr<UCommonActivatableWidget>& WidgetClass,
        TFunction<void(EBlazePushWidgetToLayerState, UCommonActivatableWidget*)> CallbackFunc);
};

template <typename T>
TSharedPtr<FStreamableHandle>
UBlazePrimaryLayout::PushWidgetToLayerAsync(const FGameplayTag LayerName,
                                            const bool bSuspendInputUntilComplete,
                                            const TSoftClassPtr<UCommonActivatableWidget> WidgetClass,
                                            const TFunction<void(EBlazePushWidgetToLayerState, T*)> CallbackFunc)
{
    static_assert(TIsDerivedFrom<T, UCommonActivatableWidget>::IsDerived,
                  "Template type T must be derived from UCommonActivatableWidget");
    return PushWidgetToLayerStackAsync_Internal(
        LayerName,
        bSuspendInputUntilComplete,
        WidgetClass,
        [CallbackFunc](auto State, auto Widget) { CallbackFunc(State, Widget); });
}

template <typename T>
T* UBlazePrimaryLayout::PushWidgetToLayer(const FGameplayTag LayerName,
                                          const UClass* WidgetClass,
                                          const TFunctionRef<void(T&)> InitInstanceFunc)
{
    static_assert(TIsDerivedFrom<T, UCommonActivatableWidget>::IsDerived,
                  "Template type T must be derived from UCommonActivatableWidget");
    const auto Layer = GetLayer(LayerName);
    return Layer ? Layer->AddWidget<T>(const_cast<UClass*>(WidgetClass), InitInstanceFunc) : nullptr;
}
