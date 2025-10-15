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

#include "Engine/GameInstance.h"
#include "BlazeGameInstance.generated.h"

class ULocalPlayer;

/**
 * @class UBlazeGameInstance
 * @brief A game instance base class, designed to manage local player interactions.
 *
 * This class provides functionality to add and remove local players from the UBlazeSubsystem
 * when they are added or removed from the GameInstance. Extending this class is not necessary for Blaze
 * integration but if this class is not used, the LocalPlayers need to be registered with the UBlazeSubsystem
 * through other mechanisms.
 */
UCLASS(MinimalAPI, Abstract)
class UBlazeGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    BLAZE_API explicit UBlazeGameInstance(const FObjectInitializer& ObjectInitializer);

    BLAZE_API virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
    BLAZE_API virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;
};
