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
#include "Blaze/BlazePrimaryLayoutManager.h"
#include "Blaze/BlazeLogging.h"
#include "Blaze/BlazePrimaryLayout.h"
#include "Blaze/BlazeSubsystem.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlazePrimaryLayoutManager)

UWorld* UBlazePrimaryLayoutManager::GetWorld() const
{
    return GetOuterUBlazeSubsystem()->GetGameInstance()->GetWorld();
}

UBlazePrimaryLayout* UBlazePrimaryLayoutManager::GetPrimaryLayout(const ULocalPlayer* LocalPlayer) const
{
    const auto Entry = nullptr != LocalPlayer ? PrimaryLayouts.FindByKey(LocalPlayer) : nullptr;
    return Entry ? Entry->PrimaryLayout : nullptr;
}

UBlazePrimaryLayout* UBlazePrimaryLayoutManager::CreatePrimaryLayout(APlayerController* const PlayerController)
{
    checkf(false,
           TEXT("UBlazePrimaryLayoutManager::CreatePrimaryLayout invoked when it "
                "should be overridden in a derived class"));
    return nullptr;
}

void UBlazePrimaryLayoutManager::TryCreateAndAddPrimaryLayoutToViewport(ULocalPlayer* LocalPlayer)
{
    if (const auto Entry = PrimaryLayouts.FindByKey(LocalPlayer))
    {
        if (!Entry->bAddedToViewport)
        {
            AddPrimaryLayoutToViewport(LocalPlayer, Entry->PrimaryLayout);
            Entry->bAddedToViewport = true;
        }
        else
        {
            UE_LOGFMT(LogBlaze,
                      Log,
                      "[{PrimaryLayoutManager}] did not add PrimaryLayout {PrimaryLayout} "
                      "to player [{LocalPlayer}] as it was already added",
                      GetName(),
                      GetNameSafe(Entry->PrimaryLayout),
                      GetNameSafe(LocalPlayer));
        }
    }
    else if (const auto PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
    {
        if (const auto NewPrimaryLayout = CreatePrimaryLayout(PlayerController))
        {
            PrimaryLayouts.Emplace(LocalPlayer, NewPrimaryLayout, true);
            AddPrimaryLayoutToViewport(LocalPlayer, NewPrimaryLayout);
        }
        else
        {
            UE_LOGFMT(LogBlaze,
                      Error,
                      "[{PrimaryLayoutManager}] failed to create PrimaryLayout for player [{LocalPlayer}]",
                      GetName(),
                      GetNameSafe(LocalPlayer));
        }
    }
}

void UBlazePrimaryLayoutManager::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
    TryCreateAndAddPrimaryLayoutToViewport(LocalPlayer);
}

void UBlazePrimaryLayoutManager::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
    if (const auto Layout = PrimaryLayouts.FindByKey(LocalPlayer))
    {
        RemovePrimaryLayoutFromViewport(LocalPlayer, Layout->PrimaryLayout);
        Layout->bAddedToViewport = false;
    }
}

void UBlazePrimaryLayoutManager::NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer)
{
    NotifyPlayerRemoved(LocalPlayer);
    const auto EntryIndex = PrimaryLayouts.IndexOfByKey(LocalPlayer);
    if (INDEX_NONE != EntryIndex)
    {
        const auto Entry = PrimaryLayouts[EntryIndex].PrimaryLayout.Get();

        PrimaryLayouts.RemoveAt(EntryIndex);

        RemovePrimaryLayoutFromViewport(LocalPlayer, Entry);
        OnPrimaryLayoutReleased(LocalPlayer, Entry);
    }
}

int UBlazePrimaryLayoutManager::GetAddLayoutToPlayerScreenZOrder(ULocalPlayer* LocalPlayer)
{
    return DefaultZOrder;
}

void UBlazePrimaryLayoutManager::AddPrimaryLayoutToViewport(ULocalPlayer* LocalPlayer, UBlazePrimaryLayout* Layout)
{
    UE_LOGFMT(LogBlaze,
              Log,
              "[{LayoutManager}]: Adding the primary layout [{PrimaryLayout}] "
              "from the viewport for the player [{LocalPlayer}]",
              GetName(),
              GetNameSafe(Layout),
              GetNameSafe(LocalPlayer));

    Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
    Layout->AddToPlayerScreen(GetAddLayoutToPlayerScreenZOrder(LocalPlayer));

#if WITH_EDITOR
    if (GIsEditor && LocalPlayer->IsPrimaryPlayer())
    {
        // So our controller will work in PIE without needing to click in the viewport
        FSlateApplication::Get().SetUserFocusToGameViewport(0);
    }
#endif

    OnPrimaryLayoutAddedToViewport(LocalPlayer, Layout);
}

void UBlazePrimaryLayoutManager::RemovePrimaryLayoutFromViewport(ULocalPlayer* LocalPlayer, UBlazePrimaryLayout* Layout)
{
    const auto Widget = Layout->GetCachedWidget();
    if (Widget.IsValid())
    {
        UE_LOGFMT(LogBlaze,
                  Log,
                  "[{LayoutManager}]: Removing the primary layout [{PrimaryLayout}] "
                  "from the viewport for the player [{LocalPlayer}]",
                  GetName(),
                  GetNameSafe(Layout),
                  GetNameSafe(LocalPlayer));
        Layout->RemoveFromParent();
        if (Widget.IsValid())
        {
            UE_LOGFMT(LogBlaze,
                      Log,
                      "[{LayoutManager}]: The primary layout [{PrimaryLayout}] has been removed"
                      "from the viewport for the player [{LocalPlayer}] but references to the "
                      "underlying widget still exist.",
                      GetName(),
                      GetNameSafe(Layout),
                      GetNameSafe(LocalPlayer));
        }

        OnPrimaryLayoutRemovedFromViewport(LocalPlayer, Layout);
    }
}

void UBlazePrimaryLayoutManager::OnPrimaryLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UBlazePrimaryLayout* Layout)
{
}

void UBlazePrimaryLayoutManager::OnPrimaryLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer,
                                                                    UBlazePrimaryLayout* Layout)
{
}

void UBlazePrimaryLayoutManager::OnPrimaryLayoutReleased(ULocalPlayer* LocalPlayer, UBlazePrimaryLayout* Layout) {}
