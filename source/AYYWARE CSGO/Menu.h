/*
Syn's AyyWare Framework 2015
*/

#pragma once

#include "GUI.h"
#include "Controls.h"

class CRageBotTab : public CTab
{
public:
	void Setup();

	CLabel ActiveLabel;
	CCheckBox Active;

	CGroupBox AimbotGroup;
	CCheckBox AimbotEnable;
	CCheckBox AimbotAutoFire;
	CSlider  AimbotFov;
	CCheckBox AimbotSilentAim;
	CCheckBox PVSFix;
	CCheckBox AimbotResolver;
	CCheckBox AimbotResolverBrute;
	CCheckBox AdvancedResolver;
	CCheckBox AutoRevolver;
	CSlider AddFakeYaw;
	CSlider CustomPitch;
	CCheckBox LBY;
	CComboBox FakeYaw;
	CCheckBox LowerbodyFix;
	CCheckBox PreferBodyAim;
	CCheckBox AWPAtBody;
	CSlider BaimIfUnderXHealth;
	CSlider XShots;
	CCheckBox EnginePrediction;
	CCheckBox AimbotAimStep;
	CCheckBox AimbotKeyPress;
	CKeyBind  AimbotKeyBind;
	CKeyBind  AimbotStopKey;
	CKeyBind OtherSlowMotion;
	CKeyBind OtherFakeWalk;

	CGroupBox TargetGroup;
	CComboBox TargetSelection;
	CCheckBox TargetFriendlyFire;
	CComboBox TargetHitbox;
	CComboBox TargetHitscan;
	CCheckBox TargetMultipoint;
	CSlider   TargetPointscale;

	CGroupBox AccuracyGroup;
	CCheckBox AccuracyRecoil;
	CCheckBox AccuracyAutoWall;
	CSlider AccuracyMinimumDamage;
	CCheckBox AccuracyAutoStop;
	CCheckBox AccuracyAutoCrouch;
	CCheckBox AccuracyAutoScope;
	CSlider   AccuracyHitchance;
	CCheckBox ResolverDebug;
	CComboBox AccuracyResolver;
	CCheckBox AccuracyPositionAdjustment;

	CGroupBox AntiAimGroup;
	CCheckBox AntiAimEnable;
	CComboBox AntiAimPitch;
	CComboBox AntiAimYaw;
	CCheckBox FlipAA;
	CCheckBox AntiAimEdge;
	CSlider AntiAimOffset;
	CCheckBox AntiAimKnife;
	CCheckBox AccuracyResolver2;
	CKeyBind AccuracyAntifakewalk;
	CCheckBox AntiAimTarget;
};


class CVisualTab : public CTab
{
public:
	void Setup();

	// Master Switch
	CLabel ActiveLabel;
	CCheckBox Active;

	// Options Settings
	CGroupBox OptionsGroup;
	CComboBox OptionsBox;
	CCheckBox OptionsName;
	CCheckBox OptionsHealth;
	CCheckBox OptionsWeapon;
	CCheckBox OptionsArmor;
	CCheckBox OptionsInfo;
	CCheckBox OptionsMoney;
	CCheckBox HitmarkerSound;
	CCheckBox OptionsHelmet;
	CCheckBox OptionsBarrels;
	CCheckBox OptionsKit;
	CCheckBox OptionsDefuse;
	CComboBox OptionsGlow;
	CComboBox OptionsChams;
	CCheckBox OptionsSkeleton;
	CCheckBox OptionsAimSpot;
	CCheckBox OptionsCompRank;
	CCheckBox lbyidicador;

	// Filters Settings
	CGroupBox FiltersGroup;
	CCheckBox FiltersAll;
	CCheckBox FiltersPlayers;
	CCheckBox FiltersEnemiesOnly;
	CCheckBox FiltersWeapons;
	CCheckBox Filtersselfesp;
	CCheckBox FiltersChickens;
	CCheckBox FiltersC4;

	// Other Settings
	CCheckBox SniperCrosshair;
	CGroupBox OtherGroup;
	CComboBox OtherCrosshair;
	CCheckBox Clock;
	CCheckBox Watermark;
	CComboBox OtherRecoilCrosshair;
	CCheckBox OtherHitmarker;
	CCheckBox OtherRadar;
	CCheckBox NightSky;
	CCheckBox OtherNoVisualRecoil;
	CCheckBox Grenades;
	CCheckBox OtherNoSky; 
	CCheckBox OtherNoFlash; 
	CCheckBox DisablePostProcess;
	CSlider AmbientRed;
	CSlider AmbientGreen;
	CSlider AmbientBlue;
	CCheckBox OtherNoScope;
	CCheckBox OtherNoSmoke;
	CCheckBox OtherAsusWalls;
	CComboBox OtherNoHands;
	CComboBox OtherWepChams;
	CSlider OtherViewmodelFOV;
	CSlider OtherFOV;
	CCheckBox Lines;
};

class CMiscTab : public CTab
{
public:
	void Setup();

	CGroupBox Gloves;
	CCheckBox EnableGloves;
	CComboBox GloveSkin;
	CComboBox GloveModel;
	CButton GlovesApply;

	//settings
	CComboBox ConfigBox;
	CGroupBox ConfigGroup;
	CGroupBox Settings;
	CButton SettingsSave;
	CButton SettingsLoad;

	// Other Settings
	CGroupBox OtherGroup;
	CCheckBox TrashTalk;
	CCheckBox OtherAutoJump;
	CCheckBox OtherEdgeJump;
	CCheckBox OtherAutoStrafe;
	CCheckBox OtherSafeMode;
	CComboBox OtherChatSpam;
	CCheckBox OtherTeamChat;
	CSlider	  OtherChatDelay;
	CCheckBox EnableBuyBot;
	CComboBox BuyBot;
	CGroupBox BuyBotGroup;
	CComboBox BuyBotGrenades;
	CCheckBox BuyBotKevlar;
	CCheckBox BuyBotDefuser;
	CKeyBind  OtherAirStuck;
	CKeyBind  OtherLagSwitch;
	CCheckBox OtherSpectators;
	CCheckBox OtherThirdperson;
	CCheckBox OtherAutoAccept;
	//CCheckBox OtherAutoAccept;
	CCheckBox OtherWalkbot;
	CCheckBox OtherClantag;

	// Fake Lag Settings
	CGroupBox FakeLagGroup;
	CCheckBox FakeLagEnable;
	CSlider   FakeLagChoke;
	CSlider	  FakeLagSend;
	CCheckBox ChokeRandomize;
	CCheckBox SendRandomize;
	//CCheckBox FakeLagWhileShooting;
};

class ColorsTab : public CTab
{
public:
	void Setup();
	// Box ESP Colour
	CGroupBox ESPColorBox;
	CSlider   RBoxCT;
	CSlider   GBoxCT;
	CSlider   BBoxCT;
	CSlider   ABoxCT;
	CSlider   RBoxCTV;
	CSlider   GBoxCTV;
	CSlider   BBoxCTV;
	CSlider   ABoxCTV;
};


class CColorsTab : public CTab
{
public:
	void Setup();
	CGroupBox ControlGroup;
	CComboBox ControlColor;

	// CT IS VISUAL ESP
	CGroupBox CTVisESP;
	CSlider CTVisESPR; //red
	CSlider CTVisESPG; //green
	CSlider CTVisESPB; //blue

					   // T IS VISUAL ESP
	CGroupBox TVisESP;
	CSlider TVisESPR; //red
	CSlider TVisESPG; //green
	CSlider TVisESPB; //blue

					  // CT NOT VISIBLE ESP
	CGroupBox CTInvisESP;
	CSlider CTInvisESPR; //red
	CSlider CTInvisESPG; //green
	CSlider CTInvisESPB; //blue

						 // T NOT VISIBLE ESP
	CGroupBox TInvisESP;
	CSlider TInvisESPR; //red
	CSlider TInvisESPG; //green
	CSlider TInvisESPB; //blue

						// CT IS VISUAL CHAMS
	CGroupBox CTVisChams;
	CSlider CTVisChamsR; //red
	CSlider CTVisChamsG; //green
	CSlider CTVisChamsB; //blue

						 // T IS VISUAL CHAMS
	CGroupBox TVisChams;
	CSlider TVisChamsR; //red
	CSlider TVisChamsG; //green
	CSlider TVisChamsB; //blue

						// CT NOT VISIBLE CHAMS
	CGroupBox CTInvisChams;
	CSlider CTInvisChamsR; //red
	CSlider CTInvisChamsG; //green
	CSlider CTInvisChamsB; //blue

						   // T NOT VISIBLE CHAMS
	CGroupBox TInvisChams;
	CSlider TInvisChamsR; //red
	CSlider TInvisChamsG; //green
	CSlider TInvisChamsB; //blue
};



class AyyWareWindow : public CWindow
{
public:
	void Setup();

	CRageBotTab RageBotTab;
	CVisualTab VisualsTab;
	CMiscTab MiscTab;
	CColorsTab ColorsTab;


	CButton SaveButton;
	CButton LoadButton;
	CComboBox ConfigBox;
	CButton UnloadButton;
};

namespace Menu
{
	void SetupMenu();
	void DoUIFrame();

	extern AyyWareWindow Window;
};