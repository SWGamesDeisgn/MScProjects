#include "CServerTerminal.h"

#include "GamerCamp/GCObject/GCObjectManager.h"
#include "GamerCamp/GCCocosInterface/GCCocosHelpers.h"
#include "GamerCamp/GCCocosInterface/IGCGameLayer.h"
#include "proj.win32/Code/CInGameUILayer.h"
#include "proj.win32/Code/CIntermediateLayer.h"
#include "proj.win32/Code/CProgressionHandler.h"
#include "proj.win32/Code/T3Helpers.h"
#include "proj.win32/Code/CTutorialScreen.h"


USING_NS_CC;

GCFACTORY_IMPLEMENT_CREATEABLECLASS( CServerTerminal );

//Constructor

// Note CIntermediateLayer used to be called CPhysicsLayer.
// Some duplicate code here from CTerminal as I was unaware,
// that the server terminal would be a different asset.
CServerTerminal::CServerTerminal()
	: CPhysicsObject( GetGCTypeIDOf( CServerTerminal ) )
	, m_pcCCSpHackingBarBackGround( nullptr )
	, m_pcCCSpHackingBarForeGround( nullptr )
	, m_pcGCSpriteHackingProgress( nullptr )
	, m_pcGCSpriteHackingCircle( nullptr )
	, m_pcGCSprStoryBeat( nullptr )
	, m_bStoryBeatVis( false )
	, m_bHasBeatOccured( false )
	, m_iBeatNo( 0 )
	, m_iHackCounter( 3 )
	, m_bHackPause( true )
	, m_bInit( false )
	, m_bIsHacked( false )
	, m_bPlayerInteract( false )
	, m_bStartHack( false )
	, m_bPlayerDet( false )
	, m_fHackingTime( 3.0f )
	, m_pcPhysLayer( nullptr )
	, m_pcPlayer( nullptr )
{
}

//Destructor
CServerTerminal::~CServerTerminal()
{
}
// This allows the terminal to know when the player collides with it.
// Also used to set the collider of the terminal to false so the player can walk through it.
void CServerTerminal::vPreCollision( b2Contact* p_cContact, bool isA )
{
	// get the fixtures
	b2Fixture* pourCollider = nullptr;
	b2Fixture* potherCollider = nullptr;
	p_cContact->SetEnabled( false );
	if ( isA )
	{
		pourCollider = p_cContact->GetFixtureA();
		potherCollider = p_cContact->GetFixtureB();
	}
	else
	{
		pourCollider = p_cContact->GetFixtureB();
		potherCollider = p_cContact->GetFixtureA();
	}
	// If the other collider has the mask data of the player.
	if ( potherCollider->GetFilterData().categoryBits & (1) )
	{
		m_pcPlayer = SafeCastToDerived<CPlayer*>( static_cast<CGCObjSpritePhysics*>(
			(isA ? p_cContact->GetFixtureB()->GetBody() : p_cContact->GetFixtureA()->GetBody())->GetUserData()) );
		PlayerDetected( true );
	}
}
void CServerTerminal::VOnUpdate( f32 fTimeStep )
{
	// Custom initiation function
	if ( !m_bInit )
	{
		Init();
	}
	if ( !m_bIsHacked && !m_bHasBeatOccured )
	{
		// If the player pointer isn't null then check if they are attempting to hack
		// and flip the Interact bool else set the player's hacking and movement to false and true
		// enabling the player to move again.
		if ( m_pcPlayer != nullptr )
		{
			if ( m_pcPlayer->GetInteractIntent() )
			{
				m_bPlayerInteract = !m_bPlayerInteract;
			}
			if ( !m_bPlayerInteract )
			{
				m_pcPlayer->SetCanMove( true );
				m_pcPlayer->SetHacking( false );
				PlayerDetected( false );
				HideProgressBar();
			}
			if ( !m_bPlayerDet )
			{
				m_bPlayerInteract = false;
			}
			// If the player is in front of a terminal AND they are holding interact then run HackTerminal

			// If the IsHacked bool is true then set the state of the terminal to Hacked
			// and set the player detect & interact to false.
			if ( m_bIsHacked )
			{
				m_bPlayerDet = false;
				m_bPlayerInteract = false;
			}
			if ( m_bPlayerDet && m_bPlayerInteract )
			{
				HackTerminal( fTimeStep );
			}
		}

	}	
	// If the terminal has been hacked then the next time the player
	// presses Circle the storybeat will close and allow the player to move again.
	if ( m_bStoryBeatVis && !m_bHasBeatOccured )
	{
		m_pcPlayer->SetCanMove( false );
		if ( m_pcPlayer->GetInteractIntent() )
		{
			m_pcPlayer->SetCanMove( true );
			m_bStoryBeatVis = false;
			m_pcGCSprStoryBeat->SetVisible( false );
			m_bHasBeatOccured = true;
			m_pcPlayer = nullptr;
			// Will added the restartTimer call to this object
			CIntermediateLayer::GetInstance()->VRestartTimer();
		}
	}

}

void CServerTerminal::PlayerDetected( bool bHack )
{
	m_bPlayerDet = bHack;
}
// Dirty way of making the progressbar data available to the entire class.
// Would use inheritance to make this available in the future
const char* pszPlist0 = "TexturePacker/Sprites/HackingUI/HackingProgressBar.plist";
const char* pszAnim_ProgressBar0 = "ProgressBar_00";
const char* pszAnim_ProgressBar1 = "ProgressBar_01";
const char* pszAnim_ProgressBar2 = "ProgressBar_02";

// The hacking function of the terminal, this is called when the player is in front
// of a terminal and has pressed the interact button.
void CServerTerminal::HackTerminal( f32 fTimeStep )
{
	m_iHackCounter = roundf( m_fHackingTime );
	if ( m_bPlayerDet && m_bPlayerInteract )
	{
		// Set the player's CanMove to false and Hacking to true, this prevents the player from moving unless they
		// press the interact button again.
		m_pcPlayer->SetCanMove( false );
		m_pcPlayer->SetHacking( true );
		ShowProgressBar();
		if ( m_iHackCounter != 0 )
		{		
			// m_iHackCounter can be used to write to a label to display the time remaining until hacked :)
			m_iHackCounter = roundf( m_fHackingTime );
			// subtract fTimestep from m_fHackTimer and then update the iHackCounter
			m_fHackingTime -= fTimeStep;
		}
		// Based on the value of the counter run a specific animation frame for the hacking progress.
		if ( m_iHackCounter == 3 )
		{
			m_pcGCSpriteHackingProgress->RunAnimation( pszPlist0, pszAnim_ProgressBar0 );
		}
		if ( m_iHackCounter == 2 )
		{
			m_pcGCSpriteHackingProgress->RunAnimation( pszPlist0, pszAnim_ProgressBar1 );
		}
		if ( m_iHackCounter == 1 )
		{
			m_pcGCSpriteHackingProgress->RunAnimation( pszPlist0, pszAnim_ProgressBar2 );
		}
	}
	// when counter is 0 hide the progress bar assets, spawn a story beat if set in ogmo extra data else spawn nothing
	// and allow the player to continue playing.
	if ( m_iHackCounter == 0 )
	{
		HideProgressBar();
		if ( !m_bHasBeatOccured && m_iBeatNo > 0 )
		{		
			m_bStoryBeatVis = true;
			m_pcPlayer->SetHacking( false );
			CreateBeat();	
		}
		CHackable* pcHackable = GetHackable();
		if ( pcHackable )
		{
			pcHackable->VHack();
		}
		m_bIsHacked = true;
		if ( m_iBeatNo <= 0 )
		{
			m_pcPlayer->SetCanMove( true );
			m_pcPlayer->SetHacking( false );
		}
	}
}

// Getter to return the IsHacked bool.
bool CServerTerminal::IsHacked()
{
	return m_bIsHacked;
}

void CServerTerminal::Init()
{
	// Just a method to set the terminal to the neutral state on creation. 
	// This only happens ONCE per terminal.
	m_pcPhysLayer = CIntermediateLayer::GetInstance();
	// Create any links to objects with the same TerminalIDs.
	CIntermediateLayer::GetInstance()->GetProgressionHandler()->CreateLink( GetFactoryCreationParams()->strExtraInfo, this );
	// Grab the extra info from OGMO,
	m_iBeatNo = GetIntDataFromExtraInfo( GetFactoryCreationParams()->strExtraInfo, "StoryBeat:" );
	m_pcPhysLayer->GetProgressionHandler()->CreateLink( GetFactoryCreationParams()->strExtraInfo, this );
	CreateProgressBar();
	m_bInit = true;

}

//create the storybeat if ogmo extra data is greater than 0 and set the z order to 10
// A rather applicable use of a switch statement, based on the number specified in the ExtraData from OGMO,
// Set the visible storybeat animation accordingly, Beats to Variables are off by 1 due to frames starting from 00,
// and if 0 is in the extra data then default storybeat is shown.
void CServerTerminal::CreateBeat()
{
	const char* pszAnim_Beat00 = "StoryBeat01";
	const char* pszAnim_Beat01 = "StoryBeat02";
	const char* pszAnim_Beat02 = "StoryBeat03";
	const char* pszAnim_Beat03 = "StoryBeat04";
	const char* pszPlist = "TexturePacker/Sprites/StoryBeats/StoryBeats.plist";
	auto visibleSize = Director::getInstance()->getVisibleSize();
	m_pcGCSprStoryBeat = new CGCObjSprite();
	m_pcGCSprStoryBeat->CreateSprite( pszPlist );
	m_pcGCSprStoryBeat->GetSprite()->setLocalZOrder( 10 );
	m_pcGCSprStoryBeat->SetSpritePosition( Vec2( visibleSize.width / 2, visibleSize.height / 2 ) );
	m_pcGCSprStoryBeat->SetParent( IGCGameLayer::ActiveInstance() );
	
	switch ( m_iBeatNo )
	{
	case 1:
		m_pcGCSprStoryBeat->RunAnimation( pszPlist, pszAnim_Beat00 );
		m_pcGCSprStoryBeat->GetSprite()->setLocalZOrder( 10 );
		break;
	case 2:
		m_pcGCSprStoryBeat->RunAnimation( pszPlist, pszAnim_Beat01 );
		m_pcGCSprStoryBeat->GetSprite()->setLocalZOrder( 10 );
		break;
	case 3:
		m_pcGCSprStoryBeat->RunAnimation( pszPlist, pszAnim_Beat02 );
		m_pcGCSprStoryBeat->GetSprite()->setLocalZOrder( 10 );
		break;
	case 4:
		m_pcGCSprStoryBeat->RunAnimation( pszPlist, pszAnim_Beat03 );
		m_pcGCSprStoryBeat->GetSprite()->setLocalZOrder( 10 );
		break;
	default:
		break;
	}

	CIntermediateLayer::GetInstance(  )->VPauseTimer();
}

// Simple function to hide all of the elements of the hacking progress bar.
// Makes it less tedious to hide them all.
void CServerTerminal::HideProgressBar()
{
	m_pcCCSpHackingBarBackGround->SetVisible( false );
	m_pcCCSpHackingBarForeGround->SetVisible( false );
	m_pcGCSpriteHackingProgress->SetVisible( false );
	m_pcGCSpriteHackingCircle->SetVisible( false );
}

// Simple function to show all of the elements of the hacking progress bar.
// Makes it less tedious to show them all.
void CServerTerminal::ShowProgressBar()
{
	m_pcCCSpHackingBarBackGround->SetVisible( true );
	m_pcCCSpHackingBarForeGround->SetVisible( true );
	m_pcGCSpriteHackingProgress->SetVisible( true );
	m_pcGCSpriteHackingCircle->SetVisible( true );
}

// create all of the sprites for the hacking progress bar using GCSprites.
void CServerTerminal::CreateProgressBar()
{
	const char* pszPlist = "TexturePacker/Sprites/HackingUI/HackingProgressBarBackGround.plist";
	const char* pszAnim_HPB_00 = "HPB_BackGround_00";
	const char* pszAnim_HPB_01 = "HPB_ForeGround_01";
	const char* pszAnim_HPB_02 = "HPB_Circle_02";

	m_pcCCSpHackingBarBackGround = new CGCObjSprite();
	m_pcCCSpHackingBarBackGround->CreateSprite( pszPlist );
	m_pcCCSpHackingBarBackGround->GetSprite()->setLocalZOrder( 9 );
	m_pcCCSpHackingBarBackGround->SetSpriteScale( 0.25f, 0.25f );
	m_pcCCSpHackingBarBackGround->SetSpritePosition( Vec2( this->GetSpritePosition().x, this->GetSpritePosition().y + 120.0f ) );
	m_pcCCSpHackingBarBackGround->SetParent( IGCGameLayer::ActiveInstance() );
	m_pcCCSpHackingBarBackGround->RunAnimation( pszPlist, pszAnim_HPB_00 );

	m_pcGCSpriteHackingProgress = new CGCObjSprite();
	m_pcGCSpriteHackingProgress->CreateSprite( pszPlist0 );
	m_pcGCSpriteHackingProgress->GetSprite()->setLocalZOrder( 10 );
	m_pcGCSpriteHackingProgress->SetSpriteScale( 0.25f, 0.25f );
	m_pcGCSpriteHackingProgress->SetSpritePosition( Vec2( this->GetSpritePosition().x, this->GetSpritePosition().y + 120.0f ) );
	m_pcGCSpriteHackingProgress->SetParent( IGCGameLayer::ActiveInstance() );

	m_pcGCSpriteHackingCircle = new CGCObjSprite();
	m_pcGCSpriteHackingCircle->CreateSprite( pszPlist );
	m_pcGCSpriteHackingCircle->GetSprite()->setLocalZOrder( 11 );
	m_pcGCSpriteHackingCircle->SetSpriteScale( 0.25f, 0.25f );
	m_pcGCSpriteHackingCircle->SetSpritePosition( Vec2( this->GetSpritePosition().x, this->GetSpritePosition().y + 120.0f ) );
	m_pcGCSpriteHackingCircle->SetParent( IGCGameLayer::ActiveInstance() );
	m_pcGCSpriteHackingCircle->RunAnimation( pszPlist, pszAnim_HPB_02 );

	m_pcCCSpHackingBarForeGround = new CGCObjSprite();
	m_pcCCSpHackingBarForeGround->CreateSprite( pszPlist );
	m_pcCCSpHackingBarForeGround->GetSprite()->setLocalZOrder( 12 );
	m_pcCCSpHackingBarForeGround->SetSpriteScale( 0.25f, 0.25f );
	m_pcCCSpHackingBarForeGround->SetSpritePosition( Vec2( this->GetSpritePosition().x, this->GetSpritePosition().y + 120.0f ) );
	m_pcCCSpHackingBarForeGround->SetParent( IGCGameLayer::ActiveInstance() );
	m_pcCCSpHackingBarForeGround->RunAnimation( pszPlist, pszAnim_HPB_01 );
	HideProgressBar();
}

void CServerTerminal::VOnResourceAcquire()
{
	CPhysicsObject::VOnResourceAcquire();
}