#include "CTerminal.h"
#include "proj.win32/Code/CProgressionHandler.h"
#include "proj.win32/Code/CHackable.h"
#include "proj.win32/Code/CTutorialScreen.h"

USING_NS_CC;

GCFACTORY_IMPLEMENT_CREATEABLECLASS( CTerminal );
//Constructor
// Note CIntermediateLayer used to be called CPhysicsLayer.
// Small portions of the code in this class are the same or similar to that of module 1.
// Large portions are new code though, as functionality was expanded during production.
CTerminal::CTerminal()
	: CPhysicsObject( GetGCTypeIDOf( CTerminal ) )
	, m_pcCCSpHackingBarBackGround	( nullptr )
	, m_pcCCSpHackingBarForeGround	( nullptr )
	, m_pcGCSpriteHackingProgress	( nullptr )
	, m_pcGCSpriteHackingCircle		( nullptr )
	, m_iHackCounter( 3 )
	, m_bHackPause( true )
	, m_bInit( false )
	, m_bIsHacked( false )
	, m_fHackingTime( 3.0f )
	, m_bPlayerInteract( false )
	, m_bStartHack( false )
	, m_bPlayerDet( false )
	, m_pcPlayer(nullptr)
	, m_pcPhysLayer( nullptr )
	, m_pcTutorialScreen(nullptr)
{

}

//Destructor
CTerminal::~CTerminal()
{
}

// this allows the terminal to know when the player collides with it.
// also used to set the collider of the terminal to false so the player can walk through it.
void CTerminal::vPreCollision( b2Contact* p_cContact, bool isA )
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
		// Set the PlayerDetected function to true.
		PlayerDetected( true );
	}
}

void CTerminal::VOnUpdate( f32 fTimeStep )
{
	// Custom Init function
	if ( !m_bInit )
	{
		Init();
	}
	if ( !m_bIsHacked && m_bPlayerDet)
	{
		// If the player pointer isn't null then check if they are attempting to hack,
		// and flip the Interact bool else set the player's hacking and movement to false and true,
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
				m_pcPlayer = nullptr;
			}

			// If the player is in front of a terminal AND they are holding interact then run HackTerminal
			if ( m_bPlayerDet && m_bPlayerInteract )
			{
				HackTerminal( fTimeStep );
			}
		}
		// If the IsHacked bool is true then set the state of the terminal to Hacked
		// and set the player detect & interact to false.
		if ( m_bIsHacked )
		{
			Hacked();
		}
	}
}

void CTerminal::PlayerDetected( bool bHack )
{
	// Optimised :)
	m_bPlayerDet = bHack;
}
// This is even worse than what I did in CServerTerminal, Again I think this could be avoided
// If inheritance was being used for the terminals, however I didn't know how many types of
// terminal would be in the final build, currently theres 3, originally I thought there would be 2.

const char* pszPlist1 = "TexturePacker/Sprites/HackingUI/HackingProgressBar.plist";
const char* pszAnim_ProgressBar00 = "ProgressBar_00";
const char* pszAnim_ProgressBar01 = "ProgressBar_01";
const char* pszAnim_ProgressBar02 = "ProgressBar_02";
// The hacking function of the terminal, this is called when the player is in front
// of a terminal and has pressed the interact button.
void CTerminal::HackTerminal( f32 fTimeStep )
{	
	// Some of the logic here is copied/similar to the first module, 
	// however there is also logic that expands on the old logic.
	m_iHackCounter = roundf( m_fHackingTime );
	if ( m_pcPlayer != nullptr )
	{
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

				if ( m_iHackCounter == 3 )
				{
					m_pcGCSpriteHackingProgress->RunAnimation( pszPlist1, pszAnim_ProgressBar00 );
				}
				if ( m_iHackCounter == 2 )
				{
					m_pcGCSpriteHackingProgress->RunAnimation( pszPlist1, pszAnim_ProgressBar01 );
				}			
				if ( m_iHackCounter == 1 )
				{
					m_pcGCSpriteHackingProgress->RunAnimation( pszPlist1, pszAnim_ProgressBar02 );
				}
			}
		}
		if ( m_iHackCounter == 0 )
		{
			m_bIsHacked = true;
			m_pcPlayer->SetCanMove( true );
			m_pcPlayer->SetHacking( false );
			PlayerDetected( false );
			m_bPlayerInteract = false;
			HideProgressBar();
			CHackable* pcHackable = GetHackable();
			if ( pcHackable )
			{
				pcHackable->VHack();
			}
			if (m_pcTutorialScreen)
			{
				m_pcTutorialScreen->Display();
			}
		}
	}

}
// Getter to return the IsHacked bool.
bool CTerminal::IsHacked()
{
	return m_bIsHacked;
}

void CTerminal::Init()
{
	
	// Just a method to set the terminal to the neutral state on creation. 
	// This only happens ONCE per terminal.
	m_pcPhysLayer = CIntermediateLayer::GetInstance();

	m_pcPhysLayer->GetProgressionHandler()->CreateLink( GetFactoryCreationParams()->strExtraInfo, this );
	CreateProgressBar();
	m_bInit = true;

}

//The Hacked state of the terminal, reading from a plist and also checks if Will's progression handler has hit 0.
// Old Code reused and modified to use RunAnimation instead of RunAction.
void CTerminal::Hacked()
{
	const char* pszAnim_Hacked = "Hacked";
	ValueMap& rdictPList = GCCocosHelpers::CreateDictionaryFromPlist( m_strPlist.c_str() );

	RunAnimation( m_strPlist.c_str(), pszAnim_Hacked );
}

// Neutral state of the terminal, reading from plist.
// Old Code reused and modified to use RunAnimation instead of RunAction.
void CTerminal::Neutral()
{
	const char* pszAnim_Neutral = "Neutral";
	ValueMap& rdictPList = GCCocosHelpers::CreateDictionaryFromPlist( m_strPlist.c_str() );

	RunAnimation( m_strPlist.c_str(), pszAnim_Neutral );
}

// Hide all Hacking progress bar elements
void CTerminal::HideProgressBar()
{
	m_pcCCSpHackingBarBackGround->SetVisible( false );
	m_pcCCSpHackingBarForeGround->SetVisible( false );
	m_pcGCSpriteHackingProgress->SetVisible( false );
	m_pcGCSpriteHackingCircle->SetVisible( false );
}

// Show all Hacking progress bar elements
void CTerminal::ShowProgressBar()
{
	m_pcCCSpHackingBarBackGround->SetVisible( true );
	m_pcCCSpHackingBarForeGround->SetVisible( true );
	m_pcGCSpriteHackingProgress->SetVisible( true );
	m_pcGCSpriteHackingCircle->SetVisible( true );
}

// Create the background/foreground/non-moving elements of the progress bar
// Again inheritance would make this a lot cleaner as it would remove a lot of dupelicated code
// Time was an issue as this was implemented quite late into development.
void CTerminal::CreateProgressBar()
{
	const char* pszPlist =	"TexturePacker/Sprites/HackingUI/HackingProgressBarBackGround.plist";
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
	m_pcGCSpriteHackingProgress->CreateSprite( pszPlist1 );
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

void CTerminal::VOnResourceAcquire( )
{
	CPhysicsObject::VOnResourceAcquire();

	// Create Links & get extra info from OGMO
	CIntermediateLayer::GetInstance()->GetProgressionHandler()->CreateLink(GetFactoryCreationParams()->strExtraInfo, this);
	m_strPlist = GetFactoryCreationParams( )->strPlistFile;
	Neutral();
	// Tutorial for Double Jump in level 5
	if (GetTerminalID() == 64)
	{
		m_pcTutorialScreen = new CTutorialScreen(std::string("TexturePacker/Sprites/TutorialScreens/TutorialScreens.plist").c_str(), std::string("DoubleJump").c_str());
	}
}

