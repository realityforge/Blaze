# Getting Started with Blaze

Blaze is a lightweight Unreal Engine plugin that helps you manage per‑player UI via a primary layout, named layers, and streamlined APIs for pushing/popping widgets (synchronously or asynchronously). This guide shows how to integrate Blaze into “MyGame” using only classes that live inside the Blaze plugin.

- Public headers you’ll use:
  - `Source/Public/Blaze/BlazeSubsystem.h`
  - `Source/Public/Blaze/BlazePrimaryLayout.h`
  - `Source/Public/Blaze/BlazePrimaryLayoutManager.h`
  - `Source/Public/Blaze/BlazeFunctionLibrary.h`
  - `Source/Public/Blaze/BlazePlayerControllerComponent.h`
  - `Source/Public/Blaze/Actions/AsyncAction_PushContentToLayer.h`

## Prerequisites

- Unreal Engine 5.3+
- `CommonUI` and `ModularGameplay` plugins enabled (Blaze lists them in `Blaze.uplugin` so they will be loaded when Blaze is enabled).
- Gameplay Tags enabled in your project.

## Install the Plugin

1. Copy this repository under your project’s `Plugins/Blaze/` folder.
2. Enable the plugin in the Project Settings or via the editor’s Plugins window.

## Configure Gameplay Tags (UI Layer Category)

Blaze uses gameplay tags to identify layers. Map the `UILayersCategory` category to your project’s hierarchy, for example `UI.Layer.*`:

Add to `Config/DefaultGameplayTags.ini`:

```ini
[/Script/GameplayTags.GameplayTagsSettings]
+CategoryRemapping=(BaseCategory="UILayersCategory",RemapCategories=("UI.Layer"))
```

You’ll then define tags such as `UI.Layer.Game`, `UI.Layer.Menu`, `UI.Layer.GameMenu`, etc.

## Create the Primary Layout

You can implement layouts in Blueprint or C++. The layout registers one or more layer containers (CommonUI stacks).

Option A — Blueprint
- Create a Widget Blueprint that derives from `UBlazePrimaryLayout`.
- Add `UCommonActivatableWidgetStack` children for each layer (e.g., Game, Menu, GameMenu).
- In “On Initialized”, call `RegisterLayer(Tag, Stack)` for each stack, where Tag is a gameplay tag under `UI.Layer.*`.

Option B — C++

Header `UMyGamePrimaryLayout`:

```cpp
#pragma once

#include "Blaze/BlazePrimaryLayout.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "MyGamePrimaryLayout.generated.h"

UCLASS(Abstract)
class UMyGamePrimaryLayout : public UBlazePrimaryLayout
{
    GENERATED_BODY()

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = true))
    TObjectPtr<UCommonActivatableWidgetStack> GameStack;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = true))
    TObjectPtr<UCommonActivatableWidgetStack> GameMenuStack;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = true))
    TObjectPtr<UCommonActivatableWidgetStack> MenuStack;

    virtual void NativeOnInitialized() override;
};
```

Source:

```cpp
#include "MyGamePrimaryLayout.h"
#include "GameplayTagContainer.h"

void UMyGamePrimaryLayout::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    const FGameplayTag Tag_Game     = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Game"));
    const FGameplayTag Tag_GameMenu = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.GameMenu"));
    const FGameplayTag Tag_Menu     = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Menu"));

    RegisterLayer(Tag_Menu, MenuStack);
    RegisterLayer(Tag_Game, GameStack);
    RegisterLayer(Tag_GameMenu, GameMenuStack);
}
```

In the Blueprint derived from `UMyGamePrimaryLayout`, make sure the three stacks are named and bound to these properties.

## Create the Primary Layout Manager

Your manager creates the layout per player and manages viewport addition/removal.

Header `UMyGamePrimaryLayoutManager`:

```cpp
#pragma once

#include "Blaze/BlazePrimaryLayoutManager.h"
#include "Blaze/BlazePrimaryLayout.h"
#include "MyGamePrimaryLayoutManager.generated.h"

UCLASS(Abstract)
class UMyGamePrimaryLayoutManager : public UBlazePrimaryLayoutManager
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, meta = (AllowAbstract = false))
    TSoftClassPtr<UBlazePrimaryLayout> PrimaryLayoutClass{ nullptr };

    virtual UBlazePrimaryLayout* CreatePrimaryLayout(APlayerController* PlayerController) override;
};
```

Source:

```cpp
#include "MyGamePrimaryLayoutManager.h"
#include "Blueprint/UserWidget.h"

UBlazePrimaryLayout* UMyGamePrimaryLayoutManager::CreatePrimaryLayout(APlayerController* PlayerController)
{
    const UClass* LayoutWidgetClass = PrimaryLayoutClass.LoadSynchronous();
    if (ensure(LayoutWidgetClass && !LayoutWidgetClass->HasAnyClassFlags(CLASS_Abstract)))
    {
        return CreateWidget<UBlazePrimaryLayout>(PlayerController, LayoutWidgetClass);
    }
    return nullptr;
}
```

Tip: Create a Blueprint that derives from `UMyGamePrimaryLayoutManager` and assign `PrimaryLayoutClass` to your primary layout Blueprint.

## Create the Subsystem

Subclass `UBlazeSubsystem` and enable it for your project.

Header `UMyGameBlazeSubsystem`:

```cpp
#pragma once

#include "Blaze/BlazeSubsystem.h"
#include "MyGameBlazeSubsystem.generated.h"

UCLASS(Config=Game)
class UMyGameBlazeSubsystem : public UBlazeSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
};
```

Configure the manager class in `Config/DefaultGame.ini`:

```ini
[/Script/MyGame.MyGameBlazeSubsystem]
PrimaryLayoutManagerClass=/Game/MyGame/UI/BP_MyGamePrimaryLayoutManager.BP_MyGamePrimaryLayoutManager_C
```

Create the `BP_MyGamePrimaryLayoutManager` Blueprint derived from `UMyGamePrimaryLayoutManager`, and set its `PrimaryLayoutClass` to your primary layout Blueprint.

## Register Players Automatically

Add `UBlazePlayerControllerComponent` to your PlayerController so the subsystem is notified when a local player is ready.

Header:

```cpp
#pragma once

#include "GameFramework/PlayerController.h"
#include "Blaze/BlazePlayerControllerComponent.h"
#include "MyGamePlayerController.generated.h"

UCLASS()
class AMyGamePlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AMyGamePlayerController();

private:
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
    TObjectPtr<UBlazePlayerControllerComponent> BlazePlayerControllerComponent{ nullptr };
};
```

Source:

```cpp
#include "MyGamePlayerController.h"

AMyGamePlayerController::AMyGamePlayerController()
{
    BlazePlayerControllerComponent = CreateDefaultSubobject<UBlazePlayerControllerComponent>(UBlazePlayerControllerComponent::NAME_DefaultName);
}
```

This component calls into `UBlazeSubsystem` once the player is received, ensuring each local player gets a primary layout.

## Using Blaze at Runtime

Synchronous push to a layer:

```cpp
#include "Blaze/BlazeFunctionLibrary.h"
#include "GameplayTagContainer.h"

void ShowHUD(AMyGamePlayerController* PC, TSubclassOf<UCommonActivatableWidget> HUDWidgetClass)
{
    const FGameplayTag Tag_Game = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Game"));
    UBlazeFunctionLibrary::PushContentToLayer(PC, Tag_Game, HUDWidgetClass);
}
```

Pop from a layer:

```cpp
#include "Blaze/BlazeFunctionLibrary.h"

void HideHUD(UCommonActivatableWidget* HUDInstance)
{
    const FGameplayTag Tag_Game = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Game"));
    UBlazeFunctionLibrary::PopContentFromLayer(Tag_Game, HUDInstance);
}
```

Asynchronous push (non‑blocking, with input suspension during load):

```cpp
#include "Blaze/Actions/AsyncAction_PushContentToLayer.h"

void ShowMenuAsync(AMyGamePlayerController* PC, TSoftClassPtr<UCommonActivatableWidget> MenuWidgetClass)
{
    const FGameplayTag Tag_Menu = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Menu"));

    if (UAsyncAction_PushContentToLayer* Action = UAsyncAction_PushContentToLayer::PushContentToLayerAsync(PC, Tag_Menu, MenuWidgetClass, /*bSuspendInputUntilComplete*/ true))
    {
        Action->OnInitialize.AddDynamic(Action, [](UCommonActivatableWidget* Widget){ /* init before push */ });
        Action->AfterPush.AddDynamic(Action, [](UCommonActivatableWidget* Widget){ /* opened */ });
        Action->OnCancelled.AddDynamic(Action, [](UCommonActivatableWidget* Widget){ /* handle cancel */ });
        Action->Activate();
    }
}
```

## Optional: Sync Layout Visibility with HUD

If your game toggles `AHUD::bShowHUD`, you can mirror that onto the primary layout’s visibility (example inside a subsystem or manager that ticks):

```cpp
#include "Blaze/BlazeFunctionLibrary.h"
#include "Engine/GameInstance.h"
#include "GameFramework/HUD.h"
#include "Containers/Ticker.h"

class FMyHUDSync
{
public:
    FTSTicker::FDelegateHandle Handle;
    UGameInstance* GameInstance = nullptr;

    bool Tick(float)
    {
        for (ULocalPlayer* LocalPlayer : GameInstance->GetLocalPlayers())
        {
            bool bShow = true;
            if (APlayerController* PC = LocalPlayer->GetPlayerController(GameInstance->GetWorld()))
            {
                if (AHUD* HUD = PC->GetHUD()) { bShow = HUD->bShowHUD; }
            }
            if (UBlazePrimaryLayout* Layout = UBlazeFunctionLibrary::GetPrimaryLayout(LocalPlayer))
            {
                Layout->SetVisibility(bShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
            }
        }
        return true;
    }
};
```

## Verify Your Setup

- On startup, `UBlazeSubsystem` should log that it loaded the `PrimaryLayoutManagerClass`. If you see “PrimaryLayoutManagerClass is null”, set it in `DefaultGame.ini`.
- When a local player is added, their layout is created and added to the viewport (see `LogBlaze`).
- Pushing to a layer with an unregistered tag will fail — register all layers in your primary layout.
- Async pushes suspend input (CommonInput) until the widget class finishes loading, then resume.
