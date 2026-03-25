#pragma once

#include "Blaze/Actions/AsyncAction_CreateWidgetAsync.h"
#include "Blaze/BlazePrimaryLayout.h"
#include "Blueprint/UserWidget.h"
#include "BlazeAutomationTestTypes.generated.h"

UCLASS(NotBlueprintable)
class UBlazeAutomationTestPrimaryLayout final : public UBlazePrimaryLayout
{
    GENERATED_BODY()
};

UCLASS(NotBlueprintable)
class UBlazeAutomationTestCreateWidgetListener final : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION()
    void HandleComplete(UUserWidget* InWidget)
    {
        bCompleted = true;
        Widget = InWidget;
    }

    UFUNCTION()
    void HandleCancelled() { bCancelled = true; }

    bool bCompleted{ false };
    bool bCancelled{ false };
    TObjectPtr<UUserWidget> Widget{ nullptr };
};

class FBlazeTestAsyncActionCreateWidgetAsyncFactory
{
public:
    static UAsyncAction_CreateWidgetAsync*
    Create(UWorld* World, APlayerController* OwningPlayer, const bool bSuspendInputUntilComplete)
    {
        const auto Action = NewObject<UAsyncAction_CreateWidgetAsync>();
        Action->WidgetClass = nullptr;
        Action->OwningPlayer = OwningPlayer;
        Action->World = World;
        Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
        Action->RegisterWithGameInstance(World);
        return Action;
    }
};
