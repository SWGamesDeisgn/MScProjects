#include "CIntermediateLayer.h"
#include "PhysicsObject.h"
#include "proj.win32/Code/CDoorGroup.h"
#include "proj.win32/Code/CProgressionHandler.h"
#include "CInGameUILayer.h"
#include "SimpleAudioEngine.h"
#include "GamerCamp/GameSpecific/Items/GCObjGroupItem.h"
#include "GamerCamp/GameSpecific/Platforms/GCObjGroupPlatform.h"
#include "GamerCamp/GCCocosInterface/LevelLoader/GCLevelLoader_Ogmo.h"

#include "CPlayer.h"
#include "CUIMainMenu.h"
#include "Do Not Mark/String.h"

CIntermediateLayer* CIntermediateLayer::m_pcCIntermediateLayerInstance;

// constructor
CIntermediateLayer::CIntermediateLayer(GCTypeID idDerivedType, bool bIsCutscene)
	: IGCGameLayer(idDerivedType)
	, m_pcDoorGroup(nullptr)
	, m_pcProgressionHandler(nullptr)
	, m_pcUILayer( nullptr )
	, m_pcPlayer(nullptr)
	, m_pcDanielleTerminal(nullptr)
	, m_pcShadowTerminal(nullptr)
	, m_projectileGroup(nullptr)
	, m_pcAudioEngine(CocosDenshion::SimpleAudioEngine::getInstance( ) )
	, m_pcGCGroupPlatform( nullptr )
	, m_pcGCGroupItem( nullptr )
	, m_pcGCGroupProjectilePlayer( nullptr )
	, m_pcGCSprBackGround( nullptr )
	, m_pcLevelLoader( new CGCLevelLoader_Ogmo( ) )
	, m_bLoadNextLevel( false )
	, m_bQuitToMenu( false )
	, m_bIsCutscene( bIsCutscene )
{
	m_pcCIntermediateLayerInstance = this;
	//m_projectileGroup = new CGCObjGroupProjectilePlayer();
	
	
}
// destructor
CIntermediateLayer::~CIntermediateLayer( )
{
	// delete m_pcDoorGroup;
	// m_pcDoorGroup = nullptr;
	delete m_pcUILayer;
	m_pcUILayer = nullptr;

	delete m_pcLevelLoader;
	m_pcLevelLoader = nullptr;
}
void CIntermediateLayer::SetPlayer(CPlayer* pcPlayer)
{
	m_pcPlayer = pcPlayer;
}
void CIntermediateLayer::SetDanielleTerminal(CTerminal* _DanielleTerminal)
{
	m_pcDanielleTerminal = _DanielleTerminal;
}
void CIntermediateLayer::SetShadowTerminal(CShadowTerminal* _ShadowTerminal)
{
	m_pcShadowTerminal = _ShadowTerminal;
}

void CIntermediateLayer::ExitToMenu( )
{
	m_bQuitToMenu = true;
}

bool CIntermediateLayer::LoadLevel( const std::string& strOgmoLevelPath, const std::string& strBackgroundPath )
{
	cocos2d::Size  visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
	cocos2d::Point origin = cocos2d::Director::getInstance()->getVisibleOrigin();

	FW::String levelString(strOgmoLevelPath.c_str());

	levelString.replace("OgmoEditor/IRT3_Level_", "");
	levelString.replace("p", "");
	levelString.replace(".oel", "");


	m_iLevelNumber = levelString.toInt();

	//Store the level ogmo file path so shadow can check it for disabling on level 12

	// add the given background sprite
	{
		m_pcGCSprBackGround = new CGCObjSprite();
		m_pcGCSprBackGround->CreateSprite(strBackgroundPath.c_str());
		m_pcGCSprBackGround->SetResetPosition( cocos2d::Vec2(visibleSize.width / 2, visibleSize.height / 2));
		m_pcGCSprBackGround->SetParent(IGCGameLayer::ActiveInstance());
	}

	// read the oel file for the given level
	const bool loaded = m_pcLevelLoader->LoadLevelFile(cocos2d::FileUtils::getInstance()->fullPathForFilename(strOgmoLevelPath).c_str());
	m_pcLevelLoader->CreateObjects(CGCFactory_ObjSpritePhysics::GetFactory());

	return loaded;
}
void CIntermediateLayer::VHack()
{
	m_pcPlayer->SetDoubleJump(true);
}

void CIntermediateLayer::LoadBackground( const std::string& strBackgroundPath )
{
	cocos2d::Size  visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
	cocos2d::Point origin = cocos2d::Director::getInstance()->getVisibleOrigin();

	// add the given background sprite
	{
		m_pcGCSprBackGround = new CGCObjSprite();
		m_pcGCSprBackGround->CreateSprite(strBackgroundPath.c_str());
		m_pcGCSprBackGround->SetResetPosition(cocos2d::Vec2(visibleSize.width / 2, visibleSize.height / 2));
		m_pcGCSprBackGround->SetParent(IGCGameLayer::ActiveInstance());
	}

}

void CIntermediateLayer::VOnUpdate( f32 fTimeStep )
{

	if(m_bQuitToMenu)
	{
		CUIMainMenu* pMenu = new CUIMainMenu();
		cocos2d::Director::getInstance()->replaceScene(pMenu->CreateScene());

		return;
	}
	
	IGCGameLayer::VOnUpdate( fTimeStep );

	if(m_bLoadNextLevel)
	{
		VNextLevel(  );
	}
}

int CIntermediateLayer::GetLevelNumber( ) const
{
	return m_iLevelNumber;
}

CPlayer* CIntermediateLayer::GetPlayer() const
{
	return m_pcPlayer;
}
CGCObjGroupProjectilePlayer* CIntermediateLayer::GetProjectileGroup() const
{
	return m_projectileGroup;
}
void CIntermediateLayer::VOnCreate( )
{
	if(!m_bIsCutscene)
	{
		m_pcUILayer = new CInGameUILayer( );
	}

	IGCGameLayer::VOnCreate( );

	if (!m_bIsCutscene)
	{
		//////////////////////////////////////////////////////////////////
		//Object Groups
		// create and register the object group for the platform objects
		m_pcGCGroupPlatform = new CGCObjGroupPlatform( );
		CGCObjectManager::ObjectGroupRegister( m_pcGCGroupPlatform );

		// create and register the object group for the item objects
		m_pcGCGroupItem = new CGCObjGroupItem( );
		CGCObjectManager::ObjectGroupRegister( m_pcGCGroupItem );


		// create and register the object group for the player projectile objects
		m_pcGCGroupProjectilePlayer = new CGCObjGroupProjectilePlayer( );
		CGCObjectManager::ObjectGroupRegister( m_pcGCGroupProjectilePlayer );

		m_pcDoorGroup = new CDoorGroup( );
		CGCObjectManager::ObjectGroupRegister( m_pcDoorGroup );

		cocos2d::Size  visibleSize = cocos2d::Director::getInstance( )->getVisibleSize( );
		cocos2d::Point origin      = cocos2d::Director::getInstance( )->getVisibleOrigin( );
	
		//Create border
		{
			f32 PTM_RATIO = IGCGAMELAYER_B2D_PIXELS_PER_METER;

			b2Vec2        b2v2ScreenCentre_Pixels( ( origin.x + ( visibleSize.width * 0.5f ) ), ( origin.y + ( visibleSize.height * 0.5f ) ) );
			cocos2d::Vec2 v2ScreenCentre_Pixels( ( origin.x + ( visibleSize.width * 0.5f ) ), ( origin.y + ( visibleSize.height * 0.5f ) ) );

			b2Filter filter;
			filter.categoryBits = 1 << 5;


			// define the ground body
			b2BodyDef groundBodyDef;
			groundBodyDef.position = IGCGameLayer::B2dPixelsToWorld( b2v2ScreenCentre_Pixels );
			groundBodyDef.type     = b2_kinematicBody;

			// Call the body factory which allocates memory for the ground body
			// from a pool and creates the ground box shape (also from a pool).
			// The body is also added to the world.
			b2Body* groundBody = B2dGetWorld( )->CreateBody( &groundBodyDef );

			// Define the ground box shape.
			b2PolygonShape groundBox;

			// bottom
			groundBox.SetAsBox( ( ( visibleSize.width * 0.5f ) / PTM_RATIO ), 0.5f, b2Vec2( 0.0f, -( ( visibleSize.height * 0.5f ) / PTM_RATIO ) ), 0.0f );
			groundBody->CreateFixture( &groundBox, 0 )->SetFilterData( filter );


			// top
			groundBox.SetAsBox( ( ( visibleSize.width * 0.5f ) / PTM_RATIO ), 0.5f, b2Vec2( 0.0f, ( ( visibleSize.height * 0.5f ) / PTM_RATIO ) ), 0.0f );
			groundBody->CreateFixture( &groundBox, 0 )->SetFilterData( filter );

			// left
			groundBox.SetAsBox( 0.5f, ( ( visibleSize.height * 0.5f ) / PTM_RATIO ), b2Vec2( -( ( visibleSize.width * 0.5f ) / PTM_RATIO ), 0.0f ), 0.0f );
			groundBody->CreateFixture( &groundBox, 0 )->SetFilterData( filter );

			// right
			groundBox.SetAsBox( 0.5f, ( ( visibleSize.height * 0.5f ) / PTM_RATIO ), b2Vec2( ( ( visibleSize.width * 0.5f ) / PTM_RATIO ), 0.0f ), 0.0f );
			groundBody->CreateFixture( &groundBox, 0 )->SetFilterData( filter );
		}


		// load the physics shapes from the plist created with PhysicsEditor
		B2dLoadShapesFromPlist( "PhysicsEditor/GameShapes.plist" );

		m_pcProgressionHandler = new CProgressionHandler(  );
		B2dGetWorld( )->SetContactListener( this );
	}
}


// pre-collision handler
void CIntermediateLayer::PreSolve( b2Contact* pB2Contact, const b2Manifold* pOldManifold )
{
	
	
	 // get the fixtures
	 const b2Fixture* pFixtureA = pB2Contact->GetFixtureA( );
	 const b2Fixture* pFixtureB = pB2Contact->GetFixtureB( );
	 // and, their bodies
	 const b2Body* pBodyA = pFixtureA->GetBody( );
	 const b2Body* pBodyB = pFixtureB->GetBody( );
	
	 CGCObjSpritePhysics* pGcSprPhysA = (CGCObjSpritePhysics*)pBodyA->GetUserData( );
	 // empty pointers to physics objects - they _should_ be cast later
	 CPhysicsObject* pT3ObjA = nullptr;
	 CPhysicsObject* pT3ObjB = nullptr;
	 // if we don't have a sprite for the "A" side of the collision, do nothing
	 if ( pGcSprPhysA == nullptr )
	 {
	 }
	 	// otherwise, test if it is derived
	 else
	 {
	 	// if the object is a PhysicsObject
	 	if ( pGcSprPhysA->isExtended( ) )
	 	{
	 		// cast it
	 		pT3ObjA = static_cast< CPhysicsObject* >(pGcSprPhysA);
	 	}
	 }
	
	 CGCObjSpritePhysics* pGcSprPhysB = (CGCObjSpritePhysics*)pBodyB->GetUserData( );
	 // if we don't have a sprite for the "B" side of the collision, do nothing
	 if ( pGcSprPhysB == nullptr )
	 {
	 }
	 	// otherwise, test if it is derived
	 else
	 {
	 	// if the object is a PhysicsObject
	 	if ( pGcSprPhysB->isExtended( ) )
	 	{
	 		// cast it
	 		pT3ObjB = static_cast< CPhysicsObject* >(pGcSprPhysB);
	 	}
	 }
	 // if we have an "A side"
	 if ( pT3ObjA )
	 {
	 	// call the A side, telling it that it is
	 	// on the A side of the contact
	 	pT3ObjA->vPreCollision( pB2Contact, true );
	 }
	 if ( pT3ObjB )
	 {
	 	// call the B side, telling it that it is
	 	// on the B side of the contact
	 	pT3ObjB->vPreCollision( pB2Contact, false );
	 }
}

// post collision handler
void CIntermediateLayer::PostSolve( b2Contact* pB2Contact, const b2ContactImpulse* pImpulse )
{
		//b2Body* pBodyToDestroy = NULL;
		for (b2Contact* pB2Contact = IGCGameLayer::ActiveInstance()->B2dGetWorld()->GetContactList();
			NULL != pB2Contact;
			pB2Contact = pB2Contact->GetNext())
		{
			//Code removed until working
	
				 // get the fixtures
			const b2Fixture* pFixtureA = pB2Contact->GetFixtureA();
			const b2Fixture* pFixtureB = pB2Contact->GetFixtureB();
			// and, their bodies
			const b2Body* pBodyA = pFixtureA->GetBody();
			const b2Body* pBodyB = pFixtureB->GetBody();

			CGCObjSpritePhysics* pGcSprPhysA = (CGCObjSpritePhysics*)pBodyA->GetUserData();
			CGCObjSpritePhysics* pGcSprPhysB = (CGCObjSpritePhysics*)pBodyB->GetUserData();
			// empty pointers to physics objects - they _should_ be cast later
			CPhysicsObject* pT3ObjA = nullptr;
			CPhysicsObject* pT3ObjB = nullptr;
			// if we don't have a sprite for the "A" side of the collision, do nothing
			if (pGcSprPhysA == nullptr)
			{
			}
			// otherwise, test if it is derived
			else
			{
				// if the object is a PhysicsObject
				if (pGcSprPhysA->isExtended())
				{
					// cast it
					pT3ObjA = static_cast<CPhysicsObject*>(pGcSprPhysA);
				}
			}

			// if we don't have a sprite for the "B" side of the collision, do nothing
			if (pGcSprPhysB == nullptr)
			{
			}
			// otherwise, test if it is derived
			else
			{
				// if the object is a PhysicsObject
				if (pGcSprPhysB->isExtended())
				{
					// cast it
					pT3ObjB = static_cast<CPhysicsObject*>(pGcSprPhysB);
				}
			}
			// if we have an "A side"
			if (pT3ObjA)
			{
				// call the A side, telling it that it is
				// on the A side of the contact
				pT3ObjA->vPostCollision(pB2Contact, true);
			}
			if (pT3ObjB)
			{
				// call the B side, telling it that it is
				// on the B side of the contact
				pT3ObjB->vPostCollision(pB2Contact, false);
			}
	}

}

void CIntermediateLayer::VOnDestroy( )
{
	if(!m_bIsCutscene)
	{
		m_pcUILayer->removeFromParent();
		delete m_pcUILayer;
		m_pcUILayer = nullptr;


	///////////////////////////////////////////////////////////////////////////
	// clean up anything we allocated in opposite order to creation
	///////////////////////////////////////////////////////////////////////////	
	// clean up the level
	m_pcLevelLoader->DestroyObjects();
	}

	delete m_pcGCSprBackGround;
	m_pcGCSprBackGround = NULL;

	if (!m_bIsCutscene)
	{
		///////////////////////////////////////////////////////////////////////////
		// N.B. because object groups must register manually, 
		// we also unregister them manually
		///////////////////////////////////////////////////////////////////////////
		CGCObjectManager::ObjectGroupUnRegister(m_pcGCGroupPlatform);
		delete m_pcGCGroupPlatform;
		m_pcGCGroupPlatform = NULL;

		CGCObjectManager::ObjectGroupUnRegister(m_pcGCGroupProjectilePlayer);
		delete m_pcGCGroupProjectilePlayer;
		m_pcGCGroupProjectilePlayer = NULL;
		
		CGCObjectManager::ObjectGroupUnRegister(m_pcGCGroupItem);
		delete m_pcGCGroupItem;
		m_pcGCGroupItem = NULL;
	}
	
	IGCGameLayer::VOnDestroy();
}

void CIntermediateLayer::CallNextLevel( )
{
	m_bLoadNextLevel = true;
}

CProgressionHandler* CIntermediateLayer::GetProgressionHandler( ) const
{
	return m_pcProgressionHandler;
}

CIntermediateLayer* CIntermediateLayer::GetInstance( )
{
	return m_pcCIntermediateLayerInstance;
}

void CIntermediateLayer::VOnResourceAcquire( )
{
	IGCGameLayer::VOnResourceAcquire( );
	//m_projectileGroup->VOnGroupResourceAcquire();
	if (!m_bIsCutscene) 
	{
		addChild(m_pcUILayer);
		m_pcProgressionHandler->CreateLink("TerminalID:64", this);
	}
}
