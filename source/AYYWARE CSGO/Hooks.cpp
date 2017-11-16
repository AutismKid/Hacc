/*
Syn's AyyWare Framework 2015
*/

#include "Hooks.h"
#include "Hacks.h"
#include "Chams.h"
#include "Menu.h"
#include "hitmarker.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "MiscHacks.h"
#include "CRC32.h"
#include "Resolver.h"
#include <intrin.h>



//Resolver.h Include --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Vector LastAngleAA;
bool flipAA;
bool Resolver::didhitHS;
CUserCmd* Globals::UserCmd;
IClientEntity* Globals::Target;
int Globals::Shots;
bool Globals::change;
int Globals::choked_ticks;
int Globals::TargetID;
std::map<int, QAngle>Globals::storedshit;
int Globals::missedshots;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



// Funtion Typedefs
typedef void(__thiscall* DrawModelEx_)(void*, void*, void*, const ModelRenderInfo_t&, matrix3x4*);
typedef void(__thiscall* PaintTraverse_)(PVOID, unsigned int, bool, bool);
typedef bool(__thiscall* InPrediction_)(PVOID);
typedef void(__stdcall *FrameStageNotifyFn)(ClientFrameStage_t);
typedef bool(__thiscall *FireEventClientSideFn)(PVOID, IGameEvent*);
typedef void(__thiscall* RenderViewFn)(void*, CViewSetup&, CViewSetup&, int, int);

using OverrideViewFn = void(__fastcall*)(void*, void*, CViewSetup*);
typedef float(__stdcall *oGetViewModelFOV)();

#define MakePtr(cast, ptr, addValue) (cast)( (DWORD)(ptr) + (DWORD)(addValue))
// Function Pointers to the originals
PaintTraverse_ oPaintTraverse;
DrawModelEx_ oDrawModelExecute;
FrameStageNotifyFn oFrameStageNotify;
OverrideViewFn oOverrideView;
FireEventClientSideFn oFireEventClientSide;
RenderViewFn oRenderView;

// Hook function prototypes
void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
bool __stdcall Hooked_InPrediction();
bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event);
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld);
bool __stdcall CreateMoveClient_Hooked(/*void* self, int edx,*/ float frametime, CUserCmd* pCmd);
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage);
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup);
float __stdcall GGetViewModelFOV();
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw);

// VMT Managers
namespace Hooks
{
	// VMT Managers
	Utilities::Memory::VMTManager VMTPanel; // Hooking drawing functions
	Utilities::Memory::VMTManager VMTClient; // Maybe CreateMove
	Utilities::Memory::VMTManager VMTClientMode; // CreateMove for functionality
	Utilities::Memory::VMTManager VMTModelRender; // DrawModelEx for chams
	Utilities::Memory::VMTManager VMTPrediction; // InPrediction for no vis recoil
	Utilities::Memory::VMTManager VMTPlaySound; // Autoaccept 
	Utilities::Memory::VMTManager VMTRenderView;
	Utilities::Memory::VMTManager VMTEventManager;
};

// Undo our hooks
void Hooks::UndoHooks()
{
	VMTPanel.RestoreOriginal();
	VMTPrediction.RestoreOriginal();
	VMTModelRender.RestoreOriginal();
	VMTClientMode.RestoreOriginal();
}


// Initialise all our hooks
void Hooks::Initialise()
{
	// Panel hooks for drawing to the screen via surface functions
	VMTPanel.Initialise((DWORD*)Interfaces::Panels);
	oPaintTraverse = (PaintTraverse_)VMTPanel.HookMethod((DWORD)&PaintTraverse_Hooked, Offsets::VMT::Panel_PaintTraverse);
	//Utilities::Log("Paint Traverse Hooked");

	// No Visual Recoi	l
	VMTPrediction.Initialise((DWORD*)Interfaces::Prediction);
	VMTPrediction.HookMethod((DWORD)&Hooked_InPrediction, 14);
	//Utilities::Log("InPrediction Hooked");

	// Chams
	VMTModelRender.Initialise((DWORD*)Interfaces::ModelRender);
	oDrawModelExecute = (DrawModelEx_)VMTModelRender.HookMethod((DWORD)&Hooked_DrawModelExecute, Offsets::VMT::ModelRender_DrawModelExecute);
	//Utilities::Log("DrawModelExecute Hooked");

	// Setup ClientMode Hooks
	VMTClientMode.Initialise((DWORD*)Interfaces::ClientMode);
	VMTClientMode.HookMethod((DWORD)CreateMoveClient_Hooked, 24);

	oOverrideView = (OverrideViewFn)VMTClientMode.HookMethod((DWORD)&Hooked_OverrideView, 18);
	VMTClientMode.HookMethod((DWORD)&GGetViewModelFOV, 35);

	VMTEventManager.Initialise((DWORD*)Interfaces::EventManager);
	oFireEventClientSide = (FireEventClientSideFn)VMTEventManager.HookMethod((DWORD)&Hooked_FireEventClientSide, 9);

	// Setup client hooks
	VMTClient.Initialise((DWORD*)Interfaces::Client);
	oFrameStageNotify = (FrameStageNotifyFn)VMTClient.HookMethod((DWORD)&Hooked_FrameStageNotify, 36);

}

void MovementCorrection(CUserCmd* pCmd)
{

}

//---------------------------------------------------------------------------------------------------------
//                                         Hooked Functions
//---------------------------------------------------------------------------------------------------------

void SetClanTag(const char* tag, const char* name)//190% paste
{
	static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(((DWORD)Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x53\x56\x57\x8B\xDA\x8B\xF9\xFF\x15\x00\x00\x00\x00\x6A\x24\x8B\xC8\x8B\x30", "xxxxxxxxx????xxxxxx")));
	pSetClanTag(tag, name);
}
void NoClantag()
{
	SetClanTag("", "");
}

void ClanTag()
{
	if (Menu::Window.MiscTab.OtherClantag.GetState())
	{
		SetClanTag("HACC", "HACC");
	}
}

bool __stdcall CreateMoveClient_Hooked(/*void* self, int edx,*/ float frametime, CUserCmd* pCmd)
{
	if (!pCmd->command_number)
		return true;

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{

		PVOID pebp;
		__asm mov pebp, ebp;
		bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
		bool& bSendPacket = *pbSendPacket;

		if (Menu::Window.MiscTab.OtherClantag.GetState() > 0)
			ClanTag();

		//	CUserCmd* cmdlist = *(CUserCmd**)((DWORD)Interfaces::pInput + 0xEC);
		//	CUserCmd* pCmd = &cmdlist[sequence_number % 150];


			// Backup for safety
		Vector origView = pCmd->viewangles;
		Vector viewforward, viewright, viewup, aimforward, aimright, aimup;
		Vector qAimAngles;
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);

		// Do da hacks
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal && pLocal->IsAlive())
			Hacks::MoveHacks(pCmd, bSendPacket);


		if (Menu::Window.VisualsTab.NightSky.GetState())
		{
			ConVar* sky = Interfaces::CVar->FindVar("sv_skyname");
			SpoofedConvar* sky_spoofed = new SpoofedConvar(sky);
			sky_spoofed->SetString("sky_csgo_night02");
		}

		//Movement Fix
		//GameUtils::CL_FixMove(pCmd, origView);
		qAimAngles.Init(0.0f, GetAutostrafeView().y, 0.0f); // if pCmd->viewangles.x > 89, set pCmd->viewangles.x instead of 0.0f on first
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &aimforward, &aimright, &aimup);
		Vector vForwardNorm;		Normalize(viewforward, vForwardNorm);
		Vector vRightNorm;			Normalize(viewright, vRightNorm);
		Vector vUpNorm;				Normalize(viewup, vUpNorm);

		// Original shit for movement correction
		float forward = pCmd->forwardmove;
		float right = pCmd->sidemove;
		float up = pCmd->upmove;
		if (forward > 450) forward = 450;
		if (right > 450) right = 450;
		if (up > 450) up = 450;
		if (forward < -450) forward = -450;
		if (right < -450) right = -450;
		if (up < -450) up = -450;
		pCmd->forwardmove = DotProduct(forward * vForwardNorm, aimforward) + DotProduct(right * vRightNorm, aimforward) + DotProduct(up * vUpNorm, aimforward);
		pCmd->sidemove = DotProduct(forward * vForwardNorm, aimright) + DotProduct(right * vRightNorm, aimright) + DotProduct(up * vUpNorm, aimright);
		pCmd->upmove = DotProduct(forward * vForwardNorm, aimup) + DotProduct(right * vRightNorm, aimup) + DotProduct(up * vUpNorm, aimup);

		// Angle normalisation
		if (Menu::Window.MiscTab.OtherSafeMode.GetState())
		{
			GameUtils::NormaliseViewAngle(pCmd->viewangles);

			if (pCmd->viewangles.z != 0.0f)
			{
				pCmd->viewangles.z = 0.00;
			}

			if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
			{
				Utilities::Log("Having to re-normalise!");
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
				Beep(750, 800); // Why does it do this
				if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
				{
					pCmd->viewangles = origView;
					pCmd->sidemove = right;
					pCmd->forwardmove = forward;
				}
			}
		}

		if (pCmd->viewangles.x > 90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}

		if (pCmd->viewangles.x < -90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}

		if (bSendPacket)
			LastAngleAA = pCmd->viewangles;
	}

	return false;
}

std::string GetTimeString()
{
	time_t current_time;
	struct tm *time_info;
	static char timeString[10];
	time(&current_time);
	time_info = localtime(&current_time);
	strftime(timeString, sizeof(timeString), "%X", time_info);
	return timeString;
}

// Paint Traverse Hooked function
void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	if (Menu::Window.VisualsTab.Active.GetState() && Menu::Window.VisualsTab.OtherNoScope.GetState() && strcmp("HudZoom", Interfaces::Panels->GetName(vguiPanel)) == 0)
		return;

	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);

	static unsigned int FocusOverlayPanel = 0;
	static bool FoundPanel = false;

	if (!FoundPanel)
	{
		PCHAR szPanelName = (PCHAR)Interfaces::Panels->GetName(vguiPanel);
		if (strstr(szPanelName, "MatSystemTopPanel"))
		{
			FocusOverlayPanel = vguiPanel;
			FoundPanel = true;
		}
	}
	else if (FocusOverlayPanel == vguiPanel)
	{

		if (Menu::Window.VisualsTab.Watermark.GetState())
		{
			Render::Text(7, 7, Color(66, 244, 72, 200), Render::Fonts::WaterMark, ("hacc"));
		}

		if (Menu::Window.RageBotTab.AntiAimEnable.GetState() == true)
		{
			Render::Text(7, 30, Color(193, 29, 29, 255), Render::Fonts::WaterMark, "AA: Enabled");
		}
		else
		{
			Render::Text(7, 30, Color(22, 74, 186, 255), Render::Fonts::WaterMark, "AA: Disabled");
		}

		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
			Hacks::DrawHacks();

		if (Menu::Window.VisualsTab.lbyidicador.GetState())
		{
			CUserCmd* cmdlist = *(CUserCmd**)((DWORD)Interfaces::pInput + 0xEC);
			CUserCmd* pCmd = cmdlist;

			IClientEntity* localplayer = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
			RECT TextSize = Render::GetTextSize(Render::Fonts::LBY, "LBY");
			RECT scrn = Render::GetViewport();
			if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
				if (pCmd->viewangles.y - *localplayer->GetLowerBodyYawTarget() >= -35 && pCmd->viewangles.y - *localplayer->GetLowerBodyYawTarget() <= 35)
					Render::Text(10, scrn.bottom - 40, Color(255, 0, 0, 255), Render::Fonts::LBY, "LBY");
				else
					Render::Text(10, scrn.bottom - 40, Color(0, 255, 0, 255), Render::Fonts::LBY, "LBY");
		}

		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && Menu::Window.VisualsTab.OtherHitmarker.GetState())
			hitmarker::singleton()->on_paint();

		if (Menu::Window.VisualsTab.Active.GetState())
		{
			float AmbientRedAmount = Menu::Window.VisualsTab.AmbientRed.GetValue();
			float AmbientGreenAmount = Menu::Window.VisualsTab.AmbientGreen.GetValue();
			float AmbientBlueAmount = Menu::Window.VisualsTab.AmbientBlue.GetValue();

			ConVar* AmbientRedCvar = Interfaces::CVar->FindVar("mat_ambient_light_r");
			*(float*)((DWORD)&AmbientRedCvar->fnChangeCallback + 0xC) = NULL;
			AmbientRedCvar->SetValue(AmbientRedAmount);

			ConVar* AmbientGreenCvar = Interfaces::CVar->FindVar("mat_ambient_light_g");
			*(float*)((DWORD)&AmbientGreenCvar->fnChangeCallback + 0xC) = NULL;
			AmbientGreenCvar->SetValue(AmbientGreenAmount);

			ConVar* AmbientBlueCvar = Interfaces::CVar->FindVar("mat_ambient_light_b");
			*(float*)((DWORD)&AmbientBlueCvar->fnChangeCallback + 0xC) = NULL;
			AmbientBlueCvar->SetValue(AmbientBlueAmount);
		}

		Menu::DoUIFrame();
	}
}

// InPrediction Hooked Function
bool __stdcall Hooked_InPrediction()
{
	bool result;
	static InPrediction_ origFunc = (InPrediction_)Hooks::VMTPrediction.GetOriginalFunction(14);
	static DWORD *ecxVal = Interfaces::Prediction;
	result = origFunc(ecxVal);

	// If we are in the right place where the player view is calculated
	// Calculate the change in the view and get rid of it
	if (Menu::Window.VisualsTab.OtherNoVisualRecoil.GetState() && (DWORD)(_ReturnAddress()) == Offsets::Functions::dwCalcPlayerView)
	{
		IClientEntity* pLocalEntity = NULL;

		float* m_LocalViewAngles = NULL;

		__asm
		{
			MOV pLocalEntity, ESI
			MOV m_LocalViewAngles, EBX
		}

		Vector viewPunch = pLocalEntity->localPlayerExclusive()->GetViewPunchAngle();
		Vector aimPunch = pLocalEntity->localPlayerExclusive()->GetAimPunchAngle();

		m_LocalViewAngles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
		m_LocalViewAngles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
		m_LocalViewAngles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		return true;
	}

	return result;
}

int Kills2 = 0;
int Kills = 0;
bool RoundInfo = false;
size_t Delay = 0;
bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event)
{






	return oFireEventClientSide(ECX, Event);
}


// DrawModelExec for chams and shit
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld)
{
	Color color;
	float flColor[3] = { 0.f };
	static IMaterial* CoveredLit = CreateMaterial(true);
	static IMaterial* OpenLit = CreateMaterial(false);
	static IMaterial* CoveredFlat = CreateMaterial(true, false);
	static IMaterial* OpenFlat = CreateMaterial(false, false);
	static IMaterial* Chrome = CreateMaterial("$envmap env_cube");
	bool DontDraw = false;

	if (Menu::Window.VisualsTab.Active.GetState())
	{
		const char* ModelName = Interfaces::ModelInfo->GetModelName((model_t*)pInfo.pModel);
		IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);
		IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

		int ChamsStyle = Menu::Window.VisualsTab.OptionsChams.GetIndex();
		int HandsStyle = Menu::Window.VisualsTab.OtherNoHands.GetIndex();
		int Weps = Menu::Window.VisualsTab.OtherWepChams.GetIndex();


	

		if (pLocal && strstr(ModelName, "models/player"))
		{
			if (pModelEntity && ChamsStyle != 0)
			{

				if (ChamsStyle == 1)
				{
					if (pLocal->IsAlive())
					{
						IMaterial *Pplayer = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/trophy_majors/crystal_blue", "Model textures");
						IMaterial* playerr = Interfaces::MaterialSystem->FindMaterial(ModelName, "Model textures");
						playerr->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						Interfaces::ModelRender->ForcedMaterialOverride(Pplayer);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

					}
				}
			}
		}
		else if (strstr(ModelName, "arms"))
		{
			if (HandsStyle != 0 && pLocal && pLocal->IsAlive())
			{
				if (HandsStyle == 1)
				{
					DontDraw = true;
				}
				else if (HandsStyle == 2)
				{

					if (pLocal->IsAlive())
					{
						//color.SetColor(HandR, HandG, HandB);
						IMaterial *TestHands2 = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/trophy_majors/gold", "Model textures");
						IMaterial* Hands = Interfaces::MaterialSystem->FindMaterial(ModelName, "Model textures");
						//Hands->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
						Hands->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						Interfaces::ModelRender->ForcedMaterialOverride(TestHands2);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
				}
				else if (HandsStyle == 3)
				{

					if (pLocal->IsAlive())
					{
						//color.SetColor(HandR, HandG, HandB);
						//IMaterial *GlassChams = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER);
						static float colors[3] = { 255.f, 20.f, 20.f };
						IMaterial *GlassHands = Interfaces::MaterialSystem->FindMaterial("models/player/ct_fbi/ct_fbi_glass", "Model textures");
						IMaterial* Hands = Interfaces::MaterialSystem->FindMaterial(ModelName, "Model textures");
						//Hands->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
						Hands->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						//Interfaces::RenderView->SetColorModulation(colors);
						//Interfaces::RenderView->SetBlend(0.3);
						Interfaces::ModelRender->ForcedMaterialOverride(GlassHands);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
				}
				else if (HandsStyle == 4)
				{

					if (pLocal->IsAlive())
					{
						static float colors[3] = { 255.f, 20.f, 20.f };
						IMaterial *GlassHands = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/trophy_majors/crystal_blue", "Model textures");
						IMaterial* Hands = Interfaces::MaterialSystem->FindMaterial(ModelName, "Model textures");
						Hands->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						Interfaces::ModelRender->ForcedMaterialOverride(GlassHands);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
				}
				else
				{
					static int counter = 0;
					static float colors[3] = { 1.f, 0.f, 0.f };

					if (colors[counter] >= 1.0f)
					{
						colors[counter] = 1.0f;
						counter += 1;
						if (counter > 2)
							counter = 0;
					}
					else
					{
						int prev = counter - 1;
						if (prev < 0) prev = 2;
						colors[prev] -= 0.05f;
						colors[counter] += 0.05f;
					}

					Interfaces::RenderView->SetColorModulation(colors);
					Interfaces::RenderView->SetBlend(0.3);
					Interfaces::ModelRender->ForcedMaterialOverride(OpenLit);
				}
			}
		}

		else if (strstr(ModelName, "models/weapons"))
		{
			if (Weps != 0 && pLocal && pLocal->IsAlive())
			{
				if (Weps == 1)
				{
					if (pLocal->IsAlive())
					{
						static float colors[3] = { 255.f, 20.f, 20.f };
						IMaterial *GlassWeps = Interfaces::MaterialSystem->FindMaterial("models/player/ct_fbi/ct_fbi_glass", "Model textures");
						IMaterial* weps = Interfaces::MaterialSystem->FindMaterial(ModelName, "Model textures");
						weps->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						Interfaces::ModelRender->ForcedMaterialOverride(GlassWeps);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
				}
				else if (Weps == 2)
				{
					if (pLocal->IsAlive())
					{
						static float colors[3] = { 255.f, 20.f, 20.f };
						IMaterial *GlassWeps = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/trophy_majors/gold", "Model textures");
						IMaterial* weps = Interfaces::MaterialSystem->FindMaterial(ModelName, "Model textures");
						weps->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						Interfaces::ModelRender->ForcedMaterialOverride(GlassWeps);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}

				}
				else if (Weps == 3)
				{
					if (pLocal->IsAlive())
					{
						static float colors[3] = { 255.f, 20.f, 20.f };
						IMaterial *GlassWeps = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/trophy_majors/crystal_blue", "Model textures");
						IMaterial* weps = Interfaces::MaterialSystem->FindMaterial(ModelName, "Model textures");
						weps->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						Interfaces::ModelRender->ForcedMaterialOverride(GlassWeps);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
				}
			}
		}
	}




	if (!DontDraw)
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	Interfaces::ModelRender->ForcedMaterialOverride(NULL);



}


int RandomInt(int min, int max)
{
	return rand() % max + min;
}

bool bGlovesNeedUpdate;
void ApplyCustomGloves(IClientEntity* pLocal)
{
	if (Menu::Window.MiscTab.EnableGloves.GetState())
	{
		if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
			return;

		if (bGlovesNeedUpdate || !pLocal->IsAlive())
		{
			DWORD* hMyWearables = (DWORD*)((size_t)pLocal + 0x2EF4);

			if (!Interfaces::EntList->GetClientEntity(hMyWearables[0] & 0xFFF))
			{
				for (ClientClass* pClass = Interfaces::Client->GetAllClasses(); pClass; pClass = pClass->m_pNext)
				{
					if (pClass->m_ClassID != (int)CSGOClassID::CEconWearable)
						continue;

					int iEntry = (Interfaces::EntList->GetHighestEntityIndex() + 1);
					int	iSerial = RandomInt(0x0, 0xFFF);

					pClass->m_pCreateFn(iEntry, iSerial);
					hMyWearables[0] = iEntry | (iSerial << 16);

					break;
				}
			}

			player_info_t LocalPlayerInfo;
			Interfaces::Engine->GetPlayerInfo(Interfaces::Engine->GetLocalPlayer(), &LocalPlayerInfo);

			CBaseCombatWeapon* glovestochange = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntity(hMyWearables[0] & 0xFFF);

			if (!glovestochange)
				return;


			switch (Menu::Window.MiscTab.GloveModel.GetIndex())
			{
			case 1:
			{
				*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 5027;
				glovestochange->SetModelIndex(Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl"));
				break;
			}
			case 2:
			{
				*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 5032;
				glovestochange->SetModelIndex(Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl"));
				break;
			}
			case 3:
			{
				*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 5031;
				glovestochange->SetModelIndex(Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl"));
				break;
			}
			case 4:
			{
				*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 5030;
				glovestochange->SetModelIndex(Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl"));
				break;
			}
			case 5:
			{
				*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 5033;
				glovestochange->SetModelIndex(Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl"));
				break;
			}
			case 6:
			{
				*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 5034;
				glovestochange->SetModelIndex(Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl"));
				break;
			}
			default:
				break;
			}

			switch (Menu::Window.MiscTab.GloveSkin.GetIndex())
			{
			case 0:
				break;
			case 1:
				*glovestochange->FallbackPaintKit() = 10007;
				break;
			case 2:
				*glovestochange->FallbackPaintKit() = 10006;
				break;
			case 3:
				*glovestochange->FallbackPaintKit() = 10039;
				break;
			case 4:
				*glovestochange->FallbackPaintKit() = 10008;
				break;
			case 5:
				*glovestochange->FallbackPaintKit() = 10021;
				break;
			case 6:
				*glovestochange->FallbackPaintKit() = 10036;
				break;
			case 7:
				*glovestochange->FallbackPaintKit() = 10009;
				break;
			case 8:
				*glovestochange->FallbackPaintKit() = 10010;
				break;
			case 9:
				*glovestochange->FallbackPaintKit() = 10016;
				break;
			case 10:
				*glovestochange->FallbackPaintKit() = 10013;
				break;
			case 11:
				*glovestochange->FallbackPaintKit() = 10040;
				break;
			case 12:
				*glovestochange->FallbackPaintKit() = 10015;
				break;
			case 13:
				*glovestochange->FallbackPaintKit() = 10037;
				break;
			case 14:
				*glovestochange->FallbackPaintKit() = 10038;
				break;
			case 15:
				*glovestochange->FallbackPaintKit() = 10018;
				break;
			case 16:
				*glovestochange->FallbackPaintKit() = 10019;
				break;
			case 17:
				*glovestochange->FallbackPaintKit() = 10026;
				break;
			case 18:
				*glovestochange->FallbackPaintKit() = 10028;
				break;
			case 19:
				*glovestochange->FallbackPaintKit() = 10027;
				break;
			case 20:
				*glovestochange->FallbackPaintKit() = 10024;
				break;
			case 21:
				*glovestochange->FallbackPaintKit() = 10033;
				break;
			case 22:
				*glovestochange->FallbackPaintKit() = 10034;
				break;
			case 23:
				*glovestochange->FallbackPaintKit() = 10035;
				break;
			case 24:
				*glovestochange->FallbackPaintKit() = 10030;
				break;
			}

			*glovestochange->m_AttributeManager()->m_Item()->ItemIDHigh() = -1;
			*glovestochange->FallbackWear() = 0.001f;
			*glovestochange->m_AttributeManager()->m_Item()->AccountID() = LocalPlayerInfo.xuidlow;


			glovestochange->PreDataUpdate(0);
			bGlovesNeedUpdate = false;
		}
	}
}


// Hooked FrameStageNotify for removing visual recoil
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{
	DWORD eyeangles = NetVar.GetNetVar(0xBFEA4E7B);
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	IClientEntity* pEntity;

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && curStage == FRAME_RENDER_START)
	{

		if (pLocal->IsAlive())
		{
			if (*(bool*)((DWORD)Interfaces::pInput + 0xA5))
				*(Vector*)((DWORD)pLocal + 0x31C8) = LastAngleAA;
		}

		if (Menu::Window.MiscTab.OtherThirdperson.GetState())
		{
			static bool rekt = false;
			if (!rekt)
			{
				ConVar* sv_cheats = Interfaces::CVar->FindVar("sv_cheats");
				SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
				sv_cheats_spoofed->SetInt(1);
				rekt = true;
			}
		}

		static bool rekt1 = false;
		if (Menu::Window.MiscTab.OtherThirdperson.GetState() && pLocal->IsAlive() && pLocal->IsScoped() == 0)
		{
			if (!rekt1)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				rekt1 = true;
			}
		}
		else if (!Menu::Window.MiscTab.OtherThirdperson.GetState())
		{
			rekt1 = false;
		}

		static bool rekt = false;
		if (!Menu::Window.MiscTab.OtherThirdperson.GetState() || pLocal->IsAlive() == 0)
		{
			if (!rekt)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("firstperson");
				rekt = true;
			}
		}
		else if (Menu::Window.MiscTab.OtherThirdperson.GetState() || pLocal->IsAlive() == 0)
		{
			rekt = false;
		}

		static bool meme = false;
		if (Menu::Window.MiscTab.OtherThirdperson.GetState() && pLocal->IsScoped() == 0)
		{
			if (!meme)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				meme = true;
			}
		}
		else if (pLocal->IsScoped())
		{
			meme = false;
		}

		static bool kek = false;
		if (Menu::Window.MiscTab.OtherThirdperson.GetState() && pLocal->IsAlive())
		{
			if (!kek)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				kek = true;
			}
		}
		else if (pLocal->IsAlive() == 0)
		{
			kek = false;
		}
	}



	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		ResolverSetup::GetInst().FSN(pEntity, curStage);
		int iBayonet = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
		int iButterfly = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
		int iFlip = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
		int iGut = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
		int iKarambit = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
		int iM9Bayonet = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
		int iHuntsman = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
		int iFalchion = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
		int iDagger = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_push.mdl");
		int iBowie = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");
		int iGunGame = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_gg.mdl");

		for (int i = 0; i <= Interfaces::EntList->GetHighestEntityIndex(); i++)
		{
			IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);

			if (pEntity)
			{
				ApplyCustomGloves(pLocal);

				ULONG hOwnerEntity = *(PULONG)((DWORD)pEntity + 0x148);

				IClientEntity* pOwner = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)hOwnerEntity);

				if (pOwner)
				{
					if (pOwner == pLocal)
					{
						std::string sWeapon = Interfaces::ModelInfo->GetModelName(pEntity->GetModel());

						if (!(sWeapon.find("models/weapons", 0) != std::string::npos))
							continue;

						if (sWeapon.find("c4_planted", 0) != std::string::npos)
							continue;

						if (sWeapon.find("thrown", 0) != std::string::npos)
							continue;

						if (sWeapon.find("smokegrenade", 0) != std::string::npos)
							continue;

						if (sWeapon.find("flashbang", 0) != std::string::npos)
							continue;

						if (sWeapon.find("fraggrenade", 0) != std::string::npos)
							continue;

						if (sWeapon.find("molotov", 0) != std::string::npos)
							continue;

						if (sWeapon.find("decoy", 0) != std::string::npos)
							continue;

						if (sWeapon.find("incendiarygrenade", 0) != std::string::npos)
							continue;

						if (sWeapon.find("ied", 0) != std::string::npos)
							continue;

						if (sWeapon.find("w_eq_", 0) != std::string::npos)
							continue;

						CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)pEntity;

						ClientClass *pClass = Interfaces::Client->GetAllClasses();

					}
				}

			}
		}
	}

	oFrameStageNotify(curStage);
}


void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup)
{
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		if (Menu::Window.VisualsTab.Active.GetState() && pLocal->IsAlive() && !pLocal->IsScoped())
		{
			if (pSetup->fov = 90)
				pSetup->fov = Menu::Window.VisualsTab.OtherFOV.GetValue();
		}

		oOverrideView(ecx, edx, pSetup);
	}

}

void GetViewModelFOV(float& fov)
{
	IClientEntity* localplayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{

		if (!localplayer)
			return;


		if (Menu::Window.VisualsTab.Active.GetState())
		fov += Menu::Window.VisualsTab.OtherViewmodelFOV.GetValue();
	}
}

float __stdcall GGetViewModelFOV()
{
	float fov = Hooks::VMTClientMode.GetMethod<oGetViewModelFOV>(35)();

	GetViewModelFOV(fov);

	return fov;
}

void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw)
{
	static DWORD oRenderView = Hooks::VMTRenderView.GetOriginalFunction(6);

	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	__asm
	{
		PUSH whatToDraw
		PUSH nClearFlags
		PUSH hudViewSetup
		PUSH setup
		MOV ECX, ecx
		CALL oRenderView
	}
} //hooked for no reason yay

class player_hurt_listener
	: public IGameEventListener2
{
public:
	void start()
	{
		if (!g_pGameEventManager->AddListener(this, "player_hurt", false)) {
			throw std::exception("Failed to register the event");
		}
	}
	void stop()
	{
		g_pGameEventManager->RemoveListener(this);
	}
	void FireGameEvent(IGameEvent *event) override
	{
		hitmarker::singleton()->on_fire_event(event);
	}
	int GetEventDebugID(void) override
	{
		return 0x2A;
	}
};


