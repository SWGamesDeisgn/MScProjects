// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PaydayHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "CanvasItem.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "Core/PlayerStates/PaydayPlayerState.h"
#include "Kismet/GameplayStatics.h"

APaydayHUD::APaydayHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;

	// Get the default theme
	static ConstructorHelpers::FObjectFinder<USlateWidgetStyleAsset> DefaultThemeObj( TEXT( "/Game/Blueprints/UI/Styles/PaydayGameTheme" ) );
	if( DefaultThemeObj.Succeeded() )
	{
		cDefaultThemeClass = DefaultThemeObj.Object;
		cDefaultTheme = Cast<UPaydayGameThemeWidgetStyle>( cDefaultThemeClass->CustomStyle );
	}
}


void APaydayHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X - (CrosshairTex->GetSizeX() / 2)),
										   (Center.Y - (CrosshairTex->GetSizeY() / 2)));
	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );
	
}

void APaydayHUD::BeginPlay()
{
	Super::BeginPlay();

	// Create Ingame Screen
	if( IngameScreenClass )
	{
		ingameScreen = Cast<UIngameScreen>( CreateWidget( GetWorld(), IngameScreenClass ) );
		if ( ingameScreen )
		{
			ingameScreen->Show();
		}
	}

	// If is tutorial level init the objective hints to TUTORIAL_FIND_KEY
	if( bIsTutorialLevel )
	{
		ingameScreen->SetObjectiveMessage( EPaydayObjectiveState::TUTORIAL_FIND_KEY_CARD );
	}

	// Bind the key actions to toggle the pause menu
	const FKey cKey = EKeys::Escape;
	GetOwningPlayerController()->InputComponent->BindKey( cKey, EInputEvent::IE_Pressed, this, &APaydayHUD::OnEscapeKeyPressed );
}

void APaydayHUD::OnEscapeKeyPressed()
{
	switch( eState )
	{
		case EPaydayHUDState::IN_GAME_SCREEN:
			eState = EPaydayHUDState::SHOWING_PAUSE_MENU;
			ShowPauseMenu();
		break;
		case EPaydayHUDState::SHOWING_PAUSE_MENU:
			eState = EPaydayHUDState::IN_GAME_SCREEN;
			HidePauseMenu();
		break;
		default:
			// Do nothing if the state is not ingame screen or showing pause menu
		break;
	}
}

UPaydayGameThemeWidgetStyle* APaydayHUD::GetTheme()
{
	return cDefaultTheme;
}

void APaydayHUD::UpdateDetection( float fDetectionPercentage )
{
	if( ingameScreen )
	{
		ingameScreen->UpdateDetection( fDetectionPercentage );
	}
}

void APaydayHUD::UpdateHealth( float fNewHealthPercentage )
{
	if( ingameScreen )
	{
		ingameScreen->UpdateHealth( fNewHealthPercentage );
	}
}

void APaydayHUD::UpdateArmour( float fNewArmourPercentage )
{
	if( ingameScreen )
	{
		ingameScreen->UpdateArmour( fNewArmourPercentage );
	}
}

void APaydayHUD::UpdateAmmo( int iCurrentAmmoCount, int iMaxAmmoCount )
{
	if( ingameScreen )
	{
		ingameScreen->UpdateAmmo( iCurrentAmmoCount, iMaxAmmoCount );
	}
}

void APaydayHUD::UpdateMoney( int iCurrentMoney )
{
	if( ingameScreen )
	{
		ingameScreen->UpdateMoney( iCurrentMoney );
	}
}

void APaydayHUD::UpdateTimer( float fRemainSeconds )
{
	if( ingameScreen )
	{
		ingameScreen->UpdateTimer( fRemainSeconds );
	}
}

void APaydayHUD::SetObjectiveText( FString strObjectiveText )
{
	if( ingameScreen )
	{
		ingameScreen->SetObjectiveText( strObjectiveText );
	}
}

void APaydayHUD::SetObjectiveMessage( EPaydayObjectiveState eHintType)
{
	if( ingameScreen )
	{
		ingameScreen->SetObjectiveMessage( eHintType );
	}
}

void APaydayHUD::ShowResultPanel()
{
	if( ingameScreen )
	{
		ingameScreen->ShowResultPanel();

		// Set the input mode to UI only
		GetOwningPlayerController()->SetInputMode( FInputModeUIOnly() );
		GetOwningPlayerController()->bShowMouseCursor = true;
	}
}

void APaydayHUD::HideResultPanel()
{
	if( ingameScreen )
	{
		ingameScreen->HideResultPanel();
	}
}

void APaydayHUD::ShowPauseMenu()
{
	if( ingameScreen )
	{
		ingameScreen->ShowPauseMenu();

		// Switch UI state
		eState = EPaydayHUDState::SHOWING_PAUSE_MENU;
		
		// Set input mode to UI only and show cursor
		GetOwningPlayerController()->SetInputMode( FInputModeGameAndUI() );
		GetOwningPlayerController()->bShowMouseCursor = true;

		// Pause the game
		UGameplayStatics::SetGamePaused( GetWorld(), true );
	}
}

void APaydayHUD::HidePauseMenu()
{
	if( ingameScreen )
	{
		ingameScreen->HidePauseMenu();

		// Switch UI state
		eState = EPaydayHUDState::IN_GAME_SCREEN;

		// Set input mode back to game only and hide cursor
		GetOwningPlayerController()->SetInputMode( FInputModeGameOnly() );
		GetOwningPlayerController()->bShowMouseCursor = false;

		// Resume the game
		UGameplayStatics::SetGamePaused( GetWorld(), false );
	}
}

void APaydayHUD::ShowOptionsMenu()
{
	// Only showing the pause menu can show the options menu
	if( eState != EPaydayHUDState::SHOWING_PAUSE_MENU )
	{
		return;
	}

	if( ingameScreen )
	{
		ingameScreen->ShowOptionsMenu();
		eState = EPaydayHUDState::SHOWING_OPTIONS_MENU;
	}
}

void APaydayHUD::HideOptionsMenu()
{
	// Only showing the options menu can hide the options menu
	if( eState != EPaydayHUDState::SHOWING_OPTIONS_MENU )
	{
		return;
	}

	if( ingameScreen )
	{
		ingameScreen->HideOptionsMenu();
		eState = EPaydayHUDState::SHOWING_PAUSE_MENU;
	}
}

void APaydayHUD::DebugCLI_UpdateHealth( float fNewHealthPercentage )
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, FString::Printf( TEXT( "CLI - UpdateHealth is called, Health Percentage: %.2f" ), fNewHealthPercentage ) );
	UpdateHealth( fNewHealthPercentage );
}

void APaydayHUD::DebugCLI_UpdateDetection( float fNewDetectionPercentage )
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, FString::Printf( TEXT( "CLI - Update Detection is called, Detection Percentage: %.2f" ), fNewDetectionPercentage ) );
	UpdateDetection( fNewDetectionPercentage );
}

void APaydayHUD::DebugCLI_UpdateArmour( float fNewArmourPercentage )
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, FString::Printf( TEXT( "CLI - Update Armour is called, Armour Percentage: %.2f" ), fNewArmourPercentage ) );
	UpdateArmour( fNewArmourPercentage );
}

void APaydayHUD::DebugCLI_UpdateAmmo( int iCurrentAmmoCount, int iMaxAmmoCount )
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, FString::Printf( TEXT( "CLI - Update Ammo is called, Current Ammo: %i, Max Ammo: %i" ), iCurrentAmmoCount, iMaxAmmoCount ) );
	UpdateAmmo( iCurrentAmmoCount, iMaxAmmoCount );
}

void APaydayHUD::DebugCLI_UpdateMoney(int iCurrentMoney)
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, FString::Printf( TEXT( "CLI - Update Money is called, Current Money: %i" ), iCurrentMoney ) );
	UpdateMoney( iCurrentMoney );
}

void APaydayHUD::DebugCLI_ShowTimer( float fRemainSeconds )
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, FString::Printf( TEXT( "CLI - Show Timer is called, Remain Seconds: %.2f" ), fRemainSeconds ) );

	// Disable the previous timer and set the UI to 0 seconds
	if( GetWorldTimerManager().IsTimerActive( cDebugTimerHandle ) )
	{
		GetWorldTimerManager().ClearTimer( cDebugTimerHandle );
		UpdateTimer( 0 );
	}
	
	// Start debug timer
	GetWorldTimerManager().SetTimer( cDebugTimerHandle, this, &APaydayHUD::Debug_OnTimerFinished, fRemainSeconds );
	UpdateTimer( fRemainSeconds );
}

void APaydayHUD::DebugCLI_SetObjectiveText( FString strObjectiveText )
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, FString::Printf( TEXT( "CLI - Set Objective Text: %s" ), *strObjectiveText ) );
	SetObjectiveText( strObjectiveText );
}

void APaydayHUD::DebugCLI_SetObjectiveHint( uint8 iHintType )
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "CLI - Update the objective hint message" ) );
	EPaydayObjectiveState eType = static_cast< EPaydayObjectiveState >( iHintType );
	SetObjectiveMessage( eType );
}

void APaydayHUD::DebugCLI_RecordPlayerDamageTaken( float fNewDamageTaken )
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "CLI - Record player damage taken" ) );
	APaydayPlayerState* cPlayerState = Cast<APaydayPlayerState>( GetOwningPlayerController()->PlayerState );
	if( cPlayerState )
	{
		cPlayerState->RecordPlayerDamageTaken( fNewDamageTaken );
	}
}

void APaydayHUD::DebugCLI_RecordPlayerCollectedMoney( int iAmount )
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "CLI - Record player collected money" ) );
	APaydayPlayerState* cPlayerState = Cast<APaydayPlayerState>( GetOwningPlayerController()->PlayerState );
	if( cPlayerState )
	{
		cPlayerState->RecordPlayerCollectedMoney( iAmount );
	}
}

void APaydayHUD::DebugCLI_RecordPlayerDeathCount()
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "CLI - Record player death count" ) );
	APaydayPlayerState* cPlayerState = Cast<APaydayPlayerState>( GetOwningPlayerController()->PlayerState );
	if( cPlayerState )
	{
		cPlayerState->RecordPlayerDied();
	}
}

void APaydayHUD::DebugCLI_ShowResultPanel()
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "CLI - Show result panel" ) );
	ShowResultPanel();
}

void APaydayHUD::DebugCLI_HideResultPanel()
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "CLI - Hide result panel" ) );
	HideResultPanel();
}

void APaydayHUD::DebugCLI_PauseGame()
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "CLI - Pause game" ) );
	ShowPauseMenu();
}

void APaydayHUD::DebugCLI_ResumeGame()
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "CLI - Resume game" ) );
	HidePauseMenu();
}

void APaydayHUD::DebugCLI_RecordPlayerKillCount( int iToAddKillCount )
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, FString::Printf( TEXT( "CLI - Record Player Kill Count: %i" ), iToAddKillCount ) );
	APaydayPlayerState* cPlayerState = GetOwningPlayerController()->GetPlayerState<APaydayPlayerState>();
	if( cPlayerState )
	{
		GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "Nani" ) );
		cPlayerState->AddKillCountToRecord( iToAddKillCount );
	}
}

void APaydayHUD::Debug_OnTimerFinished()
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Red, TEXT( "Debug Timer - Debug Main Timer Finished" ) );
}
