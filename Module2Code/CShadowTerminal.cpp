#include "CShadowTerminal.h"
#include "proj.win32/Code/CollisionBits.h"
#include "proj.win32/Code/CHackable.h"
#include "proj.win32/Code/CProgressionHandler.h"
#include "proj.win32/Code/CShadow.h"
#include "proj.win32/Code/T3Helpers.h"

USING_NS_CC;

GCFACTORY_IMPLEMENT_CREATEABLECLASS( CShadowTerminal );
//Constructor
CShadowTerminal::CShadowTerminal()
	: CPhysicsObject( GetGCTypeIDOf( CShadowTerminal ) )
	, m_bIsBeingHacked	( false )
	, m_bIsHacked		( false )
	, m_bInit			( false )
	, m_bStartHack		( false )
	, m_bPlayerDet		( false )
	, m_pcPhysLayer		( nullptr )
	, m_fTimer			( 0.f )
{
	m_pcParticleEffect = new CParticleEffect(CParticleEffect::ParticleType::TermninalHack);
}

//Destructor
CShadowTerminal::~CShadowTerminal()
{
	
	
	
}
void CShadowTerminal::Hacked()
{
	const char* pszAnim_Hacked = "STerminalHacked";
	RunAnimationLoop(m_strPlist.c_str(), pszAnim_Hacked);

	CHackable* pcHackable = GetHackable();
	if (pcHackable)
	{
		pcHackable->VHack();
	}
}
void CShadowTerminal::Neutral()
{
	const char* pszAnim_Neutral = "STerminalNeutral";
	RunAnimationLoop( m_strPlist.c_str(), pszAnim_Neutral );
}

// Originally used to enable shadow to phase through the terminal whilst still triggering a hack
// however Shadow's code didn't allow for that functionality
// That functionality required getters and setters in shadow to allow for the terminal to control 
// Shadow's state/movement
void CShadowTerminal::vPreCollision( b2Contact* p_cContact, bool isA )
{
	// get the fixtures
	b2Fixture* pourCollider = nullptr;
	b2Fixture* potherCollider = nullptr;
	// p_cContact->SetEnabled( false );
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
	// If the other collider has the filter data of the player.
	if ( potherCollider->GetFilterData().categoryBits & GetBit(ECollisionBit::EShadow) )
	{
		// m_pcShadowTerminalHacking = SafeCastToDerived<CShadowTerminal*>(static_cast<CGCObjSpritePhysics*>(
		// 	(b_isA ? p_cContact->GetFixtureB()->GetBody() : p_cContact->GetFixtureA()->GetBody())->GetUserData()));
		// if (m_pcShadowTerminalHacking && !m_pcShadowTerminalHacking->IsHacked())
		// {
		// 	SetState(EShadowState::EHacking);
		// 	SetVelocity({ 0.f, 0.f });
		//
		// 	m_pcShadowTerminalHacking->StartHack();
		// 	m_bHackedThisFrame = true;
		// }
	}
}
// PostCollision wasn't available for a long time, for a long time PreCollision was used.
// If Shadow collides with the terminal then call Shadow's PostCollision
void CShadowTerminal::vPostCollision( b2Contact* p_cContact, bool b_isA )
{
	b2Fixture* pourCollider = nullptr;
	b2Fixture* potherCollider = nullptr;
	if (b_isA)
	{
		pourCollider = p_cContact->GetFixtureA();
		potherCollider = p_cContact->GetFixtureB();
	}
	else
	{
		pourCollider = p_cContact->GetFixtureB();
		potherCollider = p_cContact->GetFixtureA();
	}
	// If the other collider has the filter data of the player.
	if (potherCollider->GetFilterData().categoryBits & GetBit(ECollisionBit::EShadow))
	{
		auto pcShadow = SafeCastToDerived<CShadow*>(static_cast<CGCObjSpritePhysics*>(
			(b_isA ?  p_cContact->GetFixtureB()->GetBody() : p_cContact->GetFixtureB()->GetBody() )->GetUserData()) );

		pcShadow->vPostCollision(p_cContact, !b_isA);
	}
}


//Just making this work Stephen pls don't hate me - Will
// Will This should be in the constructor of the terminal - Stephen
const float k_fHackTime = 3.f;

void CShadowTerminal::VOnUpdate( f32 fTimeStep )
{
	// Custom Init for the terminal, it should be in its own function
	// it got forgotten about because its old code.
	if ( !m_bInit )
	{
		m_pcPhysLayer = CIntermediateLayer::GetInstance();
		m_pcPhysLayer->SetShadowTerminal( this );
		Neutral();
		m_bInit = true;
		m_pcParticleEffect->SetSpritePosition(GetSprite()->getPosition());
	}

	if ( m_bStartHack && !m_bIsHacked )
	{
		HackShadowTerminal( fTimeStep );
	}
	m_pcParticleEffect->SetVisible(m_bIsBeingHacked);
}

// Same logic as all other terminals, after X seconds pass terminal switches to Hacked.
void CShadowTerminal::HackShadowTerminal( f32 fTimeStep )
{
	m_bIsBeingHacked = true;
	m_fTimer += fTimeStep;
	if ( m_fTimer > k_fHackTime )
	{
		m_bIsHacked = true;
		m_bIsBeingHacked = false;
		Hacked();
	}
}
bool CShadowTerminal::IsHacked()
{
	return m_bIsHacked;
}

void CShadowTerminal::StartHack( )
{
	m_bStartHack = true;
}

void CShadowTerminal::VOnResourceAcquire()
{
	CPhysicsObject::VOnResourceAcquire();

	CIntermediateLayer::GetInstance()->GetProgressionHandler()->CreateLink(GetFactoryCreationParams()->strExtraInfo, this);

	m_strPlist = GetFactoryCreationParams()->strPlistFile;

	// Not used ever I think as the Presidents Daughter is her own object for simplicity.
	m_bIsHacked = GetBoolDataFromExtraInfo( GetFactoryCreationParams(  )->strExtraInfo, "Karen:");
	
	m_pcParticleEffect->SetResetPosition(GetResetPosition());
}
// Again not used because the daughter has her own class.
bool CShadowTerminal::GetIsPresidentsDaughter( ) const
{
	return m_bIsPresidentsDaughter;
}

bool CShadowTerminal::IsBeingHacked()
{
	return m_bIsBeingHacked;
}
