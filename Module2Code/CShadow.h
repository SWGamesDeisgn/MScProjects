#ifndef CSHADOW
#define CSHADOW
#include "PhysicsObject.h"
#include "AudioEngine.h"
#include "Vector2i.h"
// #include "CPlayer.h"

class CHackableEnemy;
class CShadowTerminal;
class CPresidentsDaughter;
class CPlayer;

class CShadow : public CPhysicsObject
{
public:
	CShadow(CPlayer* pcPlayer);
	~CShadow();

	void vPreCollision(b2Contact* p_cContact, bool b_isA) override;
	void vPostCollision(b2Contact* p_cContact, bool b_isA) override;

	void VOnResourceAcquire() override;
	void VOnUpdate(float fTimeStep) override;
	bool PrepareForCommand();
	bool FootStool(f32 fLerpTime);
	bool Command( );
	Vector2i CommandPosition(int x, int y);
	void CancelThrow();

	bool CanDoubleJump();

	bool CanReturn() const;
	void ReturnToPlayer();

	void SendOutOfScene();

	// void GiveTerminal(CShadowTerminal* pcShadowTerminal);

private:
	cocos2d::experimental::AudioEngine*	m_pcAudioManager;

	enum class EShadowState{ EFollowing, ECommanding, EThrown, EHacking, EStool, EReturning };

	void SetState(const EShadowState& state );

	EShadowState m_eState;
	
	CPlayer* m_pcPlayer;

	// bool m_bAboutToThrow;
	
	// bool m_bThrown;
	// bool m_bHacking;
	// bool m_bCanThrow;

	bool m_bUninitialised;

	float m_fLastModifier;

	cocos2d::Vec2 m_v2DistanceToPlayer;

	CShadowTerminal* m_pcShadowTerminalHacking;
	CHackableEnemy* m_pcEnemyHacking;
	CPresidentsDaughter* m_pcDaughter;
	bool m_bHackedThisFrame;

	cocos2d::Vec2 m_v2Offset;

	int m_iPlayerInputx;
	int m_iPlayerInputy;

	bool m_bCanReturn;

};

#endif
