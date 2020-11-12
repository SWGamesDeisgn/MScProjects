#pragma once
#include "GamerCamp/GCCocosInterface/IGCGameLayer.h"
#include "GamerCamp/GameSpecific/Player/GCObjGroupProjectilePlayer.h"
#include "GamerCamp/GameSpecific/Player/CProjectile.h"
#include "CHackable.h"
class CGCLevelLoader_Ogmo;
class CGCObjGroupItem;
class CGCObjGroupPlatform;

namespace CocosDenshion {
	class SimpleAudioEngine;
}

class CShadowTerminal;
class CTerminal;
class CPlayer;
class CInGameUILayer;
class CProgressionHandler;
class CDoorGroup;

class CIntermediateLayer : public IGCGameLayer, public b2ContactListener, public CHackable
{
public:
	CIntermediateLayer(GCTypeID idDerivedType, bool bIsCutscene = false);
	virtual ~CIntermediateLayer();

	void VOnCreate() override;

	virtual void PreSolve(b2Contact* pB2Contact, const b2Manifold* pOldManifold) override;
	virtual void PostSolve(b2Contact* pB2Contact, const b2ContactImpulse* pImpulse) override;
	virtual	void VOnDestroy() override;

	void CallNextLevel();
	virtual void VHack() override;
	virtual void VNextLevel() = 0;
	virtual void VResetLevel() = 0;

	CProgressionHandler* GetProgressionHandler() const;

	//Not a singleton
	static CIntermediateLayer* GetInstance();
	CGCObjGroupProjectilePlayer* GetProjectileGroup() const;
	void VOnResourceAcquire() override;
	void SetPlayer(CPlayer* pcPlayer);
	CPlayer* GetPlayer() const;
	void SetDanielleTerminal( CTerminal* _DanielleTerminal);
	void SetShadowTerminal( CShadowTerminal* _ShadowTerminal);
	CShadowTerminal* GetShadowTerminal() const {return m_pcShadowTerminal;}

	void ExitToMenu();

protected:
	CocosDenshion::SimpleAudioEngine* GetAudioEngine() const { return m_pcAudioEngine; }
	CGCLevelLoader_Ogmo* GetLevelLoader() const {return m_pcLevelLoader; }
	CGCObjSprite* GetBackground() const {return m_pcGCSprBackGround;}

	bool LoadLevel(const std::string& strOgmoLevelPath, const std::string& strBackgroundPath);

	void LoadBackground(const std::string& strBackgroundPath);

public:
	void VOnUpdate( f32 fTimeStep ) override;

	int GetLevelNumber( ) const;

	virtual void VPauseTimer(){};
	virtual void VRestartTimer(){};
private:
	CDoorGroup*								m_pcDoorGroup;

	CProgressionHandler*					m_pcProgressionHandler;

	static CIntermediateLayer*				m_pcCIntermediateLayerInstance;

	CInGameUILayer*							m_pcUILayer;
	CPlayer*								m_pcPlayer;
	CTerminal*								m_pcDanielleTerminal;
	CShadowTerminal*						m_pcShadowTerminal;
	CGCObjGroupProjectilePlayer*			m_projectileGroup;


	CocosDenshion::SimpleAudioEngine* m_pcAudioEngine;
	// object groups
	CGCObjGroupPlatform*         m_pcGCGroupPlatform;
	CGCObjGroupItem*             m_pcGCGroupItem;
	CGCObjGroupProjectilePlayer* m_pcGCGroupProjectilePlayer;

	// backgrounds
	CGCObjSprite* m_pcGCSprBackGround;
	// level loader
	CGCLevelLoader_Ogmo* m_pcLevelLoader;

	bool m_bLoadNextLevel;

	bool m_bQuitToMenu;

	bool m_bIsCutscene;

	int m_iLevelNumber;
};

