#include "Resolver.h"
#include "Ragebot.h"
#include "Hooks.h"

void LowerBodyYawFix(IClientEntity* pEntity)
{
	if (Menu::Window.RageBotTab.LowerbodyFix.GetState())
	{
		if (!pEntity) return;
		if (pEntity->GetClientClass()->m_ClassID != (int)CSGOClassID::CCSPlayer) return;
		if (!pEntity->IsAlive() || !pEntity->GetActiveWeaponHandle()) return;
		if (Interfaces::Engine->GetLocalPlayer()) return;

		auto EyeAngles = pEntity->GetEyeAnglesXY();
		if (pEntity->GetVelocity().Length() > 1 && (pEntity->GetFlags() & (int)pEntity->GetFlags() & FL_ONGROUND))
			EyeAngles->y = pEntity->GetLowerBodyYaw();
	}
}

void PitchCorrection()
{
	CUserCmd* pCmd;
	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
	{
		IClientEntity* pLocal = hackManager.pLocal();
		IClientEntity *player = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

		if (!player || player->IsDormant() || player->GetHealth() < 1 || (DWORD)player == (DWORD)pLocal)
			continue;

		if (!player)
			continue;

		if (pLocal)
			continue;

		if (pLocal && player && pLocal->IsAlive())
		{
			if (Menu::Window.RageBotTab.AdvancedResolver.GetState())
			{
				Vector* eyeAngles = player->GetEyeAnglesXY();
				if (eyeAngles->x < -179.f) eyeAngles->x += 360.f;
				else if (eyeAngles->x > 90.0 || eyeAngles->x < -90.0) eyeAngles->x = 89.f;
				else if (eyeAngles->x > 89.0 && eyeAngles->x < 91.0) eyeAngles->x -= 90.f;
				else if (eyeAngles->x > 179.0 && eyeAngles->x < 181.0) eyeAngles->x -= 180;
				else if (eyeAngles->x > -179.0 && eyeAngles->x < -181.0) eyeAngles->x += 180;
				else if (fabs(eyeAngles->x) == 0) eyeAngles->x = std::copysign(89.0f, eyeAngles->x);
			}
		}
	}
}

void PVSFIX()
{
	for (int i = 1; i < Interfaces::Globals->maxClients; i++)
	{
		if (i == Interfaces::Engine->GetLocalPlayer()) continue;
		IClientEntity* pCurEntity = Interfaces::EntList->GetClientEntity(i);
		if (!pCurEntity) continue;
		*(int*)((uintptr_t)pCurEntity + 0xA30) = Interfaces::Globals->framecount;
		*(int*)((uintptr_t)pCurEntity + 0xA28) = 0;
	}
}

void NormalizeVector(float& lowerDelta) {
	for (int i = 0; i < 3; ++i) {
		while (lowerDelta > 180.f)
			lowerDelta -= 360.f;

		while (lowerDelta < -180.f)
			lowerDelta += 360.f;
	}
	lowerDelta = 0.f;
}

float OldLowerBodyYaw[64];
float YawDelta[64];
float reset[64];
float Delta[64];
float Resolved_angles[64];
int iSmart;
static int jitter = -1;




void ResolverSetup::Resolve(IClientEntity* pEntity)
{
		PVSFIX();


	bool MeetsLBYReq;
	if (pEntity->GetFlags() & FL_ONGROUND)
		MeetsLBYReq = true;
	else
		MeetsLBYReq = false;

	bool IsMoving;
	if (pEntity->GetVelocity().Length2D() >= 0.5)
		IsMoving = true;
	else
		IsMoving = false;

	ResolverSetup::NewANgles[pEntity->GetIndex()] = *pEntity->GetEyeAnglesXY();
	ResolverSetup::newlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::newsimtime = pEntity->GetSimulationTime();
	ResolverSetup::newdelta[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y;
	ResolverSetup::newlbydelta[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::finaldelta[pEntity->GetIndex()] = ResolverSetup::newdelta[pEntity->GetIndex()] - ResolverSetup::storeddelta[pEntity->GetIndex()];
	ResolverSetup::finallbydelta[pEntity->GetIndex()] = ResolverSetup::newlbydelta[pEntity->GetIndex()] - ResolverSetup::storedlbydelta[pEntity->GetIndex()];
	if (newlby == storedlby)
		ResolverSetup::lbyupdated = false;
	else
		ResolverSetup::lbyupdated = true;


	if (Menu::Window.RageBotTab.AimbotResolver.GetState()) // shoutout to Mirror.tk V3 lmao
	{

		pEntity->GetEyeAnglesXY()->y = 160.f;
		{
			if (Globals::missedshots > 1 && Globals::missedshots < 21)
			{
				if (MeetsLBYReq && lbyupdated)
				{
					pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
				}
				else if (!MeetsLBYReq && lbyupdated)
				{
					switch (Globals::Shots % 1)
					{
					case 1:
						pEntity->GetEyeAnglesXY()->y = -30 + rand() % 45 - rand() % 51;
						break;
					case 2:
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 20 - rand() % 35;
						break;
					case 3:
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 45 % 30 - rand() % 90;
						break;
					case 4:
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 90 - rand() % 90 - rand() % 40;
						break;
					case 5:
						pEntity->GetEyeAnglesXY()->y = -160 - rand() % 90 - rand() % 40;
						break;
					}
				}
				else
					pEntity->GetEyeAnglesXY()->y = rand() % 180 - rand() % 45;
			}

			else if (Globals::missedshots >= 1 && Globals::missedshots <= 2)
			{
				if (MeetsLBYReq && lbyupdated)
				{
					pEntity->GetEyeAnglesXY()->y = ResolverSetup::finallbydelta[pEntity->GetIndex()];
				}
				else
					pEntity->GetEyeAnglesXY()->y = ResolverSetup::finaldelta[pEntity->GetIndex()];
			}
			else
			{
				if (MeetsLBYReq && lbyupdated)
				{
					bool timer;
					if (timer)
						pEntity->GetEyeAnglesXY()->y = ResolverSetup::finallbydelta[pEntity->GetIndex()] + rand() % 20;
					else
						pEntity->GetEyeAnglesXY()->y = ResolverSetup::finallbydelta[pEntity->GetIndex()] - rand() % 100;
					timer = !timer;
				}
				else
				{
					bool timer;
					if (timer)
						pEntity->GetEyeAnglesXY()->y = ResolverSetup::finaldelta[pEntity->GetIndex()] + rand() % 20;
					else
						pEntity->GetEyeAnglesXY()->y = ResolverSetup::finaldelta[pEntity->GetIndex()] - rand() % 20;
					timer = !timer;
				}

				PitchCorrection();
				LowerBodyYawFix(pEntity);
			}
		}


		if (Menu::Window.RageBotTab.AimbotResolverBrute.GetState()) // Bruteforce
		{
			Vector AimAngle;
			CalcAngleYawOnly(pEntity->GetEyePosition(), pEntity->GetAbsOrigin2(), AimAngle);
			NormalizeVector(AimAngle);
			int num = Globals::Shots % 5;
			if (GetAsyncKeyState(VK_MENU)) {
				switch (num) {
				case 0:pEntity->GetEyeAnglesXY()->y = AimAngle.y - 0; break;
				case 1:pEntity->GetEyeAnglesXY()->y = AimAngle.y - 15; break;
				case 2:pEntity->GetEyeAnglesXY()->y = AimAngle.y + 40; break;
				case 3:pEntity->GetEyeAnglesXY()->y = AimAngle.y - 40; break;
				case 4:pEntity->GetEyeAnglesXY()->y = AimAngle.y + 15; break;
				}
			}

		}
	}

}

void ResolverSetup::StoreFGE(IClientEntity* pEntity)
{
	ResolverSetup::storedanglesFGE = pEntity->GetEyeAnglesXY()->y;
	ResolverSetup::storedlbyFGE = pEntity->GetLowerBodyYaw();
	ResolverSetup::storedsimtimeFGE = pEntity->GetSimulationTime();
}

void ResolverSetup::StoreThings(IClientEntity* pEntity)
{
	ResolverSetup::StoredAngles[pEntity->GetIndex()] = *pEntity->GetEyeAnglesXY();
	ResolverSetup::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::storedsimtime = pEntity->GetSimulationTime();
	ResolverSetup::storeddelta[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y;
	ResolverSetup::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
}

void ResolverSetup::CM(IClientEntity* pEntity)
{
	for (int x = 1; x < Interfaces::Engine->GetMaxClients(); x++)
	{

		pEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(x);

		if (!pEntity
			|| pEntity == hackManager.pLocal()
			|| pEntity->IsDormant()
			|| !pEntity->IsAlive())
			continue;

		ResolverSetup::StoreThings(pEntity);
	}
}


	
void ResolverSetup::FSN(IClientEntity* pEntity, ClientFrameStage_t stage)
{
	if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (int i = 1; i < Interfaces::Engine->GetMaxClients(); i++)
		{

			pEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

			if (!pEntity
				|| pEntity == hackManager.pLocal()
				|| pEntity->IsDormant()
				|| !pEntity->IsAlive())
				continue;

			ResolverSetup::Resolve(pEntity);
		}
	}
}