// Fill out your copyright notice in the Description page of Project Settings.

#include "IngameScreen.h"

#include "Engine/Engine.h"

void UIngameScreen::NativeConstruct()
{
	Super::NativeConstruct();

	// By default, hide the timer
	TimerContainer->SetVisibility( ESlateVisibility::Hidden );

	// Config the options menu
	OptionsMenu->SetParentWidget( EOptionsMenuParentType::INGAME_PAUSE_MENU, this );
}

void UIngameScreen::UpdateDetection( float fNewDetectionValue )
{
	DetectionComponent->UpdateDetectionBar( fNewDetectionValue );
}

void UIngameScreen::UpdateHealth( float fNewHealthValue )
{
	HealthBar->SetPercent( fNewHealthValue );
}

void UIngameScreen::UpdateArmour(float fNewArmourValue)
{
	ArmourBar->SetPercent( fNewArmourValue );
}

void UIngameScreen::UpdateAmmo( int iCurrentAmmoCount, int iMaxAmmoCount )
{
	// Show the ammo text by setting the render opacity to 1
	if( AmmoText->GetRenderOpacity() == 0.0f )
	{
		AmmoText->SetRenderOpacity( 1.0f );
	}

	AmmoText->SetText( FText::FromString( FString::Printf( TEXT( "%i / %i" ), iCurrentAmmoCount, iMaxAmmoCount ) ) );
}

void UIngameScreen::UpdateMoney( int iCurrentMoney )
{
	MoneyText->SetText( FText::FromString( FString::Printf( TEXT( "$ %i" ), iCurrentMoney ) ) );
}

void UIngameScreen::UpdateTimer( float fRemainSeconds )
{
	FTimespan cTimeSpan = FTimespan::FromSeconds( fRemainSeconds );
	TimerText->SetText( FText::FromString( FString::Printf( TEXT( "%02d : %02d" ), cTimeSpan.GetMinutes(), cTimeSpan.GetSeconds() ) ) );

	// Hide the timer when the remain time is less than 0 second
	if( fRemainSeconds <= 0.0f )
	{
		TimerContainer->SetVisibility( ESlateVisibility::Hidden );
	}
	else
	{
		TimerContainer->SetVisibility( ESlateVisibility::Visible );
	}
}

void UIngameScreen::SetObjectiveText( FString strObjectiveText )
{
	ObjectiveComponent->ChangeText( strObjectiveText );
}

void UIngameScreen::SetObjectiveMessage( EPaydayObjectiveState eHintType )
{
	ObjectiveComponent->ChangeMessage( eHintType );
}

void UIngameScreen::ShowResultPanel()
{
	PlayAnimationByName( "ShowResult" );
	ResultPanel->Show();
}

void UIngameScreen::HideResultPanel()
{
	ResultPanel->Hide( false );
	PlayAnimationByName( "ShowResult", 0, 1, EUMGSequencePlayMode::Reverse );
}

void UIngameScreen::ShowPauseMenu()
{
	PlayAnimationByName( "ShowPauseMenu" );
}

void UIngameScreen::HidePauseMenu()
{
	PlayAnimationByName( "ShowPauseMenu", 0, 1, EUMGSequencePlayMode::Reverse );
}

void UIngameScreen::ShowOptionsMenu()
{
	OptionsMenu->Show();
	PlayAnimationByName( "ShowOptionsMenu" );
}

void UIngameScreen::HideOptionsMenu()
{
	OptionsMenu->Hide( false );
	PlayAnimationByName( "ShowOptionsMenu", 0, 1, EUMGSequencePlayMode::Reverse );
}
