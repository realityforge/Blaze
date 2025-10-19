# Repository Guidelines

## Authoritative Repository
The canonical source for this plugin is maintained at https://github.com/realityforge/Blaze. Please open pull requests and issues on that repository.

This plugin is sometimes integrated into other repositories via git subtree merge or distributed as a direct download (unzip into `Plugins/Blaze/`). Those copies are mirrors for consumption; treat them as downstream integrations and avoid filing issues/PRs there.

## Project Structure & Module Organization
This repository hosts the Blaze Unreal Engine plugin. `Blaze.uplugin` defines the single module, while `Source/Public` exposes headers for gameplay layers, managers, and the `UBlazeFunctionLibrary`. `Source/Private` contains implementation details and is where new native logic should live. Generated artifacts (for example `Binaries/` and `Intermediate/`) should stay untouched unless you are troubleshooting a local build. Integration uses `APlayerController::ReceivedPlayer()`. Keep `README.md` aligned with new features so downstream teams stay informed.

## Build & Development Commands
- `Engine/Build/BatchFiles/RunUAT.sh BuildPlugin -Plugin=$(pwd)/Blaze.uplugin -Package=/path/to/Build/Blaze` packages the plugin for distribution; switch to the `.bat` variant on Windows.
- `Engine/Binaries/DotNET/UnrealBuildTool.exe BlazeEditor Win64 Development -Project=/path/to/YourProject.uproject` rebuilds the plugin module for local iteration.
Invoke these from an Unreal Engine checkout where the plugin is installed under `Plugins/Blaze/`. Use consistent engine versions to avoid churn in generated binaries.

## Coding Style & Naming Conventions
Follow Epic’s C++ style: four-space indentation, PascalCase for classes (`UBlazePrimaryLayout`), camelCase for member functions, and lowercase with underscores for local variables where Unreal guidelines allow it. Use type deduction (for example, `auto`) when it keeps intent clear and remains within Unreal Engine 5.6’s supported C++ feature set. Keep public APIs header-only in `Source/Public` and minimize includes by using forward declarations. When touching Blueprint-exposed types, ensure metadata specifiers stay alphabetized and reflect the runtime behavior. Honor line endings tracked in `.gitattributes` so headers, sources, and Markdown stay on the native EOLs the repository enforces. Run your IDE’s clang-format profile if available, and never commit trailing whitespace or BOMs in new files.

## Commit & Pull Request Guidelines
Use imperative commit subjects under 50 characters. Describe intent  and motivation after the subject before summarizing changes.
