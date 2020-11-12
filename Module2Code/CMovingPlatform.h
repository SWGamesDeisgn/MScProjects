#ifndef CMOVINGPLATFORM
#define CMOVINGPLATFORM
#include "PhysicsObject.h"
#include "COsscilator.h"
#include "CHackable.h"
#include "T3Helpers.h"
#include "CParticleEffect.h"
class CMovingPlatform : public CPhysicsObject, public COsscilator, public CHackable
{
public:
	CMovingPlatform();
	~CMovingPlatform();

	GCFACTORY_DECLARE_CREATABLECLASS(CMovingPlatform);

	void VOnUpdate( float fTimeStep ) override;
	virtual void VHack();
	virtual void VOnResourceAcquire();
	// void vPostCollision( b2Contact* p_cContact, bool b_isA ) override;
private:
	bool m_bInit;
	bool m_bHacked;
	CParticleEffect* m_pcParticleEffect;
};

#endif