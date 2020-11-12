#include "CWeaponChargePack.h"
#include "GamerCamp/GCObject/GCObjectManager.h"
#include "proj.win32/Code/CIntermediateLayer.h"
#include "GamerCamp/GCCocosInterface/GCCocosHelpers.h"
#include "GamerCamp/GCCocosInterface/IGCGameLayer.h"
#include "proj.win32/Code/CPlayer.h"

USING_NS_CC;

GCFACTORY_IMPLEMENT_CREATEABLECLASS( CWeaponChargePack );

CWeaponChargePack::CWeaponChargePack()
	: CPhysicsObject( GetGCTypeIDOf( CWeaponChargePack ) )
	,m_pcPlayer		( nullptr )
{

}

CWeaponChargePack::~CWeaponChargePack()
{
}
// Super Simple code which just destroys this object when the player collides with it.
void CWeaponChargePack::vPreCollision( b2Contact* p_cContact, bool isA )
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
		m_pcPlayer->SetWeapon(true);
			CGCObjectManager::ObjectKill( this );
	}
}