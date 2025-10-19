# Repository Guidelines

## Authoritative Repository
The canonical source for this plugin is maintained at https://github.com/realityforge/Blaze. Please open pull requests and issues on that repository.

This plugin is sometimes integrated into other repositories via git subtree merge or distributed as a direct download (unzip into `Plugins/Blaze/`). Those copies are mirrors for consumption; treat them as downstream integrations and avoid filing issues/PRs there.

## Project Structure & Module Organization
- `Blaze.uplugin` declares a single module; the module sits under `Source/Public` and `Source/Private` so shared headers stay isolated from implementation-only details.
- Core gameplay helpers live in `Source/Aeon`, AI behaviors in `Source/AeonAI`, animation utilities in `Source/AeonAnimation`, and editor-only tooling in `Source/AeonEditor`.
- Raw files (such as `.csv` files) from which Unreal assets are imported belong in `SourceContent`, while `Content` is reserved for runtime assets that ship with the plugin.
- Generated binaries and build artifacts should stay out of version control and should stay untouched unless you are troubleshooting a local build.
- Keep `README.md` aligned with new features so downstream teams stay informed.

## Tooling & Engine Version
- Target Unreal Engine 5.6 for both development and verification; earlier engine releases are unsupported.

## Coding Style & Naming Conventions
- Follow Unreal Engine defaults: 4-space indentation, PascalCase types, camelCase locals, `FBlaze*` for structs, `UBlaze*` for UObject classes, and `EBlaze*` for enums.
- Use type deduction (for example, `auto`) when it keeps intent clear and remains within Unreal Engine 5.6’s supported C++ feature set.
- Place new public headers under `Source/<Module>/Public/<Module>/` and implementation files under the matching `Private` path.
- Prefer UE logging macros with the `LogBlaze` category; declare new categories in module `Private` headers when needed.
- Keep includes ordered and minimal; prefer forward declarations in headers.
- Match existing code style when not covered by the above rules.

## Testing Guidelines
- Automation coverage is aspirational. Capture edge cases in unit-style specs once a testing harness lands under `Source/<Module>/Private/Tests/`.
- Until formal suites exist, document manual reproduction steps or sample maps in the pull request so reviewers can exercise the change.

## Commit & Pull Request Guidelines
- Write imperative, present-tense commit messages under 72 characters, mirroring existing history such as “Add accessor for AeonAbilitySystemComponent”.
- Squash noisy work-in-progress commits locally; each change should stand on its own.
- Open pull requests with a clear summary, reproduction or test notes, and screenshots or GIFs when changes impact in-editor UX.
