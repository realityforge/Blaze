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

#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/SoftObjectPtr.h"
#include "BlazeSubsystem.generated.h"

class FSubsystemCollectionBase;
class UCommonLocalPlayer;
class UBlazePrimaryLayoutManager;
class UObject;

/**
 * @brief The subsystem manages the PrimaryLayoutManager.
 *
 * The Subsystem should be subclassed and the ShouldCreateSubsystem MUST be overriden to
 * return true. The application may also override other methods to provide additional
 * application-specific functionality.
 */
UCLASS(MinimalAPI, Abstract, Config = Game)
class UBlazeSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    BLAZE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    BLAZE_API virtual void Deinitialize() override;

    /**
     * Determines whether the subsystem should be created based on specific conditions.
     *
     * This method returns false and MUST be overriden to return true in derived classes.
     *
     * @return True to create the subsystem else false.
     */
    BLAZE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /**
     * @brief Handles the event when a PlayerController is received.
     *
     * This function should be called when a PlayerController instance is received.
     * This is typically invoked from the `UPlayerController::ReceivedPlayer` method.
     */
    BLAZE_API void OnReceivedPlayerController(const APlayerController* Owner);

    /**
     * Notify the Subsystem that a player has been added.
     * This creates the primary layout for the player if necessary and adds the layout to
     * the player's viewport.
     *
     * This is expected to be invoked before using Blaze functionality and may be
     * invoked through one of the following mechanisms. (The specific choice will depend
     * on how the game integrates with Blaze).
     * - explicitly in game code
     * - via BlazeGameInstance extensions
     *
     * @param LocalPlayer The player that was added.
     */
    BLAZE_API virtual void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);

    /**
     * Notify the Subsystem that a player has been removed.
     * This removes the primary layout from the player's viewport but does not destroy the layout.
     *
     * See NotifyPlayerAdded documentation on how it should be invoked.
     *
     * @param LocalPlayer The player that was removed.
     */
    BLAZE_API virtual void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);

    /**
     * Notify the Subsystem that a player has been destroyed.
     * This removes the primary layout from the player's viewport and destroys the layout.
     *
     * See NotifyPlayerAdded documentation on how it should be invoked.
     *
     * @param LocalPlayer The player that was destroyed.
     */
    BLAZE_API virtual void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

protected:
    /**
     * @brief A template method invoked after a primary layout manager is switched in.
     *
     * This method is not invoked if the layout manager is set to nullptr.
     * Subclasses can override this method to perform additional behavior.
     *
     * @param NewPrimaryLayoutManager The new primary layout manager. Must not be nullptr.
     */
    BLAZE_API virtual void OnSwitchInPrimaryLayoutManager(UBlazePrimaryLayoutManager* NewPrimaryLayoutManager);

    /**
     * @brief A template method invoked prior to a primary layout manager being switched out.
     *
     * This method is not invoked if there was no previously set layout manager.
     * Subclasses can override this method to perform additional behavior.
     *
     * @param OldPrimaryLayoutManager The old primary layout manager. Must not be nullptr.
     */
    BLAZE_API virtual void OnSwitchOutPrimaryLayoutManager(UBlazePrimaryLayoutManager* OldPrimaryLayoutManager);

    FORCEINLINE const UBlazePrimaryLayoutManager* GetPrimaryLayoutManager() const { return PrimaryLayoutManager; }
    FORCEINLINE UBlazePrimaryLayoutManager* GetPrimaryLayoutManager() { return PrimaryLayoutManager; }

private:
    UPROPERTY(Transient)
    TObjectPtr<UBlazePrimaryLayoutManager> PrimaryLayoutManager{ nullptr };

    UPROPERTY(Config, EditAnywhere)
    TSoftClassPtr<UBlazePrimaryLayoutManager> PrimaryLayoutManagerClass{ nullptr };

    void SwitchToPrimaryLayoutManager(UBlazePrimaryLayoutManager* InPrimaryLayoutManager);

    friend class UBlazeFunctionLibrary;
};
