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

Quick setup overview:

1. Add the Blaze plugin to your project’s `Plugins/` folder and enable it.
2. Configure gameplay tags for UI layers (for example, `UI.Layer.Game`, `UI.Layer.Menu`) and remap the `UILayersCategory` to your hierarchy. In `Config/DefaultGameplayTags.ini`:

```ini
[/Script/GameplayTags.GameplayTagsSettings]
+CategoryRemapping=(BaseCategory="UILayersCategory",RemapCategories=("UI.Layer"))
```

3. Create a subclass of `UBlazePrimaryLayout` (Blueprint or C++) and register your layer stacks with `RegisterLayer()` or `BP_RegisterLayer()`.
4. Create a subclass of `UBlazePrimaryLayoutManager` that returns your layout in `CreatePrimaryLayout(...)` and (optionally via Blueprint) assign the layout class.
5. Create a subclass of `UBlazeSubsystem` (for example, `UMyGameBlazeSubsystem`) and set `PrimaryLayoutManagerClass` in `DefaultGame.ini` to your manager Blueprint.
6. Add `UBlazePlayerControllerComponent` to your PlayerController to auto-register local players.
7. Push/pop content at runtime via `UBlazeFunctionLibrary` or the async action:

```c++
UBlazeFunctionLibrary::PushContentToLayer(PlayerController, LayerTag, WidgetClass);
// or async:
UAsyncAction_PushContentToLayer::PushContentToLayerAsync(PlayerController, LayerTag, SoftWidgetClass, true);
```

For a complete, step‑by‑step guide with inline code examples, see `docs/GettingStarted.md`.

## Requirements

* Unreal Engine 5.3 or higher
* CommonUI plugin enabled
* ModularGameplay plugin enabled

## Authoritative Repository

The canonical source of Blaze is maintained at:

- https://github.com/realityforge/Blaze

Please open issues and pull requests on the upstream repository. This plugin may be integrated into other repositories via git subtree merge or by direct download/unzip; treat those copies as downstream integrations.

## License

Licensed under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).
