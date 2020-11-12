#include "CPlayer.h"

#include "GamerCamp/GCCocosInterface/IGCGameLayer.h"
#include "GamerCamp/GameController/GCController.h"
#include "CShadow.h"

#include "GamerCamp/GameSpecific/GCGameLayerPlatformer.h"
#include "AppDelegate.h"
#include <iostream>
#include "CUIMainMenu.h"
#include "CollisionBits.h"
#include "Levels/CLevel1.h"


const float k_fDeathWaitTime = 0.5f;


enum EInputs
{
	EPA_AxisMove_X,
	EPA_AxisMove_Y,
	EPA_DLeft,
	EPA_DRight,
	EPA_ButtonFire,
	EPA_Jump,
	EPA_Interact,
	EPA_RightStick_X,
	EPA_RightStick_Y,
	EPA_ShadowThrow
};
// action map arrays must match in length - in the templated controller class we use they map from the user define enum to cocos2d::Controller::Key 
static EInputs  s_aePlayerActions[] =		 { EPA_AxisMove_X,								EPA_AxisMove_Y,								EPA_ButtonFire,						 EPA_Jump,							 EPA_Interact,						 EPA_RightStick_X,								EPA_RightStick_Y							, EPA_ShadowThrow,           EPA_DLeft,                                 EPA_DRight };
static cocos2d::Controller::Key	s_aeKeys[] = { cocos2d::Controller::Key::JOYSTICK_LEFT_X,	cocos2d::Controller::Key::JOYSTICK_LEFT_Y,	cocos2d::Controller::Key::BUTTON_X , cocos2d::Controller::Key::BUTTON_A, cocos2d::Controller::Key::BUTTON_B, cocos2d::Controller::Key::JOYSTICK_RIGHT_X,	cocos2d::Controller::Key::JOYSTICK_RIGHT_Y	, Controller::Key::BUTTON_Y, cocos2d::Controller::Key::BUTTON_DPAD_LEFT, cocos2d::Controller::Key::BUTTON_DPAD_RIGHT};
// cocos namespace
USING_NS_CC;
// implementing factory creatable
GCFACTORY_IMPLEMENT_CREATEABLECLASS( CPlayer );

// constructor
CPlayer::CPlayer()
	: CPhysicsObject(GetGCTypeIDOf(CPlayer))
	, m_pcAudioManager(nullptr)
	, m_bWeaponObtained ( false )
	, m_bInitialized(false)
	, m_bJumpIntent(false)
	, m_bDoubleJump(false)
	, m_bOnFloor(false)
	, m_bInteractIntent(false)
	, m_bFire(false)
	, m_bCanMove(true)
	, m_fHorizontalMotive(0.0f)
	, m_fGravity(-40.0f)
	, m_fJumpHeight(14.0f)
	, m_fDoubleJumpHeight(9.0f)
	, m_fDoubleJumpHangTime(0.18f)
	, m_fDoubleJumpHangTimer(0.0f)
	, m_fWalkSpeed(20.0f)
	, m_fLastFloorHeight(0.0f)
	, m_fFallDistance(32767.0f)
	, m_fStunTime(3.0f)
	, m_ePlayerState(EPS_Idle)
	, m_eAnimState(EPS_Idle)
	, animDict(ValueMap())
	, m_pcPhysLayer(CIntermediateLayer::GetInstance())
	, _platVel(b2Vec2(0,0))
	, m_fDeathWaitTimer( 0.f )
{
	m_pcAudioManager =  new cocos2d::experimental::AudioEngine();
	m_pcAudioManager->preload( "Audio/PH_Danielle/PH_Danielle_Jump.mp3" );
	m_pcAudioManager->preload( "Audio/PH_Danielle/PH_Danielle_Land.mp3" );
	m_pcAudioManager->preload( "Audio/PH_Danielle/PH_Danielle_Damaged.mp3" );
	m_pcAudioManager->preload( "Audio/PH_Danielle/PH_Danielle_Walking.mp3" );
	m_pcAudioManager->preload("Audio/PH_DanielleWeapons/PH_Danielle_Weapon_Fire.mp3");
	m_pcAudioManager->preload("Audio/PH_DanielleWeapons/PH_Danielle_Weapon_Hit.mp3");
	m_pcAudioManager->preload("Audio/PH_DanielleWeapons/PH_DanielleWeapon_fizzle.mp3");
	if(CIntermediateLayer::GetInstance(  )->GetLevelNumber() < 9)
	{
		m_pcShadow = new CShadow(this);
	}
	m_pcPhysLayer->SetPlayer(this);
}
void CPlayer::AddAnim(std::string anim)
{
	cocos2d::Animation* left = GCCocosHelpers::CreateAnimation(animDict, anim + "Left");
	left->retain();
	left->setDelayPerUnit(1.0f / 30.0f);
	cocos2d::Animation* right = GCCocosHelpers::CreateAnimation(animDict, anim + "Right");
	right->retain();
	m_vLeftAnims.push_back(left);
	m_vRightAnims.push_back(right);
}
void CPlayer::SetAnim(EPlayerStates _state)
{
	if (m_eAnimState != _state)
	{
	if(_state == EPS_Fired)
		{
			return;
		}
		if(m_bInitialized && _state == EPS_Firing)
		{
			if (true)
			{
				GetSprite()->stopAllActions();
				// RunAction(cocos2d::Sequence::create(GCCocosHelpers::CreateAnimationActionOnce(m_vLeftAnims[_state]), CC_CALLBACK_0(CPlayer::SetIdle, this)));
				RunAction(cocos2d::Sequence::create(GCCocosHelpers::CreateAnimationActionOnce(m_vLeftAnims[EPS_Fired]), cocos2d::CallFunc::create(std::bind(&CPlayer::SetIdle, this)), NULL));
			}
			else
			{
				GetSprite()->stopAllActions();
				// RunAction(cocos2d::Sequence::create(GCCocosHelpers::CreateAnimationActionOnce(m_vRightAnims[_state]), CC_CALLBACK_0(CPlayer::SetIdle, this)));
				RunAction(cocos2d::Sequence::create(GCCocosHelpers::CreateAnimationActionOnce(m_vRightAnims[EPS_Fired]), cocos2d::CallFunc::create(std::bind(&CPlayer::SetIdle, this)), NULL));
			}
		}
		else if (m_bInitialized)
		{
			if(true)
			{
				GetSprite(  )->stopAllActions();
				RunAction(GCCocosHelpers::CreateAnimationActionLoop(m_vLeftAnims[_state]));
				// RunAnimationLoop( GetFactoryCreationParams(  )->strPlistFile, m_vLeftAnims[_state] );
				//OutputDebugStringA("Left\n");
			}
			else
			{
				GetSprite()->stopAllActions();
				RunAction(GCCocosHelpers::CreateAnimationActionLoop(m_vRightAnims[_state]));
				//OutputDebugStringA("Right\n");
			}
			//m_eAnimState = _state;
		}
		m_eAnimState = _state;
	}
	SetFlippedX(!m_bLeft);
}
void CPlayer::SetWeapon(bool bSetWeapon)
{
	m_bWeaponObtained = bSetWeapon;
}
bool CPlayer::SetHacking(bool _hacking)
{
	if(_hacking && m_ePlayerState != EPS_Dead)
	{
		m_pcAudioManager->stopAll();
		m_ePlayerState = EPS_Hacking;
	}
	else if(m_ePlayerState == EPS_Hacking && !_hacking)
	{
		m_ePlayerState = EPS_Idle;
	}
	m_bDirtyLabel = true;
	return m_ePlayerState != EPS_Dead;
}

void CPlayer::StartTutorial( )
{
	m_ePlayerState = EPS_InTutorial;
	m_bDirtyLabel = true;	
}

void CPlayer::EndTutorial( )
{
	m_ePlayerState = EPS_Idle;
	m_bDirtyLabel = true;
}

void CPlayer::SetIdle( )
{
	m_ePlayerState = EPS_Idle;
	m_bDirtyLabel = true;
}

void CPlayer::RemoveShadow( )
{
	m_pcShadow = nullptr;
}

// kill
void CPlayer::vKillPlayer()
{
	m_pcAudioManager->play2d("Audio/PH_Danielle/PH_Danielle_Damaged.mp3", true, 0.5f);
	if (m_ePlayerState != EPS_Dead)
	{
	}
	m_bDirtyLabel = m_ePlayerState != EPS_Dead;
	m_ePlayerState = EPS_Dead;
	
}
// update input for the player
void CPlayer::updateInput()
{
	m_fHorizontalMotive = 0.0f;
	m_bJumpIntent = false;
	m_bInteractIntent = false;
	m_bFire = false;
	//m_fHorizontalMotive = 0.0f;
	const CGCKeyboardManager* pKeyManager = AppDelegate::GetKeyboardManager();
	// get the controller
	TGCController<EInputs> cCont = TGetActionMappedController(CGCControllerManager::eControllerOne, (*m_pcControllerActionToKeyMap));
	// check if it is active
	bool activeController = cCont.IsActive();
	if ( activeController )
	{
		// set our intent to interact based on if the player pressed interact
		m_bInteractIntent = cCont.ButtonHasJustBeenPressed( EPA_Interact );
	}
	m_bInteractIntent = pKeyManager->ActionHasJustBeenPressed(CGCGameLayerPlatformer::EPA_Down) || m_bInteractIntent;
	// if it is
	if(m_bCanMove)
	{
		if (activeController)
		{
			// set our intent based on if the player pressed jump
			m_bJumpIntent = cCont.ButtonHasJustBeenPressed(EPA_Jump);
			// If the Weapon Upgrade has been obtained or set in level then the player can shoot.
			if ( m_bWeaponObtained )
			{
				m_bFire = cCont.ButtonHasJustBeenPressed( EPA_ButtonFire );
			}
			cocos2d::Vec2 v2LeftStickInput = cCont.GetCurrentStickValueDeadzoned(EPA_AxisMove_X, EPA_AxisMove_Y, 0.01f);
			if(fabs(v2LeftStickInput.x) < 0.1)
			{
				v2LeftStickInput.x = 0;
			}
			// and do the same for the left right movement
			m_fHorizontalMotive = v2LeftStickInput.x;
			if(fabs(v2LeftStickInput.x) >= fabs(m_fHorizontalMotive))
			{
				m_fHorizontalMotive = v2LeftStickInput.x;
			}
			

			//////////////////////////////
			//Shadow
			//
			
			bool bShadowThrowPressed = cCont.ButtonHasJustBeenPressed(EPA_ShadowThrow);

			if(m_pcShadow)
			{
				if ( bShadowThrowPressed && m_pcShadow->CanReturn())
					//Cancel Hacking
				{
					m_pcShadow->ReturnToPlayer();
				}
				else if(m_ePlayerState == EPS_Idle || m_ePlayerState == EPS_Walking || m_ePlayerState == EPS_Commanding)
				{
					
					if(bShadowThrowPressed)
						//Begin Commanding
					{
						
						//Change the player state to commanding shadow if shadow is able to be commanded
						if (m_pcShadow->PrepareForCommand())
						{
							m_ePlayerState = EPS_Commanding;
							m_bDirtyLabel = true;
						}
						
					}
					else if(cCont.ButtonHasJustBeenReleased(EPA_ShadowThrow))
					{
						if (m_ePlayerState == EPS_Commanding)
							//Send Shadow in the input direction
						{
							m_pcShadow->Command();
							
							m_ePlayerState = EPS_Idle;
							m_bDirtyLabel = true;
							
						}
					}
					else if(m_ePlayerState == EPS_Commanding )
						//Cancel the command
					{
						if(cCont.ButtonHasJustBeenPressed(EPA_ButtonFire))
						{
							m_ePlayerState = EPS_Idle;
							m_bDirtyLabel = true;
							m_pcShadow->ReturnToPlayer();
						}
						else
						{
							int inputVectorX = static_cast<int>(2 * v2LeftStickInput.x);//, static_cast<int>(2 * v2LeftStickInput.y));
							if((m_bLeft && inputVectorX > 0) ||(!m_bLeft && inputVectorX < 0))
							{
								inputVectorX = 0;
							}
							Vector2i shadowVector = m_pcShadow->CommandPosition(inputVectorX, static_cast<int>(2 * v2LeftStickInput.y));

							if(!(shadowVector == m_v2LastShadowVector))
							{
								if (shadowVector.y > 0)
								{
									if (shadowVector.x != 0)
									{
										RunAnimation(GetFactoryCreationParams()->strPlistFile.c_str(), "CommandShadow45degrees");
									}
									else
									{
										RunAnimation(GetFactoryCreationParams()->strPlistFile.c_str(), "CommandShadowUp");
									}
								}
								else
								{
									RunAnimation(GetFactoryCreationParams()->strPlistFile.c_str(), "CommandShadowLeft");
								}
							}

							m_v2LastShadowVector = shadowVector;
						}
					}
				}
			}
		}
		if (pow(m_fHorizontalMotive, 2.0f) < pow(0.1f, 2.0f))
		{
			if (pKeyManager->ActionIsPressed(CGCGameLayerPlatformer::EPA_Left))
			{
				m_fHorizontalMotive = -1.0f;
			}
			if (pKeyManager->ActionIsPressed(CGCGameLayerPlatformer::EPA_Right))
			{
				m_fHorizontalMotive = 1.0f;
			}
			if (pKeyManager->ActionHasJustBeenPressed(CGCGameLayerPlatformer::EPA_Up))
			{
				m_bJumpIntent = true;
			}
			
		}
	}
	if ( m_bWeaponObtained )
	{
		m_bFire = pKeyManager->ActionHasJustBeenPressed( (CGCGameLayerPlatformer::EPA_Fire) ) || m_bFire;
	}
}
// animate!
void CPlayer::updateAnimation()
{
	SetAnim(m_ePlayerState);
	if((m_ePlayerState == EPS_Walking && fabs(GetBody()->GetLinearVelocity().x) < 0.2f))
	{
		SetAnim(EPS_Idle);
	}
	
	
	
}
// state transition
void CPlayer::DoStateTransisition()
{
	EPlayerStates oldState = m_ePlayerState;
	switch(m_ePlayerState)
	{
	case EPS_Idle:
		// if we are outside our deadzone
		if((pow(GetPhysicsBody()->GetLinearVelocity().x, 2.0f) > pow(0.2f, 2.0f) || pow(m_fHorizontalMotive, 2.0f) > pow(0.2f, 2.0f)) && m_bOnFloor)
		{
			m_pcAudioManager->stopAll();
			m_pcAudioManager->play2d( "Audio/PH_Danielle/PH_Danielle_Walking.mp3", true, 0.5f );
			// move into the "walking" state
			m_ePlayerState = EPS_Walking;
		}
		if(!m_bOnFloor && GetBody()->GetLinearVelocity().y <= 0.0f)
		{
			// move into the falling state
			m_ePlayerState = EPS_Falling;
		}
		else if(!m_bOnFloor || GetBody()->GetLinearVelocity().y > 0.0f)
		{
			m_pcAudioManager->stopAll();
			m_pcAudioManager->play2d( "Audio/PH_Danielle/PH_Danielle_Jump.mp3", false, 1.0f );
			// move into the jumping state
			m_ePlayerState = EPS_Jumping;
		}
		if (m_bFire)
		{
			m_ePlayerState = EPS_Firing;
			m_pcAudioManager->play2d("Audio/PH_DanielleWeapons/PH_Danielle_Weapon_Fire.mp3", false);
		}
		break;
	case EPS_Firing:
		m_ePlayerState = EPS_Fired;
		break;
	case EPS_Walking:
		
		if (!m_bOnFloor && GetBody()->GetLinearVelocity().y <= 0.0f)
		{
			// move into the falling state
			m_ePlayerState = EPS_Falling;
		}
		else if (!m_bOnFloor || GetBody()->GetLinearVelocity().y > 0.0f)
		{
			m_pcAudioManager->stopAll();
			m_pcAudioManager->play2d( "Audio/PH_Danielle/PH_Danielle_Jump.mp3", false, 1.0f );
			// move into the jumping state
			m_ePlayerState = EPS_Jumping;
		}
		else if(m_bOnFloor && (pow(GetPhysicsBody()->GetLinearVelocity().x, 2.0f) < pow(0.2f, 2.0f) && pow(m_fHorizontalMotive, 2.0f) < pow(0.2f, 2.0f)))
		{
			m_pcAudioManager->stopAll();
			m_ePlayerState = EPS_Idle;
		}
		break;
	case EPS_Jumping:
	case EPS_DoubleJumping:
		if(GetBody()->GetLinearVelocity().y < 0.0f && m_ePlayerState == EPS_Jumping)
		{
			// move into the jump-falling state, which is near identical to falling
			// but, will likely use a different animation
			m_ePlayerState = EPS_JumpFalling;
		}
		if (GetBody()->GetLinearVelocity().y < 0.0f && m_ePlayerState == EPS_DoubleJumping)
		{
			// move into the jump-falling state, which is near identical to falling
			// but, will likely use a different animation
			m_ePlayerState = EPS_Falling;
		}
		if (m_bOnFloor && !(GetBody()->GetLinearVelocity().y > 0.0f))
		{
			// move into the idle state
			m_ePlayerState = EPS_Idle;
			if(m_ePlayerState == EPS_DoubleJumping && m_pcShadow)
			{
				m_pcShadow->ReturnToPlayer();
			}
			
		}
		if (m_bDoubleJump && !(m_ePlayerState == EPS_DoubleJumpFloat || m_ePlayerState == EPS_DoubleJumping) && m_bJumpIntent && m_pcShadow && m_pcShadow->FootStool(m_fDoubleJumpHangTime))
		{
			m_ePlayerState = EPS_DoubleJumpFloat;
			m_fDoubleJumpHangTimer = m_fDoubleJumpHangTime;
		}
		break;
	case EPS_JumpFalling:
		if (m_bOnFloor)
		{
			m_pcAudioManager->stopAll();
			m_pcAudioManager->play2d( "Audio/PH_Danielle/PH_Danielle_Land.mp3", false, 0.5f );
			// move into the idle state
			m_ePlayerState = EPS_Idle;

		}
		if ((m_bDoubleJump && m_bJumpIntent && m_pcShadow && m_pcShadow->FootStool(m_fDoubleJumpHangTime) && !(m_ePlayerState == EPS_DoubleJumpFloat || m_ePlayerState == EPS_DoubleJumping)))
		{
			m_ePlayerState = EPS_DoubleJumpFloat;
			m_fDoubleJumpHangTimer = m_fDoubleJumpHangTime;
		}
		break;
	case EPS_Falling:
		if(m_bOnFloor)
		{
			m_pcAudioManager->stopAll();
			m_pcAudioManager->play2d( "Audio/PH_Danielle/PH_Danielle_Land.mp3", false, 0.5f );
			// move into the idle state
			m_ePlayerState = EPS_Idle;
		}
		break;
	default:
		break;
	}
	m_bDirtyLabel = m_ePlayerState != oldState || m_bDirtyLabel;
	if(m_bDirtyLabel)
	{
		m_StateLabel->setString(EPlayerStateNames[m_ePlayerState]);
		m_bDirtyLabel = false;
	}
	
}
// double jump
void CPlayer::SetDoubleJump(bool _allowed)
{
	m_bDoubleJump = _allowed;
}
// update velocity based on the data set by input
void CPlayer::updateVelocity(f32 fTimeStep)
{
	float direction = 1.0f;
	float fallingSpeed = 0;
	float horizontalSpeed = 0;
	switch(m_ePlayerState)
	{
	case EPS_Idle:
		horizontalSpeed = 0;
		
		if(m_bJumpIntent)
		{
			fallingSpeed = sqrt(m_fJumpHeight * 2.0f * -m_fGravity);
		}
		if (m_fFallDistance < (m_fLastFloorHeight - GetBody()->GetPosition().y))
		{
			vKillPlayer();
		}
		m_fLastFloorHeight = GetBody()->GetPosition().y;
		break;
	case EPS_Walking:
		horizontalSpeed = m_fWalkSpeed * m_fHorizontalMotive;
		
		if (m_bJumpIntent)
		{
			fallingSpeed = sqrt(m_fJumpHeight * 2.0f * -m_fGravity);
		}
		if(GetPhysicsBody()->GetLinearVelocity().x > 0.2f)
		{
			m_bLeft = false;
		}
		else if(GetPhysicsBody()->GetLinearVelocity().x < -0.2f)
		{
			m_bLeft = true;
		}
		
		m_fLastFloorHeight = GetBody()->GetPosition().y;
		break;
	case EPS_Jumping:
		horizontalSpeed = m_fWalkSpeed * m_fHorizontalMotive;
		fallingSpeed = GetPhysicsBody()->GetLinearVelocity().y + (m_fGravity * fTimeStep);
		if (GetPhysicsBody()->GetLinearVelocity().x > 0.2f)
		{
			m_bLeft = false;
		}
		else if (GetPhysicsBody()->GetLinearVelocity().x < -0.2f)
		{
			m_bLeft = true;
		}
		
		break;
	case EPS_DoubleJumpFloat:
		if(m_fDoubleJumpHangTimer > 0.0f)
		{
			m_fDoubleJumpHangTimer -= fTimeStep;
			//m_pcShadow->FootStool(m_fDoubleJumpHangTime);
		}
		else
		{
			m_ePlayerState = EPS_DoubleJumping;
			m_bDirtyLabel = true;
			fallingSpeed = sqrt(m_fDoubleJumpHeight * 2.0f * -m_fGravity);
			m_pcShadow->ReturnToPlayer();
		}
		break;
	case EPS_DoubleJumping:
		horizontalSpeed = m_fWalkSpeed * m_fHorizontalMotive;
		fallingSpeed = GetPhysicsBody()->GetLinearVelocity().y + (m_fGravity * fTimeStep);
		if (GetPhysicsBody()->GetLinearVelocity().x > 0.2f)
		{
			m_bLeft = false;
		}
		else if (GetPhysicsBody()->GetLinearVelocity().x < -0.2f)
		{
			m_bLeft = true;
		}
		break;
	case EPS_JumpFalling:
		horizontalSpeed = m_fWalkSpeed * m_fHorizontalMotive;
		fallingSpeed = GetPhysicsBody()->GetLinearVelocity().y + (m_fGravity * fTimeStep);
		if (GetPhysicsBody()->GetLinearVelocity().x > 0.2f)
		{
			m_bLeft = false;
		}
		else if (GetPhysicsBody()->GetLinearVelocity().x < -0.2f)
		{
			m_bLeft = true;
		}
		
		break;
	case EPS_Falling:
		horizontalSpeed = m_fWalkSpeed * m_fHorizontalMotive;
		fallingSpeed = GetPhysicsBody()->GetLinearVelocity().y + (m_fGravity * fTimeStep);
		

		break;
	case EPS_Firing:

		horizontalSpeed = m_fWalkSpeed * m_fHorizontalMotive;
		fallingSpeed = GetPhysicsBody()->GetLinearVelocity().y + (m_fGravity * fTimeStep);
		
		if(m_bLeft)
		{
			direction= - 1.0f;
		}
		static_cast<CGCObjGroupProjectilePlayer*>(CGCObjectManager::FindObjectGroupByID(GetGCTypeIDOf(CGCObjGroupProjectilePlayer))
		)->SpawnProjectile(GetSprite()->getPosition(),
			cocos2d::Vec2(direction * 32,
				 0), 48.0f, CProjectile::EProjectileType::Ball, this);
		break;
	case EPS_Commanding:
		break;
	case EPS_Hacking:
		fallingSpeed = GetPhysicsBody()->GetLinearVelocity().y + (m_fGravity * fTimeStep);
		break;
	case EPS_Dead:
		m_fDeathWaitTimer += fTimeStep;
		if (m_fDeathWaitTimer > k_fDeathWaitTime)
		{
			CIntermediateLayer::GetInstance()->VResetLevel();
		}
		break;
	default:
		horizontalSpeed = 0;
		fallingSpeed = 0;
		break;
	}
	
	GetPhysicsBody()->SetLinearVelocity(b2Vec2(horizontalSpeed, fallingSpeed));
	GetPhysicsBody()->SetTransform(GetPhysicsBody()->GetPosition() + b2Vec2(_platVel.x * fTimeStep, _platVel.y * fTimeStep), 0.0f);
	DoStateTransisition();
	_platVel.SetZero();
	m_bOnFloor = false;
}
// get the interact intent
bool CPlayer::GetInteractIntent()
{
	return m_bInteractIntent;
}
f32 CPlayer::GetStunTime()
{
	return m_fStunTime;
}
// destructor
CPlayer::~CPlayer( )
{
	m_pcAudioManager->end();
}
// do this when we get a sprite
void CPlayer::VOnResourceAcquire()
{
	CGCObjSpritePhysics::VOnResourceAcquire();
	m_pcControllerActionToKeyMap = TCreateActionToKeyMap(s_aePlayerActions, s_aeKeys);
	m_StateLabel = Label::createWithTTF("", "fonts/arial.ttf", 24);
	m_StateLabel->setString("Idle");
	GetSprite()->addChild(m_StateLabel);
	GetSprite()->setLocalZOrder(2);
	m_StateLabel->retain();
	m_StateLabel->setPosition(Vec2(0, 40));
	m_StateLabel->setOpacity(0);
	
}
// called before a collision is resolved
// here, we check the foot sensor to see if we get to jump or not
void CPlayer::vPreCollision( b2Contact* p_cContact, bool isA )
{
	
	if(p_cContact->IsTouching() )
	{
		b2Fixture* ourCollider = nullptr;
		b2Fixture* otherCollider = nullptr;
		CPhysicsObject* otherBody = nullptr;
		if (isA)
		{
			ourCollider = p_cContact->GetFixtureA();
			otherCollider = p_cContact->GetFixtureB();
			otherBody = (CPhysicsObject*)otherCollider->GetBody()->GetUserData();
		}
		else
		{
			ourCollider = p_cContact->GetFixtureB();
			otherCollider = p_cContact->GetFixtureA();
			otherBody = (CPhysicsObject*)otherCollider->GetBody()->GetUserData();
		}
		if ((otherCollider->GetFilterData().categoryBits & GetBit(ECollisionBit::ESemiSolid)))
		{
			p_cContact->SetEnabled(false);
		}
		if (GB2ShapeCache::getFixtureIdText(ourCollider)->compare("FootSensor") == 0 &&  (otherCollider->GetFilterData().categoryBits & GetBit(ECollisionBit::EPlatform)))
		{
			m_bOnFloor = true;
			// should be a sensor, but I struggled to make them behave so this is
			// a normal collider that I'll disable resolution for right here
			
			//platform velocity
			if(otherBody && otherBody->GetVelocity().lengthSquared() > 0.1f)
			{
				_platVel = otherBody->GetBody()->GetLinearVelocity();
			}
			
			p_cContact->SetEnabled(false);
		}
		if ((otherCollider->GetFilterData().categoryBits & GetBit(ECollisionBit::ELethalObject)))
		{
			vKillPlayer();
		}
	}
	
}
// update function, called by GCframework
void CPlayer::VOnUpdate( f32 fTimeStep )
{
	// if we have not run initialisation code yet
	if(!m_bInitialized)
	{
		// prevent rotation of our body
		GetPhysicsBody()->SetFixedRotation(true);
		// prevent sleeping
		GetPhysicsBody()->SetSleepingAllowed(false);
		// ignore scene - wide gravity
		GetPhysicsBody()->SetGravityScale(0.0f);
		// value map for animation
		animDict = GCCocosHelpers::CreateDictionaryFromPlist("TexturePacker/Sprites/Player/Player.plist");
		AddAnim("Idle");
		AddAnim("Run");
		AddAnim("Jump");
		AddAnim("Fall");
		AddAnim("Idle");
		AddAnim("Jump");
		AddAnim("Fall");
		AddAnim("Firing");
		AddAnim("Fired");
		AddAnim("Death");
		AddAnim("CommandShadow");
		
		m_vLeftAnims.push_back(GCCocosHelpers::CreateAnimation(animDict, "Hacking"));
		m_vLeftAnims[EPS_Hacking]->retain();
		m_vRightAnims.push_back(GCCocosHelpers::CreateAnimation(animDict, "Hacking"));
		m_vRightAnims[EPS_Hacking]->retain();
		AddAnim("Idle");
		// this is all we need to do for initilization
		m_bInitialized = true;
	}
	// update input, this is done with no reference to time
	updateInput();
	// update velocity, this needs to know how long a frame takes
	updateVelocity(fTimeStep);


}
void CPlayer::VOnLateUpdate(float fTimeStep)
{
	// update animation
	updateAnimation();
}
