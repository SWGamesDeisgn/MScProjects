#include "CShadow.h"
#include "CPlayer.h"
#include "CollisionBits.h"
#include "ShadowTerminal/CShadowTerminal.h"
#include "CHackableEnemy.h"
#include "StaticEnemy.h"
#include "CMovingEnemy.h"
#include "proj.win32/Code/PresidentsDaughter/CPresidentsDaughter.h"

const cocos2d::Vec2 k_v2PlayerOffset( 50.f, 25.f );
const cocos2d::Vec2 k_v2PlayerCommandingOffset( 0.f, 25.f );

const float k_fShadowOffsetValue = 75.f;
const float k_fShadowOffsetValueYBase = 30.f;

const cocos2d::Vec2 k_av2CommandingOffsets[5]
{
	{ -k_fShadowOffsetValue, k_fShadowOffsetValueYBase },
	{ -k_fShadowOffsetValue / sqrtf(2), k_fShadowOffsetValueYBase + k_fShadowOffsetValue / sqrtf(2) },
	{ 0.f, k_fShadowOffsetValueYBase + k_fShadowOffsetValue },
	{ k_fShadowOffsetValue / sqrtf(2), k_fShadowOffsetValueYBase + k_fShadowOffsetValue / sqrtf(2) },
	{ k_fShadowOffsetValue, k_fShadowOffsetValueYBase }
};

const float k_fShadowFollowSpeed = 0.2f;

const float k_fMaxReturnSpeed = 15.f;
const float k_fMaxReturnSpeedSQR = k_fMaxReturnSpeed * k_fMaxReturnSpeed;

IN_CPP_CREATION_PARAMS_DECLARE( CShadow, "TexturePacker/Sprites/Shadow/Shadow.plist", "Shadow", b2_dynamicBody, true );

CShadow::CShadow( CPlayer* pcPlayer )
	: CPhysicsObject( GetGCTypeIDOf( CShadow ) )
	, m_eState( EShadowState::EFollowing )
	, m_pcPlayer( pcPlayer )
	, m_bUninitialised( true )
	, m_fLastModifier( -1.f )
	, m_pcShadowTerminalHacking(nullptr)
	, m_pcEnemyHacking(nullptr)
	, m_pcDaughter(	nullptr	)
	, m_bHackedThisFrame( false )
	, m_v2Offset( k_v2PlayerOffset )
	, m_iPlayerInputx( 0 )
	, m_iPlayerInputy( 0 )
	, m_bCanReturn( false )
{
	m_pcAudioManager = new cocos2d::experimental::AudioEngine();
	cocos2d::experimental::AudioEngine::preload("Audio/PH_SHADOW/PH_SHADOW_attachOntoTerminal.mp3");
	cocos2d::experimental::AudioEngine::preload("Audio/PH_SHADOW/PH_SHADOW_commandedwhoosh.mp3");
	cocos2d::experimental::AudioEngine::preload("Audio/PH_SHADOW/PlaceHolder_Shadow_Detach_Teleport.mp3");
	cocos2d::experimental::AudioEngine::preload("Audio/PH_SHADOW/Placeholder_Shadow_Double_Jump.mp3");
	cocos2d::experimental::AudioEngine::preload("Audio/PH_SHADOW/Placeholder_Shadow_Double_Jump_Failed.mp3");
}



CShadow::~CShadow( )
{
	cocos2d::experimental::AudioEngine::end();
}

void CShadow::vPreCollision( b2Contact* p_cContact, bool b_isA )
{
	if(m_eState == EShadowState::EHacking || (m_eState == EShadowState::EReturning && !((b_isA ? p_cContact->GetFixtureB() : p_cContact->GetFixtureA())->GetFilterData().categoryBits & GetBit(ECollisionBit::EPlatform)) && !((b_isA ? p_cContact->GetFixtureB() : p_cContact->GetFixtureA())->GetFilterData().categoryBits & GetBit(ECollisionBit::ELethalObject)) ))
	{
		p_cContact->SetEnabled( false );
	}
}

void CShadow::vPostCollision( b2Contact* p_cContact, bool b_isA )
{
	if(m_eState == EShadowState::EThrown &&!m_bHackedThisFrame)
	{
		if ((b_isA ? p_cContact->GetFixtureB() : p_cContact->GetFixtureA())->GetFilterData().categoryBits & GetBit(ECollisionBit::ETerminal))
			//Hack the shadow Terminal
		{
			m_pcShadowTerminalHacking = SafeCastToDerived<CShadowTerminal*>(static_cast<CGCObjSpritePhysics*>(
				(b_isA ? p_cContact->GetFixtureB()->GetBody() : p_cContact->GetFixtureA()->GetBody())->GetUserData()));
			if (m_pcShadowTerminalHacking && !m_pcShadowTerminalHacking->IsHacked())
			{
				SetState(EShadowState::EHacking);
				SetVelocity({ 0.f, 0.f });

				m_pcShadowTerminalHacking->StartHack();
				m_bHackedThisFrame = true;
			}
			else
			{
				SetState( EShadowState::EReturning );
			}
		}
		else if( (b_isA ? p_cContact->GetFixtureB() : p_cContact->GetFixtureA())->GetFilterData().categoryBits & GetBit(ECollisionBit::ELethalObject))
		{
			m_pcEnemyHacking = SafeCastToDerived<StaticEnemy*>(static_cast<CGCObjSpritePhysics*>(
				(b_isA ? p_cContact->GetFixtureB()->GetBody() : p_cContact->GetFixtureA()->GetBody())->GetUserData()));

			if(!m_pcEnemyHacking)
			{
				m_pcEnemyHacking = SafeCastToDerived<CMovingEnemy*>(static_cast<CGCObjSpritePhysics*>(
					(b_isA ? p_cContact->GetFixtureB()->GetBody() : p_cContact->GetFixtureA()->GetBody())->GetUserData()));				
			}
			if(m_pcEnemyHacking)
			{
				SetState(EShadowState::EHacking);
				SetVelocity({ 0.f, 0.f });

				m_pcEnemyHacking->VStartHack();
				m_bHackedThisFrame = true;
				m_bCanReturn = true;
			}
			else
			{
				SetState(EShadowState::EReturning);
			}
		}
		else if ( (b_isA ? p_cContact->GetFixtureB() : p_cContact->GetFixtureA())->GetFilterData().categoryBits & GetBit( ECollisionBit::EPresidentsDaughter ) )
		{
			m_pcDaughter = SafeCastToDerived<CPresidentsDaughter*>( static_cast<CGCObjSpritePhysics*>(
				(b_isA ? p_cContact->GetFixtureB()->GetBody() : p_cContact->GetFixtureA()->GetBody())->GetUserData()) );
			m_pcDaughter->RescueTheDaughter( true );
			p_cContact->SetEnabled( false );
			SetState( EShadowState::EReturning );
		}
		else
		{
			SetState(EShadowState::EReturning);
		}
	}
}

void CShadow::VOnResourceAcquire( )
{
	IN_CPP_CREATION_PARAMS_AT_TOP_OF_VONRESOURCEACQUIRE( CShadow );
	CPhysicsObject::VOnResourceAcquire( );

	SetResetPosition( m_pcPlayer->GetResetPosition( ) );

	GetSprite()->setLocalZOrder(1);

	SetState( m_eState );
}

//Distance between shadow and player where shadow can be commanded
const float k_fCanThrowRangeSquare = 100.f * 100.f;
const float k_fCanJumpRangeSquare = 150.0f * 150.0f;
void CShadow::VOnUpdate( float fTimeStep )
{
	if ( m_bUninitialised )
	{
		GetPhysicsBody()->SetGravityScale(0.f);
	}
	m_bHackedThisFrame = false;

	switch(m_eState)
	{
	case EShadowState::EFollowing:
		//Apply velocity towards the player to follow smoothly
	{
		const float fVelocity = m_pcPlayer->GetVelocity().x;

		if (fVelocity > 0.01f)
		{
			m_fLastModifier = -1.f;
		}
		else if (fVelocity < -0.01f)
		{
			m_fLastModifier = 1.f;
		}

		//Apply offset so Shadow follows behind Danielle instead of inside her
		m_v2DistanceToPlayer = m_pcPlayer->GetSpritePosition() - GetSpritePosition() + cocos2d::Vec2(m_v2Offset.x * m_fLastModifier, m_v2Offset.y);

		SetVelocity(m_v2DistanceToPlayer * k_fShadowFollowSpeed);
	}break;
	case EShadowState::ECommanding:
		//Prepare to launch and hack a terminal
	{
		//Different offset used for this for consistency
		m_v2DistanceToPlayer = m_pcPlayer->GetSpritePosition() - GetSpritePosition() + m_v2Offset;

		SetVelocity(m_v2DistanceToPlayer * 0.5f);

	} break;
	case EShadowState::EThrown: break;
	case EShadowState::EHacking:
		//Monitor the status of the terminal hack
	{
		if ( m_pcShadowTerminalHacking )
		{
			//Apply offset so Shadow follows behind Danielle instead of inside her
			m_v2DistanceToPlayer = m_pcShadowTerminalHacking->GetSpritePosition( ) - GetSpritePosition( );

			SetVelocity( m_v2DistanceToPlayer * k_fShadowFollowSpeed );
			if ( m_pcShadowTerminalHacking->IsHacked( ) )
			{
				SetState( EShadowState::EReturning );
				m_bCanReturn = false;
				m_pcShadowTerminalHacking = nullptr;
			}
		}
	} break;
	case EShadowState::EStool:
	{
		m_v2DistanceToPlayer = m_pcPlayer->GetSpritePosition() - GetSpritePosition() + cocos2d::Vec2(0.0f, -100.0f);
		SetVelocity(m_v2DistanceToPlayer );

	} break;
	case EShadowState::EReturning:
		//Apply velocity towards the player to follow smoothly
	{
		const float fVelocity = m_pcPlayer->GetVelocity().x;

		if (fVelocity > 0.01f)
		{
			m_fLastModifier = -1.f;
		}
		else if (fVelocity < -0.01f)
		{
			m_fLastModifier = 1.f;
		}

		//Apply offset so Shadow follows behind Danielle instead of inside her
		m_v2DistanceToPlayer = m_pcPlayer->GetSpritePosition() - GetSpritePosition() + cocos2d::Vec2(m_v2Offset.x * m_fLastModifier, m_v2Offset.y);

		if( m_v2DistanceToPlayer.lengthSquared() > k_fMaxReturnSpeedSQR)
		{
			m_v2DistanceToPlayer = m_v2DistanceToPlayer / ( m_v2DistanceToPlayer.length() / k_fMaxReturnSpeedSQR );
		}
		else
		{
			SetState( EShadowState::EFollowing );
		}

		SetVelocity(m_v2DistanceToPlayer * k_fShadowFollowSpeed);
	} break;
	default: ;
	}
	if(GetVelocity().x < 0)
	{
		SetFlippedX(true);
	}
	else if(GetVelocity().x > 0)
	{
		SetFlippedX(false);
	}

}

bool CShadow::PrepareForCommand( )
{
	if(m_eState == EShadowState::EFollowing && m_v2DistanceToPlayer.getLengthSq() < k_fCanThrowRangeSquare)
		//Check if Shadow is close enough to be commanded
	{
		SetState( EShadowState::ECommanding );
		return true;
	}
	return false;
}

bool CShadow::FootStool(f32 fLerpTime)
{
	if (m_eState == EShadowState::EFollowing && m_v2DistanceToPlayer.getLengthSq() < k_fCanJumpRangeSquare)
		//Check if Shadow is close enough to be stood on
	{
		SetState(EShadowState::EStool);
		cocos2d::experimental::AudioEngine::stopAll();
		cocos2d::experimental::AudioEngine::play2d("Audio/PH_SHADOW/Placeholder_Shadow_Double_Jump.mp3", false);
		m_bCanReturn = true;
		return true;
	}
	cocos2d::experimental::AudioEngine::stopAll();
	cocos2d::experimental::AudioEngine::play2d("Audio/PH_SHADOW/Placeholder_Shadow_Double_Jump_Failed.mp3", false);
	return false;
}

const float k_fSqrtOf2       = sqrtf( 2 );
const float k_fThrowVelocity = 25.f;

//#define DEBUG_SHADOW_THROW
bool CShadow::Command( )
{
	if ( m_eState == EShadowState::ECommanding)
	{
		if(m_iPlayerInputy < 0 || (m_iPlayerInputy == 0 && m_iPlayerInputx == 0))
		{
			SetState( EShadowState::EFollowing );
			return false;
		}
		
		SetState( EShadowState::EThrown );
		cocos2d::experimental::AudioEngine::stopAll();
		cocos2d::experimental::AudioEngine::play2d("Audio/PH_SHADOW/PH_SHADOW_commandedwhoosh.mp3", false, 1.0f);
		OutputDebugStringA("Whoosh");
		if ( m_iPlayerInputx == 0 || m_iPlayerInputy == 0 )
			//Direction is horizontal or vertical
		{
			SetVelocity( { static_cast< float >(m_iPlayerInputx) * k_fThrowVelocity, static_cast< float >(m_iPlayerInputy) * k_fThrowVelocity } );
		}
		else
		   //Direction is diagonal
		{
		   //Multiply values by Sqrt(2) to get the diagonal values correct without using Pythagoras theorem.
		
		   SetVelocity( { static_cast< float >(m_iPlayerInputx) * k_fThrowVelocity * k_fSqrtOf2, static_cast< float >(m_iPlayerInputy) * k_fThrowVelocity * k_fSqrtOf2 } );
		}
		return true;
	}
	else
	{
		SetState( EShadowState::ECommanding );
	}
	
	return false;
}

Vector2i CShadow::CommandPosition( int x, int y )
{
	if (m_eState == EShadowState::ECommanding)
	{
		//Lock throw to positive y axis.
		// if ( ((y < 0 || ( x == 0 && !( y > 0 ) )) && ((m_iPlayerInputy < 0 || ( m_iPlayerInputx == 0 && !( m_iPlayerInputy > 0 ) )))))
		// {
			// x = 0;
			// y = 1;
		// }

		if((y < 0 || (y < 1 && x == 0)))
		{
			y = 0;

			if(x == 0)
			{
				y = 1;
			}

			if((m_iPlayerInputy < 0 || (m_iPlayerInputy < 1 && m_iPlayerInputx == 0)))
			{
				m_iPlayerInputx = x;
				m_iPlayerInputy = y;				
			}
		}
		else
		{
			m_iPlayerInputx = x;
			m_iPlayerInputy = y;
		}
		
		//Get the commanding position using the inputs (and some fancy looking maths weirdness)
		const int index = 2 +( m_iPlayerInputx * 2 + (m_iPlayerInputx*-m_iPlayerInputy));

		m_v2Offset = k_av2CommandingOffsets[index];
		
	}
	return {m_iPlayerInputx, m_iPlayerInputy};
}

void CShadow::CancelThrow( )
{
	SetState( EShadowState::EFollowing );
}

bool CShadow::CanReturn( ) const
{
	return m_bCanReturn;
}

void CShadow::ReturnToPlayer( )
{
	SetState( EShadowState::EFollowing );
	m_bCanReturn = false;
	if(m_pcEnemyHacking)
	{
		m_pcEnemyHacking->EndHack();
		m_pcEnemyHacking = nullptr;
	}
}

void CShadow::SendOutOfScene()
{
	m_pcPlayer->RemoveShadow();

	CGCObjectManager::ObjectKill(this);
}

void CShadow::SetState(const EShadowState& state)
{
	switch (state)
	//Set animations
	{
	case EShadowState::EFollowing:
		RunAnimationLoop(GetFactoryCreationParams()->strPlistFile.c_str(), m_v2DistanceToPlayer.x > 0.f ? "FollowingRight" : "FollowingLeft");
		cocos2d::experimental::AudioEngine::stopAll();
		cocos2d::experimental::AudioEngine::play2d("Audio/PH_SHADOW/PlaceHolder_Shadow_Detach_Teleport.mp3", false);
		m_v2Offset = k_v2PlayerOffset;
		break;
	case EShadowState::ECommanding:
		RunAnimationLoop(GetFactoryCreationParams()->strPlistFile.c_str(), m_v2DistanceToPlayer.x > 0.f ? "CommandingRight" : "CommandingLeft");
		break;
	case EShadowState::EHacking:
		RunAnimationLoop(GetFactoryCreationParams()->strPlistFile.c_str(), "ShadowHacking");
		cocos2d::experimental::AudioEngine::stopAll();
		cocos2d::experimental::AudioEngine::play2d("Audio/PH_SHADOW/PH_SHADOW_attachOntoTerminal.mp3", false, 0.5f);
		m_v2Offset = k_v2PlayerOffset;
		break;
	case EShadowState::EThrown:
		
		break;
	case EShadowState::EReturning:
		RunAnimationLoop(GetFactoryCreationParams()->strPlistFile.c_str(), m_v2DistanceToPlayer.x > 0.f ? "FollowingRight" : "FollowingLeft");
		cocos2d::experimental::AudioEngine::stopAll();
		cocos2d::experimental::AudioEngine::play2d("Audio/PH_SHADOW/PlaceHolder_Shadow_Detach_Teleport.mp3", false);
		m_v2Offset = k_v2PlayerOffset;
		break;
	default: ;
	}

	OutputDebugStringA( std::to_string(static_cast<int>(state)).c_str() );

	m_eState = state;
}
