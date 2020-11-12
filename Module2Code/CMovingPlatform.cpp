#include "CMovingPlatform.h"
#include "CIntermediateLayer.h"
#include "ShadowTerminal/CShadowTerminal.h"
#include "CIntermediateLayer.h"
#include "CProgressionHandler.h"
cocos2d::Vec2 k_v2Velocity( 20.f, 0.f );

// implementing factory creatable
GCFACTORY_IMPLEMENT_CREATEABLECLASS( CMovingPlatform );

CMovingPlatform::CMovingPlatform( )
	: CPhysicsObject( GetGCTypeIDOf( CMovingPlatform ) )
	, COsscilator(this)
	, m_bInit( false )
	, m_bHacked(false)
{
	m_pcParticleEffect = new CParticleEffect(CParticleEffect::ParticleType::Platform);
}
void CMovingPlatform::VHack()
{
	m_bHacked = true;
}

CMovingPlatform::~CMovingPlatform( )
{
}
void CMovingPlatform::VOnResourceAcquire()
{
	CGCObjSpritePhysics::VOnResourceAcquire();
	CIntermediateLayer::GetInstance()->GetProgressionHandler()->CreateLink(GetFactoryCreationParams()->strExtraInfo, this);

	
}
void CMovingPlatform::VOnUpdate( float fTimeStep )
{
	if ( m_bInit )
	{
		if ( m_bHacked )
		{
			
			COsscilator::VOnUpdate(fTimeStep);
			
		}
		CPhysicsObject::VOnUpdate(fTimeStep);
	}
	else
	{
		GetBody( )->SetGravityScale( 0.f );
		SetMotion(b2Vec2(35, 0));
		f32 x = GetFloatDataFromExtraInfo(GetFactoryCreationParams()->strExtraInfo, "MotionX:");
		if (x != NANINT)
		{
			SetMotion(b2Vec2(x * 2, GetMotion().y));
		}
		f32 y = GetFloatDataFromExtraInfo(GetFactoryCreationParams()->strExtraInfo, "MotionY:");
		if (y != NANINT)
		{
			SetMotion(b2Vec2(GetMotion().x * 2, y));
		}
		f32 t = GetFloatDataFromExtraInfo(GetFactoryCreationParams()->strExtraInfo, "MoveTime:");
		if (t != NANINT )
		{
			SetMoveTime(t);
		}
		f32 h = GetFloatDataFromExtraInfo(GetFactoryCreationParams()->strExtraInfo, "HoldTime:");
		if (h != NANINT)
		{
			SetHoldTime(h);
		}
		m_bInit = true;
		VOnStart();
		
		//SetHoldTime(1.0f);
		//SetMoveTime(3.0f);
		//SetTerminalID(1);
	}
	m_pcParticleEffect->GetBody()->SetTransform(GetPhysicsBody()->GetPosition(), 0.0f);
}


