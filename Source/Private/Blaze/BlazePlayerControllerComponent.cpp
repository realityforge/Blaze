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
#include "Blaze/BlazePlayerControllerComponent.h"
#include "Blaze/BlazeLogging.h"
#include "Blaze/BlazeSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Logging/StructuredLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlazePlayerControllerComponent)

const FName UBlazePlayerControllerComponent::NAME_DefaultName("BlazePlayerControllerComponent");

void UBlazePlayerControllerComponent::ReceivedPlayer()
{
    if (const auto Owner = GetOwner<APlayerController>())
    {
        if (const auto Subsystem = GetGameInstanceChecked<UGameInstance>()->GetSubsystem<UBlazeSubsystem>())
        {
            Subsystem->OnReceivedPlayerController(Owner);
        }
        else
        {
            UE_LOGFMT(LogBlaze,
                      Error,
                      "UBlazePlayerControllerComponent::ReceivedPlayer({Name}) "
                      "unable to locate BlazeSubsystem. Misconfigured application. "
                      "World=[{WorldName}]",
                      Owner->GetName(),
                      GetNameSafe(GetWorld()));
        }
    }
}
