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

#include "Components/ControllerComponent.h"
#include "BlazePlayerControllerComponent.generated.h"

/**
 * @brief A controller component responsible for integrating a player controller with the Blaze UI system.
 *
 * UBlazePlayerControllerComponent provides the glue between the game's player controller
 * and the UI management layer implemented by the UBlazeSubsystem. It ensures that
 * the local player associated with this controller is properly registered with the UI
 * manager once the player has been initialized and possessed.
 *
 * When the player is received (via ReceivedPlayer), this component notifies the
 * UBlazeSubsystem that a new local player has joined, enabling the UI framework
 * to create and manage a corresponding primary game layout and other player-specific UI.
 * If a previous registration exists, it is refreshed to guarantee a clean state.
 *
 * @see UBlazeSubsystem
 */
UCLASS(MinimalAPI, meta = (BlueprintSpawnableComponent))
class UBlazePlayerControllerComponent final : public UControllerComponent
{
    GENERATED_BODY()

public:
    /** The name of the BlazePlayerControllerComponent component */
    BLAZE_API static const FName NAME_DefaultName;

    /** Called after the player is received (after login/possession initialization) */
    virtual void ReceivedPlayer() override;
};
