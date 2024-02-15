// Copyright Nathan Guerin


#include "OverheadWidget.h"

#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(const FString& TextToDisplay) const
{
	if (Text_Display)
	{
		Text_Display->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(const APawn* InPawn) const
{
	const ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString Role;
	switch (RemoteRole)
	{
	case ROLE_Authority:
		Role = FString(TEXT("Authority"));
		break;
	case ROLE_AutonomousProxy:
		Role = FString(TEXT("Autonomous Proxy"));
		break;
	case ROLE_SimulatedProxy:
		Role = FString(TEXT("SimulatedProxy"));
		break;
	case ROLE_None:
		Role = FString(TEXT("None"));
		break;
	}
	const FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	SetDisplayText(RemoteRoleString);
}
