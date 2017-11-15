/*
Syn's AyyWare Framework 2015
*/

#include "Menu.h"
#include "Controls.h"
#include "Hooks.h" // for the unload meme
#include "Interfaces.h"
#include "CRC32.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 630

AyyWareWindow Menu::Window;

void SaveCallbk()
{
	switch (Menu::Window.ConfigBox.GetIndex())
	{
	case 0:
		GUI.SaveWindowState(&Menu::Window, "haccLegit.cfg");
		break;
	case 1:
		GUI.SaveWindowState(&Menu::Window, "haccRage.cfg");
		break;
	case 2:
		GUI.SaveWindowState(&Menu::Window, "haccMMHvH.cfg");
		break;
	case 3:
		GUI.SaveWindowState(&Menu::Window, "haccScoutHvH.cfg");
		break;
	case 4:
		GUI.SaveWindowState(&Menu::Window, "haccAWPHvH.cfg");
		break;
	case 5:
		GUI.SaveWindowState(&Menu::Window, "haccNSHvH.cfg");
		break;
	}
}

// Load Config from CSGO directory
void LoadCallbk()
{
	switch (Menu::Window.ConfigBox.GetIndex())
	{
	case 0:
		GUI.LoadWindowState(&Menu::Window, "haccLegit.cfg");
		break;
	case 1:
		GUI.LoadWindowState(&Menu::Window, "haccRage.cfg");
		break;
	case 2:
		GUI.LoadWindowState(&Menu::Window, "haccMMHvH.cfg");
		break;
	case 3:
		GUI.LoadWindowState(&Menu::Window, "haccScoutHvH.cfg");
		break;
	case 4:
		GUI.LoadWindowState(&Menu::Window, "haccAWPHvH.cfg");
		break;
	case 5:
		GUI.LoadWindowState(&Menu::Window, "haccNSHvH.cfg");
		break;
	}
}

void UnLoadCallbk()
{
	DoUnload = true;
}

void KnifeApplyCallbk()
{
	Interfaces::Engine->ClientCmd_Unrestricted("record meme;stop");
}

void GlovesApplyCallbk()
{
	static ConVar* Meme = Interfaces::CVar->FindVar("cl_fullupdate");
	Meme->nFlags &= ~FCVAR_CHEAT;
	Interfaces::Engine->ClientCmd_Unrestricted("cl_fullupdate");
	bGlovesNeedUpdate = true;
}

void AyyWareWindow::Setup()
{
	SetPosition(350, 50);
	SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	SetTitle("hacc");

	RegisterTab(&RageBotTab);
	RegisterTab(&VisualsTab);
	RegisterTab(&MiscTab);
	//RegisterTab(&Playerlist);

	RECT Client = GetClientArea();
	Client.bottom -= 29;

	RageBotTab.Setup();
	VisualsTab.Setup();
	MiscTab.Setup();
	//Playerlist.Setup();

#pragma region Bottom Buttons
	ConfigBox.SetFileId("cfg_box");
	ConfigBox.AddItem("HvH");
	ConfigBox.AddItem("HvH 2");
	ConfigBox.SetSize(112, 350);
	ConfigBox.SetPosition(600, Client.bottom - 480);

	SaveButton.SetText("Save");
	SaveButton.SetCallback(SaveCallbk);
	SaveButton.SetSize(112, 350);
	SaveButton.SetPosition(600, Client.bottom - 450);

	LoadButton.SetText("Load");
	LoadButton.SetCallback(LoadCallbk);
	LoadButton.SetSize(112, 350);
	LoadButton.SetPosition(600, Client.bottom - 420);

	MiscTab.RegisterControl(&LoadButton);

	MiscTab.RegisterControl(&SaveButton);

	MiscTab.RegisterControl(&ConfigBox);
#pragma endregion Setting up the settings buttons
}

void CRageBotTab::Setup()
{
	SetTitle("a");

	ActiveLabel.SetPosition(20, 16);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(70, 18);
	RegisterControl(&Active);

#pragma region Aimbot

	AimbotGroup.SetPosition(16, 11);
	AimbotGroup.SetText("Aimbot");
	AimbotGroup.SetSize(376, 270);
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceLabledControl("Enable", this, &AimbotEnable);

	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotGroup.PlaceLabledControl("Auto Fire", this, &AimbotAutoFire);

	AimbotFov.SetFileId("aim_fov");
	AimbotFov.SetBoundaries(0.f, 180.f);
	AimbotFov.SetValue(39.f);
	AimbotGroup.PlaceLabledControl("FOV Range", this, &AimbotFov);

	AimbotSilentAim.SetFileId("aim_silent");
	AimbotGroup.PlaceLabledControl("Silent Aim", this, &AimbotSilentAim);

	AWPAtBody.SetFileId("aim_awpatbody");
	AimbotGroup.PlaceLabledControl("AWP at Body", this, &AWPAtBody);

	AutoRevolver.SetFileId("aim_autorevolver");
	AimbotGroup.PlaceLabledControl("Auto Revolver", this, &AutoRevolver);

	AimbotAimStep.SetFileId("aim_aimstep");
	AimbotGroup.PlaceLabledControl("Aim Step", this, &AimbotAimStep);

	AimbotStopKey.SetFileId("aim_stop");
	AimbotGroup.PlaceLabledControl("Stop Aim", this, &AimbotStopKey);

#pragma endregion Aimbot Controls Get Setup in here

#pragma region Target
	TargetGroup.SetPosition(16, 297);
	TargetGroup.SetText("Target");
	TargetGroup.SetSize(376, 285);
	RegisterControl(&TargetGroup);

	TargetSelection.SetFileId("tgt_selection");
	TargetSelection.AddItem("Closest To Crosshair");
	TargetSelection.AddItem("Distance");
	TargetSelection.AddItem("Lowest Health");
	TargetSelection.AddItem("Threat");
	TargetSelection.AddItem("Next Shot");
	TargetGroup.PlaceLabledControl("Selection", this, &TargetSelection);

	TargetFriendlyFire.SetFileId("tgt_friendlyfire");
	TargetGroup.PlaceLabledControl("Friendly Fire", this, &TargetFriendlyFire);

	TargetHitbox.SetFileId("tgt_hitbox");
	TargetHitbox.AddItem("Head");
	TargetHitbox.AddItem("Neck");
	TargetHitbox.AddItem("Chest");
	TargetHitbox.AddItem("Stomach");
	TargetHitbox.AddItem("Foots");
	TargetGroup.PlaceLabledControl("Hitbox", this, &TargetHitbox);

	TargetHitscan.SetFileId("tgt_hitscan");
	TargetHitscan.AddItem("Off");
	TargetHitscan.AddItem("Low");
	TargetHitscan.AddItem("Medium");
	TargetHitscan.AddItem("High");
	TargetGroup.PlaceLabledControl("Hitscan", this, &TargetHitscan);


	

	TargetMultipoint.SetFileId("tgt_multipoint");
	TargetGroup.PlaceLabledControl("Multipoint", this, &TargetMultipoint);

	TargetPointscale.SetFileId("tgt_pointscale");
	TargetPointscale.SetBoundaries(0.f, 10.f);
	TargetPointscale.SetValue(5.f);
	TargetGroup.PlaceLabledControl("Point Scale", this, &TargetPointscale);
#pragma endregion Targetting controls 

#pragma region Accuracy
	AccuracyGroup.SetPosition(408, 11);
	AccuracyGroup.SetText("Accuracy");
	AccuracyGroup.SetSize(360, 270);
	RegisterControl(&AccuracyGroup);

	AccuracyRecoil.SetFileId("acc_norecoil");
	AccuracyGroup.PlaceLabledControl("Remove Recoil", this, &AccuracyRecoil);

	AccuracyAutoWall.SetFileId("acc_awall");
	AccuracyGroup.PlaceLabledControl("Auto Wall", this, &AccuracyAutoWall);

	AccuracyMinimumDamage.SetFileId("acc_mindmg");
	AccuracyMinimumDamage.SetBoundaries(1.f, 99.f);
	AccuracyMinimumDamage.SetValue(1.f);
	AccuracyGroup.PlaceLabledControl("Autowall Damage", this, &AccuracyMinimumDamage);

	AccuracyAutoScope.SetFileId("acc_scope");
	AccuracyGroup.PlaceLabledControl("Auto Scope", this, &AccuracyAutoScope);

	AccuracyPositionAdjustment.SetFileId("acc_posadj");
	AccuracyGroup.PlaceLabledControl("Position Adjustment", this, &AccuracyPositionAdjustment);


	AimbotResolver.SetFileId("acc_aaa");
	AccuracyGroup.PlaceLabledControl("Resolver", this, &AimbotResolver);

	AimbotResolver.SetFileId("acc_aaab");
	AccuracyGroup.PlaceLabledControl("Brute Resolver(alt)", this, &AimbotResolverBrute);

	//ResolverDebug.SetFileId("acc_debugresolver");
	//AccuracyGroup.PlaceLabledControl("Resolver ebug", this, &ResolverDebug);

	AccuracyHitchance.SetFileId("acc_chance");
	AccuracyHitchance.SetBoundaries(0, 100);
	AccuracyHitchance.SetValue(0);
	AccuracyGroup.PlaceLabledControl("Hit Chance", this, &AccuracyHitchance);

	BaimIfUnderXHealth.SetFileId("acc_BaimIfUnderXHealth");
	BaimIfUnderXHealth.SetBoundaries(0, 100);
	BaimIfUnderXHealth.SetValue(0);
	AccuracyGroup.PlaceLabledControl("Body-Aim if HP is lower than X", this, &BaimIfUnderXHealth);

	PreferBodyAim.SetFileId("acc_preferbaim");
	AccuracyGroup.PlaceLabledControl("Prefer Body-Aim", this, &PreferBodyAim);

#pragma endregion  Accuracy controls get Setup in here

#pragma region AntiAim
	AntiAimGroup.SetPosition(408, 297);
	AntiAimGroup.SetText("Anti Aim");
	AntiAimGroup.SetSize(360, 285);
	RegisterControl(&AntiAimGroup);

	AntiAimEnable.SetFileId("aa_enable");
	AntiAimGroup.PlaceLabledControl("Enable", this, &AntiAimEnable);

	AntiAimPitch.SetFileId("aa_x");
	AntiAimPitch.AddItem("Off");
	AntiAimPitch.AddItem("Down");
	AntiAimGroup.PlaceLabledControl("Pitch", this, &AntiAimPitch);

	AntiAimYaw.SetFileId("aa_y");
	AntiAimYaw.AddItem("Off");
	AntiAimYaw.AddItem("Jitter");
	AntiAimYaw.AddItem("Break LBY");
	AntiAimYaw.AddItem("New LBY");
	AntiAimYaw.AddItem("Jitter 180");
	AntiAimYaw.AddItem("Testing");
	AntiAimGroup.PlaceLabledControl("Real Yaw", this, &AntiAimYaw);

	FakeYaw.SetFileId("fakeaa");
	FakeYaw.AddItem("Off");
	FakeYaw.AddItem("Jitter");
	FakeYaw.AddItem("Break LBY");
	FakeYaw.AddItem("New LBY");
	FakeYaw.AddItem("Jitter 180");
	FakeYaw.AddItem("Testing");
	AntiAimGroup.PlaceLabledControl("Fake Yaw", this, &FakeYaw);


	AntiAimOffset.SetFileId("aa_offset");
	AntiAimOffset.SetBoundaries(0, 360);
	AntiAimOffset.SetValue(0);
	AntiAimGroup.PlaceLabledControl("Add Real Yaw", this, &AntiAimOffset);

	AddFakeYaw.SetFileId("aa_offset");
	AddFakeYaw.SetBoundaries(0, 360);
	AddFakeYaw.SetValue(0);
	AntiAimGroup.PlaceLabledControl("Add Fake Yaw", this, &AddFakeYaw);



	AntiAimKnife.SetFileId("aa_knife");
	AntiAimGroup.PlaceLabledControl("Knife AA", this, &AntiAimKnife);
#pragma endregion  AntiAim controls get setup in here
}

void CVisualTab::Setup()
{
	SetTitle("c");

	ActiveLabel.SetPosition(16, 16);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(66, 16);
	RegisterControl(&Active);

#pragma region Options
	OptionsGroup.SetText("Options");
	OptionsGroup.SetPosition(16, 48);
	OptionsGroup.SetSize(193, 430);
	RegisterControl(&OptionsGroup);

	OptionsBox.SetFileId("opt_box");
	OptionsBox.AddItem("Off");
	OptionsBox.AddItem("Corners");
	OptionsBox.AddItem("Full");
	OptionsGroup.PlaceLabledControl("Box", this, &OptionsBox);

	OptionsName.SetFileId("opt_name");
	OptionsGroup.PlaceLabledControl("Name", this, &OptionsName);

	OptionsHealth.SetFileId("opt_hp");
	OptionsGroup.PlaceLabledControl("Health", this, &OptionsHealth);


	OptionsHealth.SetFileId("opt_ghost");
	OptionsGroup.PlaceLabledControl("Lines", this, &Lines);

	lbyidicador.SetFileId("opt_lbyidicador");
	OptionsGroup.PlaceLabledControl("LBY Indicador", this, &lbyidicador);

#pragma endregion Setting up the Options controls

#pragma region Filters
	FiltersGroup.SetText("Filters");
	FiltersGroup.SetPosition(225, 48);
	FiltersGroup.SetSize(193, 430);
	RegisterControl(&FiltersGroup);


	FiltersPlayers.SetFileId("ftr_players");
	FiltersGroup.PlaceLabledControl("Players", this, &FiltersPlayers);

	FiltersEnemiesOnly.SetFileId("ftr_enemyonly");
	FiltersGroup.PlaceLabledControl("Enemies Only", this, &FiltersEnemiesOnly);
#pragma endregion Setting up the Filters controls

#pragma region Other
	OtherGroup.SetText("Other");
	OtherGroup.SetPosition(434, 48);
	OtherGroup.SetSize(334, 430);
	RegisterControl(&OtherGroup);


	OtherNoHands.SetFileId("otr_hands");
	OtherNoHands.AddItem("Off");
	OtherNoHands.AddItem("None");
	OtherNoHands.AddItem("Gold");
	OtherNoHands.AddItem("Chrome");
	OtherNoHands.AddItem("Crystal");
	OtherGroup.PlaceLabledControl("Hands", this, &OtherNoHands);


	OtherWepChams.SetFileId("otr_weps");
	OtherWepChams.AddItem("Off");
	OtherWepChams.AddItem("Chrome");
	OtherWepChams.AddItem("Gold");
	OtherWepChams.AddItem("Crystal");
	OtherGroup.PlaceLabledControl("Weapons", this, &OtherWepChams);


	OptionsChams.SetFileId("otr_weps");
	OptionsChams.AddItem("Off");
	OptionsChams.AddItem("Crystal");
	OtherGroup.PlaceLabledControl("local chams", this, &OptionsChams);

	SniperCrosshair.SetFileId("otr_snipercrosshair");
	OtherGroup.PlaceLabledControl("Sniper Crosshair", this, &SniperCrosshair);


	OtherNoFlash.SetFileId("otr_noflash");
	OtherGroup.PlaceLabledControl("Remove Flash", this, &OtherNoFlash);

	OtherNoSmoke.SetFileId("otr_nosmoke");
	OtherGroup.PlaceLabledControl("Remove Smoke", this, &OtherNoSmoke);

	OtherNoScope.SetFileId("otr_noscope");
	OtherGroup.PlaceLabledControl("Remove Scope", this, &OtherNoScope);

	OtherNoVisualRecoil.SetFileId("otr_visrecoil");
	OtherGroup.PlaceLabledControl("No Visual Recoil", this, &OtherNoVisualRecoil);


	Watermark.SetFileId("otr_watermarker");
	Watermark.SetState(true);
	OtherGroup.PlaceLabledControl("Watermark", this, &Watermark);

	OtherRadar.SetFileId("otr_radar");
	OtherGroup.PlaceLabledControl("Radar", this, &OtherRadar);

	OtherViewmodelFOV.SetFileId("otr_viewfov");
	OtherViewmodelFOV.SetBoundaries(0.f, 180.f);
	OtherViewmodelFOV.SetValue(0.f);
	OtherGroup.PlaceLabledControl("Viewmodel FOV", this, &OtherViewmodelFOV);

	OtherFOV.SetFileId("otr_fov");
	OtherFOV.SetBoundaries(0.f, 180.f);
	OtherFOV.SetValue(90.f);
	OtherGroup.PlaceLabledControl("Field of View", this, &OtherFOV);


#pragma endregion Setting up the Other controls
}

void CMiscTab::Setup()
{
	SetTitle("d");

#pragma region Other
	OtherGroup.SetPosition(408, 16);
	OtherGroup.SetSize(360, 430);
	OtherGroup.SetText("Other");
	RegisterControl(&OtherGroup);


	OtherAutoJump.SetFileId("otr_autojump");
	OtherGroup.PlaceLabledControl("Auto Jump", this, &OtherAutoJump);

	OtherAutoStrafe.SetFileId("otr_strafe");
	OtherGroup.PlaceLabledControl("Auto Strafer", this, &OtherAutoStrafe);

	OtherSafeMode.SetFileId("otr_safemode");
	OtherSafeMode.SetState(true);
	OtherGroup.PlaceLabledControl("Anti Untrusted", this, &OtherSafeMode);


	OtherClantag.SetFileId("otr_spam");
	OtherGroup.PlaceLabledControl("Set clantag", this, &OtherClantag);


	OtherSpectators.SetFileId("otr_speclist");
	OtherGroup.PlaceLabledControl("Spectators List", this, &OtherSpectators);

	OtherThirdperson.SetFileId("aa_thirdpsr");
	OtherGroup.PlaceLabledControl("Thirdperson", this, &OtherThirdperson);


#pragma endregion other random options

#pragma region FakeLag
	FakeLagGroup.SetPosition(16, 10);
	FakeLagGroup.SetSize(370, 120);
	FakeLagGroup.SetText("Fakelag");
	RegisterControl(&FakeLagGroup);

	FakeLagEnable.SetFileId("fakelag_enable");
	FakeLagGroup.PlaceLabledControl("Fake Lag", this, &FakeLagEnable);

	FakeLagChoke.SetFileId("fakelag_choke");
	FakeLagChoke.SetBoundaries(0, 16);
	FakeLagChoke.SetValue(0);
	FakeLagGroup.PlaceLabledControl("Choke Factor", this, &FakeLagChoke);

	FakeLagSend.SetFileId("fakelag_send");
	FakeLagSend.SetBoundaries(0, 16);
	FakeLagSend.SetValue(0);
	FakeLagGroup.PlaceLabledControl("Send Factor", this, &FakeLagSend);

	ChokeRandomize.SetFileId("choke_random");
	FakeLagGroup.PlaceLabledControl("Randomize Choke", this, &ChokeRandomize);

	SendRandomize.SetFileId("send_random");
	FakeLagGroup.PlaceLabledControl("Randomize Send", this, &SendRandomize);

#pragma endregion fakelag shit
}



void Menu::SetupMenu()
{
	Window.Setup();

	GUI.RegisterWindow(&Window);
	GUI.BindWindow(VK_INSERT, &Window);
}

void Menu::DoUIFrame()
{
		if (Window.VisualsTab.FiltersAll.GetState())
	{
		Window.VisualsTab.FiltersC4.SetState(true);
		Window.VisualsTab.FiltersChickens.SetState(true);
		Window.VisualsTab.FiltersPlayers.SetState(true);
		Window.VisualsTab.FiltersWeapons.SetState(true);
		Window.VisualsTab.FiltersEnemiesOnly.SetState(true);
	}

	GUI.Update();
	GUI.Draw();

	
}


