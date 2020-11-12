// Fill out your copyright notice in the Description page of Project Settings.


#include "ResultStatPanel.h"


#include "Kismet/GameplayStatics.h"
#include "Payday/PaydayHUD.h"
#include "Payday/Core/PaydayGameInstance.h"
#include "Payday/Core/PlayerStates/PaydayPlayerState.h"

void UResultStatPanel::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind the return to main menu on click delegate
	ReturnToMainMenuButton->OnClicked.AddDynamic( this, &UResultStatPanel::OnReturnToMainMenuPressed );
}

void UResultStatPanel::Show()
{
	UpdateState();
	Super::Show();
}

void UResultStatPanel::UpdateState()
{
	// Get player state from the owner / player controller
	APaydayPlayerState* cPlayerState = Cast<APaydayPlayerState>( GetOwningPlayer()->PlayerState );

	// Apply the value to labels
	if( cPlayerState )
	{
		// Damage taken
		DamageTakenValue->SetText( FText::FromString( FString::Printf( TEXT( "%.2f" ), cPlayerState->GetPlayerDamageTaken() ) ) );

		// Collected money
		CollectedMoneyValue->SetText( FText::FromString( FString::Printf( TEXT( "%i" ), cPlayerState->GetPlayerCollectedMoney ( ) ) ) );

		// Death Count
		DeathCountValue->SetText( FText::FromString( FString::Printf( TEXT( "%i" ), cPlayerState->GetPlayerDiedCount() ) ) );

		// Kill Count
		KillCountValue->SetText( FText::FromString( FString::Printf( TEXT( "%i" ), cPlayerState->GetPlayerKillCount() ) ) );
	}
}

void UResultStatPanel::OnReturnToMainMenuPressed()
{
	if( GetWorld()->IsServer() )
	{
		UPaydayGameInstance* cGameInstance = Cast<UPaydayGameInstance>( GetWorld()->GetGameInstance() );
		cGameInstance->OnSessionDestroy.AddUObject( this, &UResultStatPanel::OnGameSessionDestroy );
		cGameInstance->DestroyRoom();
		
		APaydayHUD* cHud = Cast<APaydayHUD>( GetOwningPlayer()->GetHUD() );
		if( cHud )
		{
			cHud->ShowLoading();
			// Check if the HUD is the tutorial HUD, enabling us to go back to the main menu if true.
			if( cHud->bIsTutorialLevel )
			{
				UGameplayStatics::OpenLevel( GetWorld() , "Landing_Level" );
			}
		}
	}
	else
	{
		APaydayHUD* cHud = Cast<APaydayHUD>( GetOwningPlayer()->GetHUD() );
		if( cHud )
		{
			cHud->AsyncLoadMap( "/Game/Levels/Landing_Level", false, true, 3.0f );
			cHud->ShowLoading();
		}
	}
}

void UResultStatPanel::OnGameSessionDestroy()
{
	APaydayHUD* cHud = Cast<APaydayHUD>( GetOwningPlayer()->GetHUD() );
	if( cHud )
	{
		cHud->AsyncLoadMap( "/Game/Levels/Landing_Level", true, true, 3.0f );
	}
}
