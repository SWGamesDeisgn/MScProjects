#include "CPresidentsDaughter.h"
#include "GamerCamp/GCObject/GCObjectManager.h"
#include "proj.win32/Code/CIntermediateLayer.h"
#include "GamerCamp/GCCocosInterface/GCCocosHelpers.h"
#include "GamerCamp/GCCocosInterface/IGCGameLayer.h"
#include "proj.win32/Code/CollisionBits.h"
#include "proj.win32/Code/T3Helpers.h"
#include "proj.win32/Code/CProgressionHandler.h"
#include "proj.win32/Code/CHackable.h"
#include "proj.win32/Code/CShadow.h"

USING_NS_CC;

GCFACTORY_IMPLEMENT_CREATEABLECLASS( CPresidentsDaughter );

CPresidentsDaughter::CPresidentsDaughter() 
	: CPhysicsObject( GetGCTypeIDOf( CPresidentsDaughter ) )
{
}

CPresidentsDaughter::~CPresidentsDaughter()
{
}

void CPresidentsDaughter::vPreCollision( b2Contact* p_cContact, bool isA )
{
	// // get the fixtures
	// b2Fixture* pourCollider = nullptr;
	// b2Fixture* potherCollider = nullptr;
	// p_cContact->SetEnabled( false );
	// if ( isA )
	// {
	// 	pourCollider = p_cContact->GetFixtureA();
	// 	potherCollider = p_cContact->GetFixtureB();
	// }
	// else
	// {
	// 	pourCollider = p_cContact->GetFixtureB();
	// 	potherCollider = p_cContact->GetFixtureA();
	// }
	// // If the other collider has the mask data of the Shadow.
	// if ( potherCollider->GetFilterData().categoryBits & GetBit( ECollisionBit::EShadow ) )
	// {
	// 	CGCObjectManager::ObjectKill( this );
	// 	CHackable* pcHackable = GetHackable();
	// 	if ( pcHackable )
	// 	{
	// 		pcHackable->VHack();
	// 	}
	//
	// 	auto shadow = SafeCastToDerived<CShadow*>(static_cast<CGCObjSpritePhysics*>(potherCollider->GetUserData()));
	// 	if(shadow)
	// 	{
	// 		shadow->SendOutOfScene(  );
	// 	}
	// }
}

void CPresidentsDaughter::vPostCollision( b2Contact* p_cContact, bool b_isA )
{
	// If the other collider has the mask data of the Shadow.
	if ((b_isA ? p_cContact->GetFixtureB() : p_cContact->GetFixtureA())->GetFilterData().categoryBits & GetBit(ECollisionBit::EShadow))
	{
		// If shadow collides, kill the daughter then check if she was hackable, then if yes hack her.
		CGCObjectManager::ObjectKill(this);
		CHackable* pcHackable = GetHackable();
		if (pcHackable)
		{
			pcHackable->VHack();
		}
		// Will's method for sending shadow out of the scene.
		auto shadow = SafeCastToDerived<CShadow*>(static_cast<CGCObjSpritePhysics*>(
			(b_isA ? p_cContact->GetFixtureB()->GetBody() : p_cContact->GetFixtureA()->GetBody())->GetUserData()));
		if (shadow)
		{
			shadow->SendOutOfScene();
		}
	}
}
// This setter was created in anticipation of further logic which was not required in the final build.
void CPresidentsDaughter::RescueTheDaughter(bool bSaveDaughter)
{
	m_bIsSaved = bSaveDaughter;
}
void CPresidentsDaughter::VOnResourceAcquire()
{
	CPhysicsObject::VOnResourceAcquire();
	// Create the link between specific objects which have the same TerminalID number, in this case, if the daughter is hacked
	// then the object with the same ID will be activated/unlocked.
	CIntermediateLayer::GetInstance()->GetProgressionHandler()->CreateLink( GetFactoryCreationParams()->strExtraInfo, this );
}