#include "RageBot.h"
#include "RenderManager.h"
#include "Resolver.h"
#include "Autowall.h"
#include "Hooks.h"
#include <iostream>
#include "UTIL Functions.h"
#define TIME_TO_TICKS( dt )	( ( int )( 0.5f + ( float )( dt ) / Interfaces::Globals->interval_per_tick ) )

void CRageBot::Init()
{
	IsAimStepping = false;
	IsLocked = false;
	TargetID = -1;
}

void CRageBot::Draw()
{

}

bool IsAbleToShoot(IClientEntity* pLocal)
{
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (!pLocal)
		return false;

	if (!pWeapon)
		return false;

	float flServerTime = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;

	return (!(pWeapon->GetNextPrimaryAttack() > flServerTime));
}

float hitchance(IClientEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	float hitchance = 75;
	if (!pWeapon) return 0;
	if (Menu::Window.RageBotTab.AccuracyHitchance.GetValue() > 1)
	{
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;

	}
	return hitchance;
}

float InterpolationFix()
{
	static ConVar* cvar_cl_interp = Interfaces::CVar->FindVar("cl_interp");
	static ConVar* cvar_cl_updaterate = Interfaces::CVar->FindVar("cl_updaterate");
	static ConVar* cvar_sv_maxupdaterate = Interfaces::CVar->FindVar("sv_maxupdaterate");
	static ConVar* cvar_sv_minupdaterate = Interfaces::CVar->FindVar("sv_minupdaterate");
	static ConVar* cvar_cl_interp_ratio = Interfaces::CVar->FindVar("cl_interp_ratio");

	IClientEntity* pLocal = hackManager.pLocal();
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	float cl_interp = cvar_cl_interp->GetFloat();
	int cl_updaterate = cvar_cl_updaterate->GetInt();
	int sv_maxupdaterate = cvar_sv_maxupdaterate->GetInt();
	int sv_minupdaterate = cvar_sv_minupdaterate->GetInt();
	int cl_interp_ratio = cvar_cl_interp_ratio->GetInt();

	if (sv_maxupdaterate <= cl_updaterate)
		cl_updaterate = sv_maxupdaterate;

	if (sv_minupdaterate > cl_updaterate)
		cl_updaterate = sv_minupdaterate;

	float new_interp = (float)cl_interp_ratio / (float)cl_updaterate;

	if (new_interp > cl_interp)
		cl_interp = new_interp;

	return max(cl_interp, cl_interp_ratio / cl_updaterate);
}

bool CanOpenFire()
{
	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalEntity)
		return false;

	CBaseCombatWeapon* entwep = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocalEntity->GetActiveWeaponHandle());

	float flServerTime = (float)pLocalEntity->GetTickBase() * Interfaces::Globals->interval_per_tick;
	float flNextPrimaryAttack = entwep->GetNextPrimaryAttack();

	std::cout << flServerTime << " " << flNextPrimaryAttack << std::endl;

	return !(flNextPrimaryAttack > flServerTime);
}

void CRageBot::Move(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalEntity)
		return;

	if (!Menu::Window.RageBotTab.Active.GetState())
		return;

	if (Menu::Window.RageBotTab.AntiAimEnable.GetState())
	{
		static int ChokedPackets = -1;

		CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
		if (!pWeapon)
			return;

		if (ChokedPackets < 1 && pLocalEntity->GetLifeState() == LIFE_ALIVE && pCmd->buttons & IN_ATTACK && CanOpenFire() && GameUtils::IsBallisticWeapon(pWeapon))
		{
			bSendPacket = false;
		}
		else
		{
			if (pLocalEntity->GetLifeState() == LIFE_ALIVE)
			{
				DoAntiAim(pCmd, bSendPacket);
			}
			ChokedPackets = -1;
		}
	}

	if (Menu::Window.RageBotTab.AimbotEnable.GetState())
		DoAimbot(pCmd, bSendPacket);

	if (Menu::Window.RageBotTab.AccuracyPositionAdjustment.GetState())
		pCmd->tick_count = TIME_TO_TICKS(InterpolationFix());

	if (Menu::Window.RageBotTab.AccuracyRecoil.GetState())
		DoNoRecoil(pCmd);

	if (Menu::Window.RageBotTab.AimbotAimStep.GetState())
	{
		Vector AddAngs = pCmd->viewangles - LastAngle;
		if (AddAngs.Length2D() > 25.f)
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 25;
			pCmd->viewangles = LastAngle + AddAngs;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}

	LastAngle = pCmd->viewangles;
}

Vector BestPoint(IClientEntity *targetPlayer, Vector &final)
{
	IClientEntity* pLocal = hackManager.pLocal();

	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = targetPlayer;
	ray.Init(final + Vector(0, 0, 10), final);
	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	final = tr.endpos;
	return final;
}

void CRageBot::DoAimbot(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pTarget = nullptr;
	IClientEntity* pLocal = hackManager.pLocal();
	Vector Start = pLocal->GetViewOffset() + pLocal->GetOrigin();
	bool FindNewTarget = true;

	CSWeaponInfo* weapInfo = ((CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle()))->GetCSWpnData();

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (pWeapon)
	{
		if (pWeapon->GetAmmoInClip() == 0 || !GameUtils::IsBallisticWeapon(pWeapon))
		{
			return;
		}
	}
	else
		return;

	if (IsLocked && TargetID >= 0 && HitBox >= 0)
	{
		pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		if (pTarget  && TargetMeetsRequirements(pTarget))
		{
			HitBox = HitScan(pTarget);
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
				Vector View;
				Interfaces::Engine->GetViewAngles(View);
				float FoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);
				if (FoV < Menu::Window.RageBotTab.AimbotFov.GetValue())
					FindNewTarget = false;
			}
		}
	}

	if (GameUtils::IsRevolver(pWeapon) && Menu::Window.RageBotTab.AutoRevolver.GetState())
	{
		static int delay = 0;
		delay++;

		if (delay <= 15)
			pCmd->buttons |= IN_ATTACK;
		else
			delay = 0;
	}

	if (FindNewTarget)
	{
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;

		switch (Menu::Window.RageBotTab.TargetSelection.GetIndex())
		{
		case 0:
			TargetID = GetTargetCrosshair();
			break;
		case 1:
			TargetID = GetTargetDistance();
			break;
		case 2:
			TargetID = GetTargetHealth();
			break;
		case 3:
			TargetID = GetTargetThreat(pCmd);
			break;
		case 4:
			TargetID = GetTargetNextShot();
			break;
		}

		if (TargetID >= 0)
		{
			pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		}
		else
		{
			pTarget = nullptr;
			HitBox = -1;
		}
	}

	Globals::Target = pTarget;
	Globals::TargetID = TargetID;

	if (TargetID >= 0 && pTarget)
	{
		HitBox = HitScan(pTarget);

		if (!CanOpenFire())
			return;

		if (Menu::Window.RageBotTab.AimbotKeyPress.GetState())
		{
			int Key = Menu::Window.RageBotTab.AimbotKeyBind.GetKey();
			if (Key >= 0 && !GUI.GetKeyState(Key))
			{
				TargetID = -1;
				pTarget = nullptr;
				HitBox = -1;
				return;
			}
		}

		int StopKey = Menu::Window.RageBotTab.AimbotStopKey.GetKey();
		if (StopKey >= 0 && GUI.GetKeyState(StopKey))
		{
			TargetID = -1;
			pTarget = nullptr;
			HitBox = -1;
			return;
		}

		float pointscale = Menu::Window.RageBotTab.TargetPointscale.GetValue() - 5.f;

		Vector Point;
		Vector AimPoint = GetHitboxPosition(pTarget, HitBox) + Vector(0, 0, pointscale);

		if (Menu::Window.RageBotTab.TargetMultipoint.GetState())
		{
			Point = BestPoint(pTarget, AimPoint);
		}
		else
		{
			Point = AimPoint;
		}

		if (GameUtils::IsScopedWeapon(pWeapon) && !pWeapon->IsScoped() && Menu::Window.RageBotTab.AccuracyAutoScope.GetState()) // Autoscope
		{
			pCmd->buttons |= IN_ATTACK2;
		}
		else
		{
			if ((Menu::Window.RageBotTab.AccuracyHitchance.GetValue() * 1.5 <= hitchance(pLocal, pWeapon)) || Menu::Window.RageBotTab.AccuracyHitchance.GetValue() == 0 || *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() == 64)
			{
				if (AimAtPoint(pLocal, Point, pCmd, bSendPacket))
				{
					if (Menu::Window.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))
					{
						pCmd->buttons |= IN_ATTACK;
					}
					else
					{
						return;
					}
				}
				else if (Menu::Window.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))
				{
					pCmd->buttons |= IN_ATTACK;
				}
			}
		}

		if (IsAbleToShoot(pLocal) && pCmd->buttons & IN_ATTACK)
			Globals::Shots += 1;
	}
}

bool CRageBot::TargetMeetsRequirements(IClientEntity* pEntity)
{
	if (pEntity && pEntity->IsDormant() == false && pEntity->IsAlive() && pEntity->GetIndex() != hackManager.pLocal()->GetIndex())
	{
		ClientClass *pClientClass = pEntity->GetClientClass();
		player_info_t pinfo;
		if (pClientClass->m_ClassID == (int)CSGOClassID::CCSPlayer && Interfaces::Engine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			if (pEntity->GetTeamNum() != hackManager.pLocal()->GetTeamNum() || Menu::Window.RageBotTab.TargetFriendlyFire.GetState())
			{
				if (!pEntity->HasGunGameImmunity())
				{
					return true;
				}
			}
		}
	}

	return false;
}

float CRageBot::FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int aHitBox)
{
	CONST FLOAT MaxDegrees = 180.0f;

	Vector Angles = View;

	Vector Origin = ViewOffSet;

	Vector Delta(0, 0, 0);

	Vector Forward(0, 0, 0);

	AngleVectors(Angles, &Forward);
	Vector AimPos = GetHitboxPosition(pEntity, aHitBox);

	VectorSubtract(AimPos, Origin, Delta);

	Normalize(Delta, Delta);

	FLOAT DotProduct = Forward.Dot(Delta);

	return (acos(DotProduct) * (MaxDegrees / PI));
}

int CRageBot::GetTargetCrosshair()
{
	int target = -1;
	float minFoV = Menu::Window.RageBotTab.AimbotFov.GetValue();

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minFoV)
				{
					minFoV = fov;
					target = i;
				}
			}
		}
	}

	return target;
}

int CRageBot::GetTargetDistance()
{
	int target = -1;
	int minDist = 99999;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				int Distance = Difference.Length();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Distance < minDist && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minDist = Distance;
					target = i;
				}
			}
		}
	}

	return target;
}

int CRageBot::GetTargetNextShot()
{
	int target = -1;
	int minfov = 361;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minfov && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minfov = fov;
					target = i;
				}
				else
					minfov = 361;
			}
		}
	}

	return target;
}

float GetFov(const QAngle& viewAngle, const QAngle& aimAngle)
{
	Vector ang, aim;

	AngleVectors(viewAngle, &aim);
	AngleVectors(aimAngle, &ang);

	return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}

double inline __declspec (naked) __fastcall FASTSQRT(double n)
{
	_asm fld qword ptr[esp + 4]
		_asm fsqrt
	_asm ret 8
}

float VectorDistance(Vector v1, Vector v2)
{
	return FASTSQRT(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
}

int CRageBot::GetTargetThreat(CUserCmd* pCmd)
{
	auto iBestTarget = -1;
	float flDistance = 8192.f;

	IClientEntity* pLocal = hackManager.pLocal();

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			auto vecHitbox = pEntity->GetBonePos(NewHitBox);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				QAngle TempTargetAbs;
				CalcAngle(pLocal->GetEyePosition(), vecHitbox, TempTargetAbs);
				float flTempFOVs = GetFov(pCmd->viewangles, TempTargetAbs);
				float flTempDistance = VectorDistance(pLocal->GetOrigin(), pEntity->GetOrigin());
				if (flTempDistance < flDistance && flTempFOVs < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					flDistance = flTempDistance;
					iBestTarget = i;
				}
			}
		}
	}
	return iBestTarget;
}

int CRageBot::GetTargetHealth()
{
	int target = -1;
	int minHealth = 101;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Health < minHealth && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minHealth = Health;
					target = i;
				}
			}
		}
	}

	return target;
}

int CRageBot::HitScan(IClientEntity* pEntity)
{
	IClientEntity* pLocal = hackManager.pLocal();
	std::vector<int> HitBoxesToScan;

#pragma region GetHitboxesToScan
	int HitScanMode = Menu::Window.RageBotTab.TargetHitscan.GetIndex();
	int huso = (pEntity->GetHealth());
	int health = Menu::Window.RageBotTab.BaimIfUnderXHealth.GetValue();
	bool AWall = Menu::Window.RageBotTab.AccuracyAutoWall.GetState();
	bool Multipoint = Menu::Window.RageBotTab.TargetMultipoint.GetState();
	int TargetHitbox = Menu::Window.RageBotTab.TargetHitbox.GetIndex();
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (huso < health)
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
	}
	else
	{
		if (HitScanMode == 0)
		{
			switch (Menu::Window.RageBotTab.TargetHitbox.GetIndex())
			{
			case 0:
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				break;
			case 1:
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
				break;
			case 2:
				HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
				break;
			case 3:
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
				break;
			case 4:
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				break;
			}
		}

		else if (Menu::Window.RageBotTab.AWPAtBody.GetState() && GameUtils::AWP(pWeapon))
		{
			HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
		}

		else if (Menu::Window.RageBotTab.PreferBodyAim.GetState())
		{
			HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
		}
		else
		{
			switch (HitScanMode)
			{
			case 1:
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
			case 2:
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
			case 3:
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
			}
		}
	}
#pragma endregion Get the list of shit to scan

	for (auto HitBoxID : HitBoxesToScan)
	{
		if (AWall)
		{
			Vector Point = GetHitboxPosition(pEntity, HitBoxID);
			float Damage = 0.f;
			Color c = Color(255, 255, 255, 255);
			if (CanHit(Point, &Damage))
			{
				c = Color(0, 255, 0, 255);
				if (Damage >= Menu::Window.RageBotTab.AccuracyMinimumDamage.GetValue())
				{
					return HitBoxID;
				}
			}
		}
		else
		{
			if (GameUtils::IsVisible(hackManager.pLocal(), pEntity, HitBoxID))
				return HitBoxID;
		}
	}

	return -1;
}

void CRageBot::DoNoRecoil(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();
	if (pLocal)
	{
		Vector AimPunch = pLocal->localPlayerExclusive()->GetAimPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
		{
			pCmd->viewangles -= AimPunch * 2;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}
}

void CRageBot::aimAtPlayer(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (!pLocal || !pWeapon)
		return;

	Vector eye_position = pLocal->GetEyePosition();

	float best_dist = pWeapon->GetCSWpnData()->flRange;

	IClientEntity* target = nullptr;

	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			if (Globals::TargetID != -1)
				target = Interfaces::EntList->GetClientEntity(Globals::TargetID);
			else
				target = pEntity;

			Vector target_position = target->GetEyePosition();

			float temp_dist = eye_position.DistTo(target_position);

			if (best_dist > temp_dist)
			{
				best_dist = temp_dist;
				CalcAngle(eye_position, target_position, pCmd->viewangles);
			}
		}
	}
}

bool CRageBot::AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd, bool &bSendPacket)
{
	bool ReturnValue = false;

	if (point.Length() == 0) return ReturnValue;

	Vector angles;
	Vector src = pLocal->GetOrigin() + pLocal->GetViewOffset();

	CalcAngle(src, point, angles);
	GameUtils::NormaliseViewAngle(angles);

	if (angles[0] != angles[0] || angles[1] != angles[1])
	{
		return ReturnValue;
	}


	IsLocked = true;

	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	if (!IsAimStepping)
		LastAimstepAngle = LastAngle;

	float fovLeft = FovToPlayer(ViewOffset, LastAimstepAngle, Interfaces::EntList->GetClientEntity(TargetID), 0);

	if (fovLeft > 25.0f && Menu::Window.RageBotTab.AimbotAimStep.GetState())
	{
		Vector AddAngs = angles - LastAimstepAngle;
		Normalize(AddAngs, AddAngs);
		AddAngs *= 25;
		LastAimstepAngle += AddAngs;
		GameUtils::NormaliseViewAngle(LastAimstepAngle);
		angles = LastAimstepAngle;
	}
	else
	{
		ReturnValue = true;
	}

	if (Menu::Window.RageBotTab.AimbotSilentAim.GetState())
	{
		pCmd->viewangles = angles;
	}

	if (!Menu::Window.RageBotTab.AimbotSilentAim.GetState())
	{
		Interfaces::Engine->SetViewAngles(angles);
	}

	return ReturnValue;
}

namespace AntiAims
{
	void DownPitch(CUserCmd* pCmd)
	{
		pCmd->viewangles.x = 89;
	}


	void Jitter(CUserCmd *pCmd)
	{
		static int jitterangle = 0;

		if (jitterangle <= 1)
		{
			pCmd->viewangles.y += 90;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			pCmd->viewangles.y -= 90;
		}

		int re = rand() % 4 + 1;


		if (jitterangle <= 1)
		{
			if (re == 4)
				pCmd->viewangles.y += 180;
			jitterangle += 1;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			if (re == 4)
				pCmd->viewangles.y -= 180;
			jitterangle += 1;
		}
		else
		{
			jitterangle = 0;
		}
	}

	void TestLBYBreaker(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool Fast = false;
		bool flip = true;
		QAngle angle_for_yaw;
		static int counter = 0;
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1;
		int oldlowerbodyyaw = 0;
		if (Fast)
		{
			static bool f_flip = true;
			f_flip = !f_flip;

			if (f_flip)
			{
				pCmd->viewangles.y -= 170.00f;
				bSendPacket = false;
			}
			else if (!f_flip)
			{
				pCmd->viewangles.y -= 190.00f;
				bSendPacket = false;
			}
		}
		else
		{


			if (flip)
			{
				if (counter % 48 == 0)
					motion++;
				int value = ServerTime % 2;
				switch (value) {

				case 0:pCmd->viewangles.y += 180;
					bSendPacket = true;
				case 1:pCmd->viewangles.y += 90;
					bSendPacket = true;
				}
			}
		}
		Fast = !Fast;
	}

	void LBYAdapt(CUserCmd *pCmd, bool &bSendPacket)
	{
		IClientEntity* pLocal = hackManager.pLocal();
		static int skeet = 179;
		int SpinSpeed = 500;
		static int ChokedPackets = -1;
		ChokedPackets++;
		skeet += SpinSpeed;

		if (pCmd->command_number % 20)
		{
			if (skeet >= pLocal->GetLowerBodyYaw() + 179 - rand() % 30);
			skeet = pLocal->GetLowerBodyYaw() - 0;
			ChokedPackets = -1;
		}
		if (pCmd->command_number % 30)
		{
			float CalculatedCurTime = (Interfaces::Globals->curtime * 1000.0);
			pCmd->viewangles.y = CalculatedCurTime;
			ChokedPackets = -1;
		}

		pCmd->viewangles.y = skeet;
	}

	void BackJitter(CUserCmd *pCmd)
	{
		int random = rand() % 100;

		if (random < 98)

			pCmd->viewangles.y -= 180;

		if (random < 15)
		{
			float change = -70 + (rand() % (int)(140 + 1));
			pCmd->viewangles.y += change;
		}
		if (random == 69)
		{
			float change = -90 + (rand() % (int)(180 + 1));
			pCmd->viewangles.y += change;
		}
	}

	void Testing(CUserCmd *pCmd, bool &bSendPacket)
	{

		int random = rand() % 100;
		int maxJitter = rand() % (85 - 70 + 1) + 70;
		hackManager.pLocal()->GetLowerBodyYaw() - (rand() % maxJitter);
		if (random < 35 + (rand() % 15))
		{
			bSendPacket = false;
			pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() - (rand() % maxJitter);;
		}

		else if (random < 85 + (rand() % 15))
		{
			bSendPacket = true;
			pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() - (rand() % maxJitter);;
		}
	}

}

void CorrectMovement(Vector old_angles, CUserCmd* cmd, float old_forwardmove, float old_sidemove)
{
	float delta_view, first_function, second_function;

	if (old_angles.y < 0.f) first_function = 360.0f + old_angles.y;
	else first_function = old_angles.y;

	if (cmd->viewangles.y < 0.0f) second_function = 360.0f + cmd->viewangles.y;
	else second_function = cmd->viewangles.y;

	if (second_function < first_function) delta_view = abs(second_function - first_function);
	else delta_view = 360.0f - abs(first_function - second_function);

	delta_view = 360.0f - delta_view;

	cmd->forwardmove = cos(DEG2RAD(delta_view)) * old_forwardmove + cos(DEG2RAD(delta_view + 90.f)) * old_sidemove;
	cmd->sidemove = sin(DEG2RAD(delta_view)) * old_forwardmove + sin(DEG2RAD(delta_view + 90.f)) * old_sidemove;
}

float GetLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float Latency = nci->GetAvgLatency(FLOW_OUTGOING) + nci->GetAvgLatency(FLOW_INCOMING);
		return Latency;
	}
	else
	{
		return 0.0f;
	}
}
float GetOutgoingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float OutgoingLatency = nci->GetAvgLatency(FLOW_OUTGOING);
		return OutgoingLatency;
	}
	else
	{
		return 0.0f;
	}
}
float GetIncomingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float IncomingLatency = nci->GetAvgLatency(FLOW_INCOMING);
		return IncomingLatency;
	}
	else
	{
		return 0.0f;
	}
}

float OldLBY;
float LBYBreakerTimer;
float LastLBYUpdateTime;
bool bSwitch;
float CurrentVelocity(IClientEntity* LocalPlayer)
{
	int vel = LocalPlayer->GetVelocity().Length2D();
	return vel;
}
bool NextLBYUpdate()
{
	IClientEntity* LocalPlayer = hackManager.pLocal();

	float flServerTime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);


	if (OldLBY != LocalPlayer->GetLowerBodyYaw())
	{
		LBYBreakerTimer++;
		OldLBY = LocalPlayer->GetLowerBodyYaw();
		bSwitch = !bSwitch;
		LastLBYUpdateTime = flServerTime;
	}

	if (CurrentVelocity(LocalPlayer) > 0.5)
	{
		LastLBYUpdateTime = flServerTime;
		return false;
	}

	if ((LastLBYUpdateTime + 1 - (GetLatency() * 2) < flServerTime) && (LocalPlayer->GetFlags() & FL_ONGROUND))
	{
		if (LastLBYUpdateTime + 1.1 - (GetLatency() * 2) < flServerTime)
		{
			LastLBYUpdateTime += 1.1;
		}
		return true;
	}
	return false;
}



void DoRealAA(CUserCmd* pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	static bool switch2;
	Vector oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;
	if (!Menu::Window.RageBotTab.AntiAimEnable.GetState())
		return;
	
}

void DoFakeAA(CUserCmd* pCmd, bool& bSendPacket, IClientEntity* pLocal)
{
	static bool switch2;
	Vector oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;
	if (!Menu::Window.RageBotTab.AntiAimEnable.GetState())
		return;
	
}

void CRageBot::DoAntiAim(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();

	if ((pCmd->buttons & IN_USE) || pLocal->GetMoveType() == MOVETYPE_LADDER)
		return;

	if (IsAimStepping || pCmd->buttons & IN_ATTACK)
		return;

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	if (pWeapon)
	{
		CSWeaponInfo* pWeaponInfo = pWeapon->GetCSWpnData();

		if (!GameUtils::IsBallisticWeapon(pWeapon))
		{
			if (Menu::Window.RageBotTab.AntiAimKnife.GetState())
			{
				if (!CanOpenFire() || pCmd->buttons & IN_ATTACK2)
					return;
			}
			else
			{
				return;
			}
		}
	}
	if (Menu::Window.RageBotTab.AntiAimTarget.GetState())
	{
		aimAtPlayer(pCmd);
	}
	switch (Menu::Window.RageBotTab.AntiAimPitch.GetIndex())
	{
	case 0:
		// No Pitch AA
		break;
	case 1:
		// Down
		AntiAims::DownPitch(pCmd);
		break;
	}

	switch (Menu::Window.RageBotTab.FakeYaw.GetIndex())
	{
	case 0:
		// No Yaw AA
		break;
	case 1:
		// Jitter
		AntiAims::Jitter(pCmd);
		break;
	case 2:
		// LBY BREAKER TEST
		AntiAims::TestLBYBreaker(pCmd, bSendPacket);
		break;
	case 3:
		// k
		AntiAims::LBYAdapt(pCmd,bSendPacket);
		break;
	case 4:
		// 180 JITTER LMAO
		AntiAims::BackJitter(pCmd);
		break;
	case 5:
		// Testing
		AntiAims::Testing(pCmd,bSendPacket);
		break;
	}
	switch (Menu::Window.RageBotTab.AntiAimYaw.GetIndex())
	{
	case 0:
		// No Yaw AA
		break;
	case 1:
		// Jitter
		AntiAims::Jitter(pCmd);
		break;
	case 2:
		// LBY BREAKER TEST
		AntiAims::TestLBYBreaker(pCmd, bSendPacket);
		break;
	case 3:
		// k
		AntiAims::LBYAdapt(pCmd, bSendPacket);
		break;
	case 4:
		// 180 JITTER LMAO
		AntiAims::BackJitter(pCmd);
		break;
	case 5:
		// Testing
		AntiAims::Testing(pCmd, bSendPacket);
		break;
	}

}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class jglvuen {
public:
	double hzclhru;
	int omveevhsw;
	int xmoaejzqaipy;
	jglvuen();
	double bsmzxtezku(string hychecozquyyndm, int dprzbixhbxpkzlb, bool cwvbpwhkynau, bool tjhomsykblbfng);
	double ctugimmkuvhgbaemzgkhx(string bdwbztljre);
	double bfjgigeobml(string tnetjajwqzba, string pepikjdkfmh, bool ymyxgprrn, double wdvfjhepv, string llfyeicgpc, bool ixgyvmmowbing, int usaktefwr);
	bool sizknxbkmghajfhk(double ytqwcinzrln);
	int jcbdquhrskjlm(bool mwkqwwl, int bjtjdvkie, string nqpnktoehzaqtc, string lffkozxsazin, double kdsgugljn, double zevqmqbe, string jmooc, int lzfqmwig, int poqtrwxtgdb);
	int firmpktnijcpwsggjnpvyhn(double kxbhur, int sududlnpgkfctm);

protected:
	double glgsvgedhcfbrpr;
	int ciqibnt;

	void pighzlvyabfoihvpbztmwwh();
	void zcscojvyuedrmmoccdbustl(double frlbjopwu, string fupwc, double knsdd, bool adhimkpow, double kwaxbfze, double lyyyb, bool jelxduv, int jiyzhgl);

private:
	int kbjraihzf;
	double sskkbm;
	double aqjxlowd;
	double irljfyjvxbo;
	string dkykvhljknrsg;

	int nhllwcgtmcpuzucqf(double amwtz, double htuurbesdpt, int cysdz, int yhhuybownwsb);
	double fnkdeubopozgoxssckjp(string oopgwhhlknzqej, double djozajl, double znmmcrsk, bool lqvpbwlgu);
	double opwquduzuikdixkdkc(string geueondjtlcdomq, string ihhoxviz, bool ehfvifxrzf, string uomvftn, bool rwikbxfgnrrpndw, double shcadkavtmzdah, bool lkhezrbfpba, int erdnz, double ufdsilkli);
	void wpfrewmrvad(bool orxmbuywnbpcv, int zljfs);
	string zxpfiyzwqrrzvbsye(int uapvkguzk, int xzlmfe, bool tnrruoshedxj, bool futbjmbvzjnre, double hnmjesjfyw);
	void cmpbljuendplenpvjlhynmw(bool gnlwiehlulr, int tpxqeegrugjv, int uuubonckof, string rmaxqfuhwsi, string sjzrdvzl, bool kjglqqwduwxv, bool dkkaraefpia);
	bool itxbuauenejatqvwebbxntnqi(bool kwmceeqcj, bool fjfiduckb, double tcytfnsmwqo, bool qhuedzzej, double ldbojrjtijyql, int wglkooj, int cozvmn);
	void wnhzrxhpenmxqnjfkizjlywh();
	double mjimnjuzazgycjjyjqblkfsot(string rvitwylrcuxo, bool qabovfzyoa, double gazolffhxbnhl, int xouesi, bool lankgdherpxaui, string xjuefolmmwlltzu, double bymlz, string zykcylknyc, int dtjieazcfe);
	bool esszsmoijt(int ncozdtijhlr, string jzlaoadofmdd, int udqnvnue, string qibjggvw, bool uzsbatcf, double gaupakty, double shterheqjl, string ngime, double wilqucsw, bool nqglblklagt);

};


int jglvuen::nhllwcgtmcpuzucqf(double amwtz, double htuurbesdpt, int cysdz, int yhhuybownwsb) {
	double symsu = 4812;
	bool kffwkcfqqbon = false;
	double sbaytu = 3293;
	int pgwxhfvwdnj = 7238;
	string kiublq = "bbxwig";
	bool kxdeay = false;
	int ukxnlc = 5531;
	if (5531 == 5531) {
		int kljbmis;
		for (kljbmis = 87; kljbmis > 0; kljbmis--) {
			continue;
		}
	}
	if (4812 != 4812) {
		int fykcyvjb;
		for (fykcyvjb = 77; fykcyvjb > 0; fykcyvjb--) {
			continue;
		}
	}
	if (7238 != 7238) {
		int mirk;
		for (mirk = 43; mirk > 0; mirk--) {
			continue;
		}
	}
	if (false != false) {
		int xtct;
		for (xtct = 73; xtct > 0; xtct--) {
			continue;
		}
	}
	return 99278;
}

double jglvuen::fnkdeubopozgoxssckjp(string oopgwhhlknzqej, double djozajl, double znmmcrsk, bool lqvpbwlgu) {
	return 76917;
}

double jglvuen::opwquduzuikdixkdkc(string geueondjtlcdomq, string ihhoxviz, bool ehfvifxrzf, string uomvftn, bool rwikbxfgnrrpndw, double shcadkavtmzdah, bool lkhezrbfpba, int erdnz, double ufdsilkli) {
	string jmquacdqcno = "ofwatdgegnbdkwtriqpcpzmtzpronrtbgrrptdxossfkqvhqpkjflfumzxswztrkuchllicupcvyitijoprdosksdwelpbuus";
	string hhhww = "vvtdmdnsmibwimxdswcpunqizkevoipprihhohflykll";
	double phnmvpnvhx = 5346;
	return 84872;
}

void jglvuen::wpfrewmrvad(bool orxmbuywnbpcv, int zljfs) {
	string qzzqah = "glvijrebnfjmoiqwmxsxxzblzigelfthxvenfoipytbltabonkchotalhruxuqnhkbuno";
	double ykzczyjiudndf = 75013;
	int xzfzfskfbetoa = 837;
	int wejftwc = 2478;
	string pwupybx = "mdwpyost";
	int bghxpwjkl = 2488;
	int xvkjkk = 4218;
	int dubpwlzojrzjeb = 4573;
	int dpqncolkeqbdas = 996;
	int diemrsrmrypbqvy = 125;
	if (125 == 125) {
		int qpxnov;
		for (qpxnov = 87; qpxnov > 0; qpxnov--) {
			continue;
		}
	}
	if (996 == 996) {
		int nxowmfr;
		for (nxowmfr = 79; nxowmfr > 0; nxowmfr--) {
			continue;
		}
	}
	if (string("mdwpyost") == string("mdwpyost")) {
		int riojdprxqn;
		for (riojdprxqn = 31; riojdprxqn > 0; riojdprxqn--) {
			continue;
		}
	}

}

string jglvuen::zxpfiyzwqrrzvbsye(int uapvkguzk, int xzlmfe, bool tnrruoshedxj, bool futbjmbvzjnre, double hnmjesjfyw) {
	string qnwxqfaupe = "sqszrglwovonwpwmywqcpdmmjkbnbjfkkyi";
	bool alrhyobvqpxux = false;
	int iexejezefvplx = 5138;
	int nbntsgyzbc = 2362;
	double sevhmvmkxr = 4044;
	double baiyjbpnvejo = 15784;
	if (false != false) {
		int jzebsqivr;
		for (jzebsqivr = 46; jzebsqivr > 0; jzebsqivr--) {
			continue;
		}
	}
	if (false != false) {
		int qldkb;
		for (qldkb = 26; qldkb > 0; qldkb--) {
			continue;
		}
	}
	if (5138 == 5138) {
		int gnrryen;
		for (gnrryen = 35; gnrryen > 0; gnrryen--) {
			continue;
		}
	}
	return string("igfiw");
}

void jglvuen::cmpbljuendplenpvjlhynmw(bool gnlwiehlulr, int tpxqeegrugjv, int uuubonckof, string rmaxqfuhwsi, string sjzrdvzl, bool kjglqqwduwxv, bool dkkaraefpia) {
	bool ceiluuvrb = true;
	double emswrgtbmdpofv = 5520;
	if (true != true) {
		int jnrliathu;
		for (jnrliathu = 71; jnrliathu > 0; jnrliathu--) {
			continue;
		}
	}
	if (5520 != 5520) {
		int mltiahausr;
		for (mltiahausr = 75; mltiahausr > 0; mltiahausr--) {
			continue;
		}
	}

}

bool jglvuen::itxbuauenejatqvwebbxntnqi(bool kwmceeqcj, bool fjfiduckb, double tcytfnsmwqo, bool qhuedzzej, double ldbojrjtijyql, int wglkooj, int cozvmn) {
	bool dlllimrxyi = true;
	string jgjudgmouslrlr = "sciwjmwdycvahithovbiyrrrpyvaldmdtxsptaurdocqga";
	bool rawacxdtqf = true;
	string acskv = "bkfgzxvscktkidouwjgwraxmutiegjujmnmajkgxaayervdvhhzeszd";
	bool esdrsfcmfgrj = false;
	double qefisq = 2371;
	double zhonfl = 23987;
	if (false == false) {
		int einlgyck;
		for (einlgyck = 8; einlgyck > 0; einlgyck--) {
			continue;
		}
	}
	if (true == true) {
		int yr;
		for (yr = 33; yr > 0; yr--) {
			continue;
		}
	}
	return true;
}

void jglvuen::wnhzrxhpenmxqnjfkizjlywh() {
	double oxdfcmlcvmr = 23956;
	bool syessy = true;
	if (true != true) {
		int hdyzthzm;
		for (hdyzthzm = 92; hdyzthzm > 0; hdyzthzm--) {
			continue;
		}
	}
	if (23956 == 23956) {
		int atmo;
		for (atmo = 93; atmo > 0; atmo--) {
			continue;
		}
	}
	if (true == true) {
		int ttykulrgi;
		for (ttykulrgi = 31; ttykulrgi > 0; ttykulrgi--) {
			continue;
		}
	}
	if (23956 == 23956) {
		int suibblxlkf;
		for (suibblxlkf = 14; suibblxlkf > 0; suibblxlkf--) {
			continue;
		}
	}
	if (true != true) {
		int mbwr;
		for (mbwr = 66; mbwr > 0; mbwr--) {
			continue;
		}
	}

}

double jglvuen::mjimnjuzazgycjjyjqblkfsot(string rvitwylrcuxo, bool qabovfzyoa, double gazolffhxbnhl, int xouesi, bool lankgdherpxaui, string xjuefolmmwlltzu, double bymlz, string zykcylknyc, int dtjieazcfe) {
	string yjicaq = "iivgscrafofimsxevvdqrqemwttcespkswfqfbwcfqxjnxxwsxwuw";
	int rbsjmzryfgrk = 4489;
	string urjejtpnpq = "rzgusohxcjtmvvsvglhynp";
	double fkqbrjplpacw = 46375;
	bool ysvgfoajzplcrg = true;
	double iodowkoocf = 8783;
	double qssaulpczk = 316;
	string nuuziehr = "hwncvzlhuwuxgcn";
	if (string("hwncvzlhuwuxgcn") != string("hwncvzlhuwuxgcn")) {
		int yipwdiz;
		for (yipwdiz = 35; yipwdiz > 0; yipwdiz--) {
			continue;
		}
	}
	if (4489 != 4489) {
		int edc;
		for (edc = 82; edc > 0; edc--) {
			continue;
		}
	}
	if (8783 == 8783) {
		int sw;
		for (sw = 59; sw > 0; sw--) {
			continue;
		}
	}
	return 53566;
}

bool jglvuen::esszsmoijt(int ncozdtijhlr, string jzlaoadofmdd, int udqnvnue, string qibjggvw, bool uzsbatcf, double gaupakty, double shterheqjl, string ngime, double wilqucsw, bool nqglblklagt) {
	return false;
}

void jglvuen::pighzlvyabfoihvpbztmwwh() {
	double nzcyfxbmwbiczo = 204;
	if (204 == 204) {
		int trt;
		for (trt = 30; trt > 0; trt--) {
			continue;
		}
	}
	if (204 != 204) {
		int txeownsbc;
		for (txeownsbc = 51; txeownsbc > 0; txeownsbc--) {
			continue;
		}
	}
	if (204 != 204) {
		int sh;
		for (sh = 83; sh > 0; sh--) {
			continue;
		}
	}
	if (204 == 204) {
		int ia;
		for (ia = 10; ia > 0; ia--) {
			continue;
		}
	}

}

void jglvuen::zcscojvyuedrmmoccdbustl(double frlbjopwu, string fupwc, double knsdd, bool adhimkpow, double kwaxbfze, double lyyyb, bool jelxduv, int jiyzhgl) {
	string qpxisb = "wqcqpruvqdrrquqtiqtmnihmxtsxwcodeadsq";
	string vswpaqmr = "hxnoytawztllenormigjypahthlrvddabuejwdlxxyyawjeawuk";
	string kiszsthgvj = "bgwaeeeoduimpslcn";
	double avienbwljrqgjw = 35054;
	double ajvzhuvhjmhlwbh = 7347;
	string aymjqqxht = "jqcdfzcvkeaogbpolheowinewhruvu";
	string fuehvg = "qts";
	string hhperlsijdod = "dbjvklfckkfhlcvimfzttvkltdjzykkeidscohpykcpmdhuqxzdmbmfpdhrkqfvfbvyfptfzwoyxaulzoil";
	int kfnbkwtme = 2952;

}

double jglvuen::bsmzxtezku(string hychecozquyyndm, int dprzbixhbxpkzlb, bool cwvbpwhkynau, bool tjhomsykblbfng) {
	string ivsavgkddu = "cgopnourmlgoukgpxmzzocljhqfoeudftol";
	string kpkgyoqdzagcgh = "njemhyryrhtq";
	if (string("njemhyryrhtq") != string("njemhyryrhtq")) {
		int ldzo;
		for (ldzo = 0; ldzo > 0; ldzo--) {
			continue;
		}
	}
	if (string("njemhyryrhtq") == string("njemhyryrhtq")) {
		int keydqrdqw;
		for (keydqrdqw = 49; keydqrdqw > 0; keydqrdqw--) {
			continue;
		}
	}
	if (string("cgopnourmlgoukgpxmzzocljhqfoeudftol") != string("cgopnourmlgoukgpxmzzocljhqfoeudftol")) {
		int mtkj;
		for (mtkj = 39; mtkj > 0; mtkj--) {
			continue;
		}
	}
	if (string("cgopnourmlgoukgpxmzzocljhqfoeudftol") == string("cgopnourmlgoukgpxmzzocljhqfoeudftol")) {
		int pu;
		for (pu = 49; pu > 0; pu--) {
			continue;
		}
	}
	if (string("cgopnourmlgoukgpxmzzocljhqfoeudftol") == string("cgopnourmlgoukgpxmzzocljhqfoeudftol")) {
		int kmqslapq;
		for (kmqslapq = 89; kmqslapq > 0; kmqslapq--) {
			continue;
		}
	}
	return 63086;
}

double jglvuen::ctugimmkuvhgbaemzgkhx(string bdwbztljre) {
	return 7656;
}

double jglvuen::bfjgigeobml(string tnetjajwqzba, string pepikjdkfmh, bool ymyxgprrn, double wdvfjhepv, string llfyeicgpc, bool ixgyvmmowbing, int usaktefwr) {
	string rzicecuwg = "godhabhupzwblynsgkoppxnsjbymycrowsvwoiiabxgfqqwrimouateludrymesnozpnuojfd";
	string uhkock = "lndjhhklaknlqdptjqlivyhzfyrattihvpuwovtyyjcohm";
	double rgybmrzidbqc = 9024;
	string twnzxlkawklg = "wciulctejhjkqrlkvulxnsvdwa";
	string ohlwvdat = "pphgzvdmldqpmjtngimtbvqbavczrradhpezfjhzcdeycxqyegowoy";
	string bhflzpppysthtz = "sfhuspavslyvtdbprqqzsvruqxstmcedrghwprifvyyqdjujuaipegozmmvahejwfjuh";
	if (9024 != 9024) {
		int hqoo;
		for (hqoo = 69; hqoo > 0; hqoo--) {
			continue;
		}
	}
	if (string("wciulctejhjkqrlkvulxnsvdwa") == string("wciulctejhjkqrlkvulxnsvdwa")) {
		int hxts;
		for (hxts = 65; hxts > 0; hxts--) {
			continue;
		}
	}
	return 62160;
}

bool jglvuen::sizknxbkmghajfhk(double ytqwcinzrln) {
	bool fcwowavldxrfonm = false;
	double wvqeywcbeirmpzp = 41839;
	int wmxfyau = 308;
	string tcnoajutioslv = "upixidivmvbnwdmboduefvmedziskyjugmxpaezmho";
	if (308 != 308) {
		int jrhtyhgvc;
		for (jrhtyhgvc = 50; jrhtyhgvc > 0; jrhtyhgvc--) {
			continue;
		}
	}
	if (41839 != 41839) {
		int srwflyon;
		for (srwflyon = 43; srwflyon > 0; srwflyon--) {
			continue;
		}
	}
	if (41839 != 41839) {
		int agmd;
		for (agmd = 35; agmd > 0; agmd--) {
			continue;
		}
	}
	if (308 == 308) {
		int yr;
		for (yr = 11; yr > 0; yr--) {
			continue;
		}
	}
	if (308 == 308) {
		int ofjbr;
		for (ofjbr = 80; ofjbr > 0; ofjbr--) {
			continue;
		}
	}
	return false;
}

int jglvuen::jcbdquhrskjlm(bool mwkqwwl, int bjtjdvkie, string nqpnktoehzaqtc, string lffkozxsazin, double kdsgugljn, double zevqmqbe, string jmooc, int lzfqmwig, int poqtrwxtgdb) {
	bool zknrhailc = true;
	string gwzbkhkjpcr = "qcceryfdcrzezwjquxlpowmyeocxomhsfslmlcpujspaktlgswgmkxulkuocoohszcqujycpwps";
	int higxk = 7207;
	int ovnarlpsn = 2453;
	if (true == true) {
		int gjvbu;
		for (gjvbu = 32; gjvbu > 0; gjvbu--) {
			continue;
		}
	}
	if (string("qcceryfdcrzezwjquxlpowmyeocxomhsfslmlcpujspaktlgswgmkxulkuocoohszcqujycpwps") == string("qcceryfdcrzezwjquxlpowmyeocxomhsfslmlcpujspaktlgswgmkxulkuocoohszcqujycpwps")) {
		int pfl;
		for (pfl = 33; pfl > 0; pfl--) {
			continue;
		}
	}
	if (string("qcceryfdcrzezwjquxlpowmyeocxomhsfslmlcpujspaktlgswgmkxulkuocoohszcqujycpwps") != string("qcceryfdcrzezwjquxlpowmyeocxomhsfslmlcpujspaktlgswgmkxulkuocoohszcqujycpwps")) {
		int hozh;
		for (hozh = 9; hozh > 0; hozh--) {
			continue;
		}
	}
	if (string("qcceryfdcrzezwjquxlpowmyeocxomhsfslmlcpujspaktlgswgmkxulkuocoohszcqujycpwps") != string("qcceryfdcrzezwjquxlpowmyeocxomhsfslmlcpujspaktlgswgmkxulkuocoohszcqujycpwps")) {
		int jywx;
		for (jywx = 62; jywx > 0; jywx--) {
			continue;
		}
	}
	return 3249;
}

int jglvuen::firmpktnijcpwsggjnpvyhn(double kxbhur, int sududlnpgkfctm) {
	int tihepiscu = 1454;
	int sjoidgnauopbym = 1273;
	int upatmtyvpmo = 5660;
	int bbgsr = 6833;
	bool icqyhpz = false;
	bool patvddmuzndnm = true;
	bool nwuzco = true;
	return 68331;
}

jglvuen::jglvuen() {
	this->bsmzxtezku(string("acgepgufkeyxohjsgkrfmiwupiukqjfbiqbgyxvuvmnkxxtuyrwshojkbwwoegjxdxmmphsnisc"), 3701, false, false);
	this->ctugimmkuvhgbaemzgkhx(string("iljebgpenvrbbnqlcltfdjmk"));
	this->bfjgigeobml(string("ppyvzvcgxoolhdjtqxxhre"), string("gvswmveqyyrkzruoklssnulqesikdmffzlguiwzehlzoafteouvv"), true, 27276, string("zjtftqhevvdmqazyexmgbcmsfzgdtxjvvrahqzzhswd"), true, 657);
	this->sizknxbkmghajfhk(40927);
	this->jcbdquhrskjlm(true, 1372, string("sbokdqs"), string("cgqcxfqczucxihprknzqbsubbvdclmyccsqlaahtmvyylymk"), 14627, 5972, string("lezyzgbkgifdfhlzhkiyrmmvhyrvbuwsvaorqzrxvhidcbjcnhxiymrrd"), 2536, 1576);
	this->firmpktnijcpwsggjnpvyhn(51770, 6539);
	this->pighzlvyabfoihvpbztmwwh();
	this->zcscojvyuedrmmoccdbustl(3188, string("vtzwhnmadhwcumbuzaloienuwwivuomyxoufyhcvxjkn"), 37997, false, 19181, 3913, true, 1868);
	this->nhllwcgtmcpuzucqf(9292, 18980, 251, 9045);
	this->fnkdeubopozgoxssckjp(string("svzgtedcdgocatuhkoogmyrnucijysjhaikxxocklkrcstlfbjbcpmnxqnveihufkrrsreldkcknsuzulgjpferkj"), 29073, 31492, false);
	this->opwquduzuikdixkdkc(string("zxrjepormsfrijvfewskywmdnsodetajqkcdnftiaddovtduodakfgcpejqciocinxdabmkfgqgocwvabpeskmx"), string("fleyxiqmtpnxerrwsb"), false, string("qqxiebbqbijwyxnunrkspgndirudhambgnpsckgdssziombnpexwfipmorc"), false, 82607, false, 1212, 57881);
	this->wpfrewmrvad(true, 226);
	this->zxpfiyzwqrrzvbsye(4099, 1506, false, true, 14646);
	this->cmpbljuendplenpvjlhynmw(true, 661, 1402, string("uawutflssgvnqf"), string("svqoqcqbinzkajwljivkmfzvofnjanzfrjgnmdrtmjsa"), false, false);
	this->itxbuauenejatqvwebbxntnqi(false, false, 27507, true, 28461, 3932, 1905);
	this->wnhzrxhpenmxqnjfkizjlywh();
	this->mjimnjuzazgycjjyjqblkfsot(string("gkvqahognosfsregjioiuddflygetqbdtpcfucaopqcvetvhukwixtswuyypbjnhyfpbtqafly"), true, 5287, 2723, true, string("elgvdcksraozbnpkyfzejoxmaylz"), 3563, string("zwjqkldo"), 4215);
	this->esszsmoijt(7156, string("bdogzgybyitzpwpaeyx"), 2853, string("zdulrqzgyihdazmovptddmtrfzrrxzygavcyfjwlxmxlyer"), false, 31293, 5437, string("tcucapiihulbcqfwv"), 12639, false);
}


