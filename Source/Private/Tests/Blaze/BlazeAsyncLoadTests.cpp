#if WITH_DEV_AUTOMATION_TESTS

    #include "Blaze/Actions/AsyncAction_CreateWidgetAsync.h"
    #include "CommonActivatableWidget.h"
    #include "Engine/Engine.h"
    #include "Engine/World.h"
    #include "GameFramework/PlayerController.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/Blaze/BlazeAutomationTestTypes.h"

namespace BlazeAsyncLoadTests
{
    constexpr auto AutomationTestFlags =
        EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestLayerTag, "Blaze.Test.Layer");

    class FTestWorld
    {
    public:
        FTestWorld()
        {
            if (GEngine)
            {
                World = UWorld::CreateWorld(EWorldType::Game,
                                            false,
                                            MakeUniqueObjectName(GetTransientPackage(),
                                                                 UWorld::StaticClass(),
                                                                 FName(TEXT("BlazeAutomationTestWorld"))),
                                            GetTransientPackage(),
                                            true);
                if (World)
                {
                    World->SetShouldTick(false);
                    World->AddToRoot();

                    auto& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
                    WorldContext.SetCurrentWorld(World);
                }
            }
        }

        ~FTestWorld()
        {
            if (World)
            {
                World->RemoveFromRoot();
                if (GEngine)
                {
                    GEngine->DestroyWorldContext(World);
                }
                World->DestroyWorld(false);
            }
        }

        bool IsValid() const { return nullptr != World; }

        template <typename TActor>
        TActor* SpawnActor() const
        {
            if (World)
            {
                FActorSpawnParameters SpawnParameters;
                SpawnParameters.ObjectFlags |= RF_Transient;
                SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                return World->SpawnActor<TActor>(TActor::StaticClass(),
                                                 FVector::ZeroVector,
                                                 FRotator::ZeroRotator,
                                                 SpawnParameters);
            }
            else
            {
                return nullptr;
            }
        }

        UWorld* Get() const { return World; }

    private:
        UWorld* World{ nullptr };
    };

    void ForceLinkAsyncLoadTests() {}
} // namespace BlazeAsyncLoadTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBlazeCreateWidgetAsyncCancelsWhenLoadHandleInvalidTest,
                                 "Blaze.CreateWidgetAsync.CancelsWhenLoadHandleInvalid",
                                 BlazeAsyncLoadTests::AutomationTestFlags)
bool FBlazeCreateWidgetAsyncCancelsWhenLoadHandleInvalidTest::RunTest(const FString&)
{
    const auto World = MakeUnique<BlazeAsyncLoadTests::FTestWorld>();
    if (TestNotNull(TEXT("Automation test world should be created"), World.Get())
        && TestTrue(TEXT("Automation test world should be valid"), World->IsValid()))
    {
        const auto PlayerController = World->SpawnActor<APlayerController>();
        if (TestNotNull(TEXT("PlayerController should spawn"), PlayerController))
        {
            const auto Action =
                FBlazeTestAsyncActionCreateWidgetAsyncFactory::Create(World->Get(), PlayerController, false);
            if (TestNotNull(TEXT("CreateWidgetAsync test action should be created"), Action))
            {
                const auto Listener =
                    NewObject<UBlazeAutomationTestCreateWidgetListener>(GetTransientPackage(), NAME_None, RF_Transient);
                if (TestNotNull(TEXT("Listener should be created"), Listener))
                {
                    AddExpectedMessagePlain(TEXT("ResumeInputForPlayer("),
                                            ELogVerbosity::Warning,
                                            EAutomationExpectedMessageFlags::Contains,
                                            1);
                    Action->OnComplete.AddDynamic(Listener, &UBlazeAutomationTestCreateWidgetListener::HandleComplete);
                    Action->OnCancelled.AddDynamic(Listener,
                                                   &UBlazeAutomationTestCreateWidgetListener::HandleCancelled);

                    Action->Activate();

                    const auto bCancelled = TestTrue(TEXT("Invalid async widget loads should broadcast cancellation"),
                                                     Listener->bCancelled);
                    const auto bCompleted =
                        TestFalse(TEXT("Invalid async widget loads should not broadcast completion"),
                                  Listener->bCompleted);
                    const auto bWidgetCreated =
                        TestNull(TEXT("Invalid async widget loads should not create a widget"), Listener->Widget);
                    return bCancelled && bCompleted && bWidgetCreated;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBlazePrimaryLayoutCancelsWhenLoadHandleInvalidTest,
                                 "Blaze.PrimaryLayout.CancelsWhenLoadHandleInvalid",
                                 BlazeAsyncLoadTests::AutomationTestFlags)
bool FBlazePrimaryLayoutCancelsWhenLoadHandleInvalidTest::RunTest(const FString&)
{
    const auto Layout = NewObject<UBlazeAutomationTestPrimaryLayout>(GetTransientPackage(), NAME_None, RF_Transient);
    if (TestNotNull(TEXT("Primary layout should be created"), Layout))
    {
        auto bWasCalled{ false };
        auto CallbackState{ EBlazePushWidgetToLayerState::AfterPush };
        UCommonActivatableWidget* CallbackWidget = reinterpret_cast<UCommonActivatableWidget*>(0x1);

        const auto Handle = Layout->PushWidgetToLayerAsync<UCommonActivatableWidget>(
            BlazeAsyncLoadTests::TestLayerTag,
            false,
            TSoftClassPtr<UCommonActivatableWidget>(),
            [&bWasCalled, &CallbackState, &CallbackWidget](const auto State, auto* Widget) {
                bWasCalled = true;
                CallbackState = State;
                CallbackWidget = Widget;
            });

        const auto bInvalidHandle =
            TestFalse(TEXT("Invalid async widget loads should not return a handle"), Handle.IsValid());
        const auto bCancelled =
            TestTrue(TEXT("Invalid async widget loads should invoke the cancellation callback"), bWasCalled);
        const auto bCanceledState = TestEqual(TEXT("Invalid async widget loads should report the canceled state"),
                                              CallbackState,
                                              EBlazePushWidgetToLayerState::Canceled);
        const auto bNullWidget =
            TestNull(TEXT("Invalid async widget loads should not provide a widget"), CallbackWidget);
        return bInvalidHandle && bCancelled && bCanceledState && bNullWidget;
    }
    else
    {
        return false;
    }
}

#endif
