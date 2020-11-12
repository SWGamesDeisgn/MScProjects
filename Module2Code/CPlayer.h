#ifndef CPlayer_h__
#define CPlayer_h__
#include "PhysicsObject.h"
#include "CharacterController.h"
#include "GamerCamp/GCCocosInterface/GCFactory_ObjSpritePhysics.h"
#include "cocos2d/cocos/2d/CCNode.h"
#include "cocos2d/cocos/2d/CCLabel.h"
#include "CIntermediateLayer.h"
#include "AudioEngine.h"
#include "Vector2i.h"
class CShadow;
class CIntermediateLayer;
class AudioEngine;
#include "GamerCamp/GCCocosInterface/GCCocosHelpers.h"
// enum of user defined input actions the class
template< typename TActionType > class TGCActionToKeyMap;
// these are the actions the player can take
enum EPlayerStates;
enum EInputs;

// anim vector type 
typedef std::vector<cocos2d::Animation*> CCAnimVec;
// the player class
class CPlayer : public CPhysicsObject
{
	

	// the states the player may be in
	enum EPlayerStates
	{
		EPS_Idle,
		EPS_Walking,
		EPS_Jumping,
		EPS_JumpFalling,
		EPS_DoubleJumpFloat,
		EPS_DoubleJumping,
		EPS_Falling,
		EPS_Firing,
		EPS_Fired,
		EPS_Dead,
		EPS_Commanding,
		EPS_Hacking,
		EPS_InTutorial
	};
private:
	cocos2d::experimental::AudioEngine*	m_pcAudioManager;
	bool m_bWeaponObtained;
	// boolean variables
	///////////////////////////
	// are we initialized?
	bool m_bInitialized;

	// do we intend on jumping
	// on this frame?
	bool m_bJumpIntent;

	// are we allowed to double - jump?
	bool m_bDoubleJump;

	// can we jump on this frame?
	bool m_bOnFloor;

	// do we want to interact
	bool m_bInteractIntent;

	//do we want to fire
	bool m_bFire;

	// are we facing left?
	bool m_bLeft;

	// can we move?
	bool m_bCanMove;

	// floating point variables
	///////////////////////////
	// how quicky are we trying
	// to move side - to side?
	f32 m_fHorizontalMotive;

	// what is our falling
	// acceleration?
	f32 m_fGravity;

	// how high can we
	// jump?
	f32 m_fJumpHeight;

	f32 m_fDoubleJumpHeight;
	f32 m_fDoubleJumpHangTime;
	f32 m_fDoubleJumpHangTimer;

	// how fast do we
	// walk / run?
	f32 m_fWalkSpeed;

	// how high were we when we last stood on floor?
	f32 m_fLastFloorHeight;

	// how far can we fall without dying
	f32 m_fFallDistance;

	// stun time for gun
	f32 m_fStunTime;

	// what state are we in
	EPlayerStates m_ePlayerState;

	// animation state
	EPlayerStates m_eAnimState;

	// value map for anim
	cocos2d::ValueMap animDict;

	// a label to show what state we are in (temp)
	cocos2d::Label* m_StateLabel;

	//the dirty flag for this label
	bool m_bDirtyLabel;
	
	//the intermediate layer we are in
	CIntermediateLayer* m_pcPhysLayer;

	// plat velocity

	b2Vec2 _platVel;
	// animations
	CCAnimVec m_vLeftAnims;
	CCAnimVec m_vRightAnims;

	f32 m_fDeathWaitTimer;

	Vector2i m_v2LastShadowVector;

	// private functions
	///////////////////////
	// get input and record
	// it in our relevant
	// variables

	void updateInput();

	// update our velocity accordingly
	void updateVelocity(f32 fTimeStep);

	// and our animation
	void updateAnimation();
	// transition between states
	void DoStateTransisition();
	// controller keymap
	TGCActionToKeyMap<EInputs>* m_pcControllerActionToKeyMap;

	// state names for debug
	const char* EPlayerStateNames[13]
	{
		"Idle",							 
		"Walking",						 
		"Jumping",						 
		"Jump Falling",					 
		"Shadow Riding",				 
		"Jumping (again)",				 
		"Falling",
		"Firing",
		"Fired",
		"Dead",							 
		"Commanding",					 
		"Hacking",						 
		"In Tutorial"					 
	};									 
	// func to add an animation
	void AddAnim(std::string anim);
	// func to set animation from state
	void SetAnim(EPlayerStates _state);

	CShadow* m_pcShadow;
public:
	void SetWeapon( bool bSetWeapon );
	// default constructor
	CPlayer();
	// declaration to make this creatable from the
	// factory
	GCFACTORY_DECLARE_CREATABLECLASS(CPlayer);
	// virtual destructor
	virtual ~CPlayer();
	// function to update the player,
	// called in GCFramework's update
	void VOnUpdate(f32 fTimeStep)override;
	void VOnLateUpdate(float fTimeStep)override;

	// set double jump
	void SetDoubleJump(bool _allowed);
	// function to kill
	void vKillPlayer();
	// function to notify us of collisions we get into
	// called in intermediatelayer's precollision
	void vPreCollision(b2Contact* p_cContact, bool isA) override;
	// resource acquire - called when we get a sprite
	void VOnResourceAcquire() override;
	// getter for interaction
	bool GetInteractIntent();
	// getter for stun time
	f32 GetStunTime();
	// are we hacking?
	bool SetHacking(bool _hacking);
	// Set move
	inline void SetCanMove(bool _move)
	{
		m_bCanMove = _move;
	}
	inline bool GetCanMove()
	{
		return m_bCanMove;
	}

	void StartTutorial();
	void EndTutorial();

	void SetIdle();

	void RemoveShadow();
	
};
#endif // CPlayer_h__
