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

#include "BlazePrimaryLayoutManager.generated.h"

class UBlazeSubsystem;
class ULocalPlayer;
class UBlazePrimaryLayout;

/**
 * @struct FPrimaryLayoutMapping
 * @brief Represents an entry containing data about a primary layout associated with a local player.
 *
 * This structure is used to manage the mapping between a local player and its associated UI components,
 * including the primary layout instance and its state in relation to the viewport.
 */
USTRUCT()
struct FPrimaryLayoutMapping
{
    GENERATED_BODY()

    UPROPERTY(Transient)
    TObjectPtr<const ULocalPlayer> LocalPlayer{ nullptr };

    UPROPERTY(Transient)
    TObjectPtr<UBlazePrimaryLayout> PrimaryLayout{ nullptr };

    UPROPERTY(Transient)
    bool bAddedToViewport{ false };

    FPrimaryLayoutMapping() {}

    FPrimaryLayoutMapping(ULocalPlayer* InLocalPlayer,
                          UBlazePrimaryLayout* InPrimaryLayout,
                          const bool bInAddedToViewport)
        : LocalPlayer(InLocalPlayer), PrimaryLayout(InPrimaryLayout), bAddedToViewport(bInAddedToViewport)
    {
    }

    bool operator==(const ULocalPlayer* OtherLocalPlayer) const { return LocalPlayer == OtherLocalPlayer; }
};

/**
 * @class UBlazePrimaryLayoutManager
 * @brief Manages the lifecycle and viewport integration of primary layouts associated with local players.
 *
 * This class is responsible for creating, adding, removing, and maintaining primary layouts for
 * local players. It serves as an interface between local player instances and their respective
 * UI components.
 *
 * The class is abstract and it is expected that a subclass will be defined that overrides various template methods.
 */
UCLASS(MinimalAPI, Abstract, Blueprintable, Within = BlazeSubsystem)
class UBlazePrimaryLayoutManager : public UObject
{
    GENERATED_BODY()

public:
    BLAZE_API virtual UWorld* GetWorld() const override;

    /**
     * Retrieves the primary layout associated with the specified local player.
     *
     * This method searches through the registered primary layouts to return the layout corresponding
     * to the given local player.
     *
     * @param LocalPlayer A pointer to the local player for which the primary layout is to be fetched.
     * @return A pointer to the primary layout associated with the provided local player, or nullptr if none found.
     */
    BLAZE_API UBlazePrimaryLayout* GetPrimaryLayout(const ULocalPlayer* LocalPlayer) const;

protected:
    /**
     * @brief A template method invoked when a primary layout is successfully added to the viewport
     *
     * Subclasses can override this method to perform additional behavior.
     *
     * @param LocalPlayer A pointer to the local player associated with the primary layout being added.
     * @param Layout A pointer to the primary layout instance that was added to the viewport.
     */
    BLAZE_API virtual void OnPrimaryLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UBlazePrimaryLayout* Layout);

    /**
     * @brief A template method invoked when a primary layout is removed from the viewport for a given local player.
     *
     * Subclasses can override this method to perform additional behavior.
     *
     * @param LocalPlayer A pointer to the local player associated with the primary layout being removed.
     * @param Layout A pointer to the primary layout instance that was removed from the viewport.
     */
    BLAZE_API virtual void OnPrimaryLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UBlazePrimaryLayout* Layout);

    /**
     * @brief A template method invoked when a primary layout is released for a specified local player.
     *
     * Subclasses can override this method to perform additional behavior.
     *
     * @param LocalPlayer A pointer to the local player for whom the primary layout is being released.
     * @param Layout A pointer to the primary layout instance that is being released.
     */
    BLAZE_API virtual void OnPrimaryLayoutReleased(ULocalPlayer* LocalPlayer, UBlazePrimaryLayout* Layout);

    /**
     * @brief Creates a new primary layout for the specified player controller.
     *
     * This method MUST be overriden in a derived class to return a layout instance.
     *
     * @param PlayerController The player controller that will own the PrimaryLayout widget.
     * @return A pointer to the newly created primary layout instance, or nullptr if the layout creation fails.
     */
    BLAZE_API virtual UBlazePrimaryLayout* CreatePrimaryLayout(APlayerController* PlayerController);

    /**
     * @brief Return the Z-order value to be used when adding a primary layout for a specified local player to the
     * player's screen.
     *
     * Subclasses can override this but it should be rarely needed.
     *
     * @param LocalPlayer A pointer to the local player for whom the layout's Z-order is to be retrieved.
     * @return An integer representing the Z-order value to use for the primary layout of the specified local player.
     */
    BLAZE_API virtual int GetAddLayoutToPlayerScreenZOrder(ULocalPlayer* LocalPlayer);

private:
    UPROPERTY(Transient)
    TArray<FPrimaryLayoutMapping> PrimaryLayouts;

    void AddPrimaryLayoutToViewport(ULocalPlayer* LocalPlayer, UBlazePrimaryLayout* Layout);
    void RemovePrimaryLayoutFromViewport(ULocalPlayer* LocalPlayer, UBlazePrimaryLayout* Layout);

    void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);
    void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);
    void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

    void TryCreateAndAddPrimaryLayoutToViewport(ULocalPlayer* LocalPlayer);

    friend class UBlazeSubsystem;
};
