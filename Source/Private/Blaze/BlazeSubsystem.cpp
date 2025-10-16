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
#include "Blaze/BlazeSubsystem.h"
#include "Blaze/BlazeLogging.h"
#include "Blaze/BlazePrimaryLayoutManager.h"
#include "Engine/GameInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlazeSubsystem)

void UBlazeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    if (PrimaryLayoutManager)
    {
        UE_LOGFMT(LogBlaze,
                  Log,
                  "[{Name}] is initializing but PrimaryLayoutManager=[{PrimaryLayoutManager}] "
                  "already present. No need to recreate. "
                  "World=[{WorldName}]",
                  GetName(),
                  GetNameSafe(PrimaryLayoutManager),
                  GetNameSafe(GetWorld()));
    }
    else if (PrimaryLayoutManagerClass.IsNull())
    {
        UE_LOGFMT(LogBlaze,
                  Error,
                  "[{Name}] attempting to initialize but "
                  "PrimaryLayoutManagerClass is null. System misconfigured - skipping Initialize. "
                  "World=[{WorldName}]",
                  GetName(),
                  GetNameSafe(GetWorld()));
    }
    else if (const auto Class = PrimaryLayoutManagerClass.LoadSynchronous())
    {
        UE_LOGFMT(LogBlaze,
                  Log,
                  "[{Name}] is initializing. PrimaryLayoutManager=[{PrimaryLayoutManager}]. "
                  "World=[{WorldName}]",
                  GetName(),
                  GetNameSafe(Class),
                  GetNameSafe(GetWorld()));
        if (const auto NewPrimaryLayoutManager = NewObject<UBlazePrimaryLayoutManager>(this, Class))
        {
            UE_LOGFMT(LogBlaze,
                      Log,
                      "[{Name}] created PrimaryLayoutManager=[{PrimaryLayoutManager}]. "
                      "World=[{WorldName}]",
                      GetName(),
                      GetNameSafe(NewPrimaryLayoutManager),
                      GetNameSafe(GetWorld()));

            SwitchToPrimaryLayoutManager(NewPrimaryLayoutManager);
        }
        else
        {
            UE_LOGFMT(LogBlaze,
                      Error,
                      "[{Name}] failed to create PrimaryLayoutManager "
                      "for class [{PrimaryLayoutManagerClass}]. Skipping initialization. "
                      "World=[{WorldName}]",
                      GetName(),
                      GetNameSafe(Class),
                      GetNameSafe(GetWorld()));
            SwitchToPrimaryLayoutManager(nullptr);
        }
    }
    else
    {
        UE_LOGFMT(LogBlaze,
                  Error,
                  "[{Name}] PrimaryLayoutManagerClass [{PrimaryLayoutManagerClass}] "
                  "failed to load. Skipping Initialize. "
                  "World=[{WorldName}]",
                  GetName(),
                  PrimaryLayoutManagerClass.ToSoftObjectPath(),
                  GetNameSafe(GetWorld()));
    }
}

void UBlazeSubsystem::Deinitialize()
{
    Super::Deinitialize();

    SwitchToPrimaryLayoutManager(nullptr);
}

bool UBlazeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return false;
}

void UBlazeSubsystem::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
    UE_LOGFMT(LogBlaze,
              Log,
              "NotifyPlayerAdded - [{Name}] is adding LocalPlayer [{LocalPlayer}]. "
              "World=[{WorldName}]",
              GetName(),
              GetNameSafe(LocalPlayer),
              GetNameSafe(GetWorld()));
    if (ensure(LocalPlayer))
    {
        if (PrimaryLayoutManager)
        {
            PrimaryLayoutManager->NotifyPlayerAdded(LocalPlayer);
        }
    }
}

void UBlazeSubsystem::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
    UE_LOGFMT(LogBlaze,
              Log,
              "NotifyPlayerRemoved - [{Name}] is removing LocalPlayer [{LocalPlayer}]. World=[{WorldName}]",
              GetName(),
              GetNameSafe(LocalPlayer),
              GetNameSafe(GetWorld()));
    if (LocalPlayer && PrimaryLayoutManager)
    {
        PrimaryLayoutManager->NotifyPlayerRemoved(LocalPlayer);
    }
}

void UBlazeSubsystem::NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer)
{
    UE_LOGFMT(LogBlaze,
              Log,
              "NotifyPlayerDestroyed - [{Name}] is destroying LocalPlayer [{LocalPlayer}]. World=[{WorldName}]",
              GetName(),
              GetNameSafe(LocalPlayer),
              GetNameSafe(GetWorld()));
    if (LocalPlayer && PrimaryLayoutManager)
    {
        PrimaryLayoutManager->NotifyPlayerDestroyed(LocalPlayer);
    }
}

void UBlazeSubsystem::SwitchToPrimaryLayoutManager(UBlazePrimaryLayoutManager* InPrimaryLayoutManager)
{
    if (PrimaryLayoutManager != InPrimaryLayoutManager)
    {
        UE_LOGFMT(LogBlaze,
                  Log,
                  "SwitchToPrimaryLayoutManager - [{Name}] is switching from PrimaryLayoutManager [{Old}] to [{New}]. "
                  "World=[{WorldName}]",
                  GetName(),
                  GetNameSafe(PrimaryLayoutManager),
                  GetNameSafe(InPrimaryLayoutManager),
                  GetNameSafe(GetWorld()));
        if (PrimaryLayoutManager)
        {
            OnSwitchOutPrimaryLayoutManager(PrimaryLayoutManager);
        }
        PrimaryLayoutManager = InPrimaryLayoutManager;
        if (PrimaryLayoutManager)
        {
            OnSwitchInPrimaryLayoutManager(PrimaryLayoutManager);
        }
    }
}

void UBlazeSubsystem::OnSwitchOutPrimaryLayoutManager(UBlazePrimaryLayoutManager* OldPrimaryLayoutManager) {}

void UBlazeSubsystem::OnSwitchInPrimaryLayoutManager(UBlazePrimaryLayoutManager* NewPrimaryLayoutManager) {}
