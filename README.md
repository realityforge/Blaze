# Blaze UI Framework

Blaze is a lightweight Unreal Engine plugin that provides a structured framework for managing player-specific UI layouts
and layered widget systems.  

It integrates with the `CommonUI` framework, allowing developers to build scalable, multi-layered UIs for both
single-player and multiplayer (split-screen) games.

## Features

- **Primary Layout Management** - Each local player can own a `UBlazePrimaryLayout` that defines and manages UI layers (
  HUD, menus, overlays, etc.).
- **Dynamic Layer Control** - Push, stream, and pop `UCommonActivatableWidget` instances at runtime via
  `UBlazeFunctionLibrary`.
- **Asynchronous Loading** - Support for soft-referenced widgets that load on demand without blocking the UI thread.
- **Input Suspension** - Temporarily suspend and resume player input during UI transitions or async widget loading.
- **Blueprint & C++ Ready** - Fully accessible from both Blueprint and native code.

## Getting Started

1. Add the **Blaze** plugin to your Unreal project’s `Plugins/` folder.
2. Create gameplay tags for your UI layers (e.g., `UI.Layer.HUD`, `UI.Layer.Menu`).
  - Remap the "UILayersCategory" GameplayTag category to use the tag hierarchy that you have used to defined labels for your UI layers. Typically this will mean adding to the section in `DefaultGameplayTags.ini` like below:

```ini
[/Script/GameplayTags.GameplayTagsSettings]
...
+CategoryRemapping=(BaseCategory="UILayersCategory",RemapCategories=("UI.Layer"))
..
```

3. Create a `UBlazePrimaryLayout` subclass to define your game’s layout and layer structure. Ensure that layers are registered with the tags using `RegisterLayer()`. 
4. Create a `UBlazePrimaryLayoutManager` to manage the creation of PrimaryLayouts with players.
5. Optionally extend `UBlazeGameInstance` or incorporate `UBlazePlayerControllerComponent` to automatically register players with the system.
6. Push and remove content dynamically using `UBlazeFunctionLibrary` at runtime:

```c++
   UBlazeFunctionLibrary::PushContentToLayer_ForPlayer(PlayerController, LayerTag, WidgetClass);
````

## Requirements

* Unreal Engine 5.3 or higher
* CommonUI plugin enabled

## License

Licensed under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).
