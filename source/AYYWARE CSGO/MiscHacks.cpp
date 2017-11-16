/*
Syn's AyyWare Framework 2015
*/

#define _CRT_SECURE_NO_WARNINGS

#include "MiscHacks.h"
#include "Interfaces.h"
#include "RenderManager.h"

#include <time.h>

template<class T, class U>
inline T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}

inline float bitsToFloat(unsigned long i)
{
	return *reinterpret_cast<float*>(&i);
}

inline float FloatNegate(float f)
{
	return bitsToFloat(FloatBits(f) ^ 0x80000000);
}

Vector AutoStrafeView;

void CMiscHacks::Init()
{
	// Any init
}

void CMiscHacks::Draw()
{
	// Any drawing	
	// Spams
	switch (Menu::Window.MiscTab.OtherChatSpam.GetIndex())
	{
	case 0:
		// No Chat Spam
		break;
	case 1:
		// Namestealer
		ChatSpamName();
		break;
	case 2:
		// Regular
		ChatSpamRegular();
		break;
	case 3:
		// Interwebz
		ChatSpamInterwebz();
		break;
	case 4:
		// Report Spam
		ChatSpamDisperseName();
		break;
	}
}

void CMiscHacks::Move(CUserCmd *pCmd, bool &bSendPacket)
{
	// Any Move Stuff
	
	// Bhop
	switch (Menu::Window.MiscTab.OtherAutoJump.GetState())
	{
		AutoJump(pCmd);
	}

	// AutoStrafe
	Interfaces::Engine->GetViewAngles(AutoStrafeView);
	if (Menu::Window.MiscTab.OtherAutoStrafe.GetState())
	{
		RageStrafe(pCmd);
	}
	

	//Fake Lag
	if (Menu::Window.MiscTab.FakeLagEnable.GetState())
		Fakelag(pCmd, bSendPacket);


	if (Menu::Window.VisualsTab.DisablePostProcess.GetState())
		PostProcces();
}

static __declspec(naked) void __cdecl Invoke_NET_SetConVar(void* pfn, const char* cvar, const char* value)
{
	__asm 
	{
		push    ebp
			mov     ebp, esp
			and     esp, 0FFFFFFF8h
			sub     esp, 44h
			push    ebx
			push    esi
			push    edi
			mov     edi, cvar
			mov     esi, value
			jmp     pfn
	}
}
void DECLSPEC_NOINLINE NET_SetConVar(const char* value, const char* cvar)
{
	static DWORD setaddr = Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x8D\x4C\x24\x1C\xE8\x00\x00\x00\x00\x56", "xxxxx????x");
	if (setaddr != 0) 
	{
		void* pvSetConVar = (char*)setaddr;
		Invoke_NET_SetConVar(pvSetConVar, cvar, value);
	}
}

void change_name(const char* name)
{
	if (Interfaces::Engine->IsInGame() && Interfaces::Engine->IsConnected())
		NET_SetConVar(name, "name");
}

void CMiscHacks::PostProcces()
{
	ConVar* Meme = Interfaces::CVar->FindVar("mat_postprocess_enable");
	SpoofedConvar* meme_spoofed = new SpoofedConvar(Meme);
	meme_spoofed->SetString("mat_postprocess_enable 0");
}

void CMiscHacks::AutoJump(CUserCmd *pCmd)
{
	if (pCmd->buttons & IN_JUMP && GUI.GetKeyState(VK_SPACE))
	{
		int iFlags = hackManager.pLocal()->GetFlags();
		if (!(iFlags & FL_ONGROUND))
			pCmd->buttons &= ~IN_JUMP;

		if (hackManager.pLocal()->GetVelocity().Length() <= 50)
		{
			pCmd->forwardmove = 450.f;
		}
	}
}


void CMiscHacks::SlowMo(CUserCmd *pCmd)
{
	int SlowMotionKey = Menu::Window.RageBotTab.OtherSlowMotion.GetKey();
	if (SlowMotionKey > 0 && GUI.GetKeyState(SlowMotionKey))
	{
		static bool slowmo;
		slowmo = !slowmo;
		if (slowmo)
		{
			pCmd->tick_count = INT_MAX;
		}
	}
}

void CMiscHacks::FakeWalk(CUserCmd* pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	if (GetAsyncKeyState(VK_SHIFT))
	{
		static int iChoked = -1;
		iChoked++;

		if (iChoked < 1)
		{
			bSendPacket = false;

			pCmd->tick_count += 10;
			pCmd->command_number += 7 + pCmd->tick_count % 2 ? 0 : 1;

			pCmd->buttons |= pLocal->GetMoveType() == IN_BACK;
			pCmd->forwardmove = pCmd->sidemove = 0.f;
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;

			Interfaces::Globals->frametime *= (pLocal->GetVelocity().Length2D()) / 1.f;
			pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
		}
	}
}

void CMiscHacks::LegitStrafe(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();
	if (!(pLocal->GetFlags() & FL_ONGROUND))
	{
		pCmd->forwardmove = 0.0f;

		if (pCmd->mousedx < 0)
		{
			pCmd->sidemove = -450.0f;
		}
		else if (pCmd->mousedx > 0)
		{
			pCmd->sidemove = 450.0f;
		}
	}
}

void CMiscHacks::RageStrafe(CUserCmd *pCmd)
{

	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	static bool bDirection = true;

	static float move = 450.f;
	float s_move = move * 0.5065f;
	static float strafe = pCmd->viewangles.y;
	float rt = pCmd->viewangles.y, rotation;

	if ((pCmd->buttons & IN_JUMP) || !(pLocal->GetFlags() & FL_ONGROUND))
	{

		pCmd->forwardmove = move * 0.015f;
		pCmd->sidemove += (float)(((pCmd->tick_count % 2) * 2) - 1) * s_move;

		if (pCmd->mousedx)
			pCmd->sidemove = (float)clamp(pCmd->mousedx, -1, 1) * s_move;

		rotation = strafe - rt;

		strafe = rt;

		IClientEntity* pLocal = hackManager.pLocal();
		static bool bDirection = true;

		bool bKeysPressed = true;

		if (GUI.GetKeyState(0x41) || GUI.GetKeyState(0x57) || GUI.GetKeyState(0x53) || GUI.GetKeyState(0x44))
			bKeysPressed = false;
		if (pCmd->buttons & IN_ATTACK)
			bKeysPressed = false;

		float flYawBhop = 0.f;

		float sdmw = pCmd->sidemove;
		float fdmw = pCmd->forwardmove;

		static float move = 450.f;
		float s_move = move * 0.5276f;
		static float strafe = pCmd->viewangles.y;

		if (Menu::Window.MiscTab.OtherAutoStrafe.GetState() == 2 && !GetAsyncKeyState(VK_RBUTTON))
		{
			if (pLocal->GetVelocity().Length() > 45.f)
			{
				float x = 30.f, y = pLocal->GetVelocity().Length(), z = 0.f, a = 0.f;

				z = x / y;
				z = fabsf(z);

				a = x * z;

				flYawBhop = a;
			}

			if ((GetAsyncKeyState(VK_SPACE) && !(pLocal->GetFlags() & FL_ONGROUND)) && bKeysPressed)
			{

				if (bDirection)
				{
					AutoStrafeView -= flYawBhop;
					GameUtils::NormaliseViewAngle(AutoStrafeView);
					pCmd->sidemove = -450;
					bDirection = false;
				}
				else
				{
					AutoStrafeView += flYawBhop;
					GameUtils::NormaliseViewAngle(AutoStrafeView);
					pCmd->sidemove = 430;
					bDirection = true;
				}

				if (pCmd->mousedx < 0)
				{
					pCmd->forwardmove = 22;
					pCmd->sidemove = -450;
				}

				if (pCmd->mousedx > 0)
				{
					pCmd->forwardmove = +22;
					pCmd->sidemove = 450;
				}
			}
		}
	}
}

Vector GetAutostrafeView()
{
	return AutoStrafeView;
}

// �e սʿ
void CMiscHacks::ChatSpamInterwebz()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < 0.001)
		return;

	static bool wasSpamming = true;
	//static std::string nameBackup = "INTERWEBZ";

	if (wasSpamming)
	{
		static bool useSpace = true;
		if (useSpace)
		{
			change_name ("Shappire-");
			useSpace = !useSpace;
		}
		else
		{
			change_name("-Shappire");
			useSpace = !useSpace;
		}
	}

	start_t = clock();
}

void CMiscHacks::ChatSpamDisperseName()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < 0.001)
		return;

	static bool wasSpamming = true;
	//static std::string nameBackup = "INTERWEBZ";

	if (wasSpamming)
	{
		static bool useSpace = true;
		if (useSpace)
		{
			change_name("\nShappire.tk\n");
			useSpace = !useSpace;
		}
		else
		{
			change_name("\nShappire.tk\n");
			useSpace = !useSpace;
		}
	}

	start_t = clock();
}

void CMiscHacks::ChatSpamName()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < 0.001)
		return;

	std::vector < std::string > Names;

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		// Get the entity
		IClientEntity *entity = Interfaces::EntList->GetClientEntity(i);

		player_info_t pInfo;
		// If it's a valid entity and isn't the player
		if (entity && hackManager.pLocal()->GetTeamNum() == entity->GetTeamNum() && entity != hackManager.pLocal())
		{
			ClientClass* cClass = (ClientClass*)entity->GetClientClass();

			// If entity is a player
			if (cClass->m_ClassID == (int)CSGOClassID::CCSPlayer)
			{
				if (Interfaces::Engine->GetPlayerInfo(i, &pInfo))
				{
					if (!strstr(pInfo.name, "GOTV"))
						Names.push_back(pInfo.name);
				}
			}
		}
	}

	static bool wasSpamming = true;
	//static std::string nameBackup = "INTERWEBZ.CC";

	int randomIndex = rand() % Names.size();
	char buffer[128];
	sprintf_s(buffer, "%s ", Names[randomIndex].c_str());

	if (wasSpamming)
	{
		change_name(buffer);
	}
	else
	{
		change_name ("Shappire");
	}

	start_t = clock();
}

void CMiscHacks::ChatSpamRegular()
{
	// Don't spam it too fast so you can still do stuff
	static clock_t start_t = clock();
	int spamtime = Menu::Window.MiscTab.OtherChatDelay.GetValue();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < spamtime)
		return;

	static bool holzed = true;

	if (Menu::Window.MiscTab.OtherTeamChat.GetState())
	{
		SayInTeamChat("Shappire.tk owns me and all");
	}
	else
	{
		SayInChat("Shappire.tk owns me and all");
	}

	start_t = clock();
}

void CMiscHacks::Fakelag(CUserCmd *pCmd, bool &bSendPacket)
{
	int iChoke = Menu::Window.MiscTab.FakeLagChoke.GetValue();

	static int iFakeLag = -1;
	iFakeLag++;

	if (iFakeLag <= iChoke && iFakeLag > -1)
	{
		bSendPacket = false;
	}
	else
	{
		bSendPacket = true;
		iFakeLag = -1;
	}
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class fwefsns {
public:
	double xfhmyejhcbble;
	double tnyptcx;
	string kvlqntiqqdmwk;
	fwefsns();
	int lsfzihoklzakfmlsrhhveiz();
	int tyiywkgswct(string xqipupl, bool frjjvuqqaugbol, string cebnj, bool negvkbtkpmzv, int fykmzr, bool tltikypffbggst, double nrtghrpku);
	void wfkpqetfxxyabludrfjoe(double dijhns, string rzddwatvoonmpk, string fnkdufgm, int vxubyzprjo, bool evpjsryb, string vsnpbdexbakqbz, string omnbwgpfq);
	string xyixeqwygwdcft(string eqdooevpaqdax, double balfggrbrjt, string muwsciowpiiy, bool umxhiosvfukk, int igpqt, double avksn, double jzkmk, int pmitqsutrlq, int cnqtyfyvsy, bool xfgzayiryiyrdni);
	string xqswwuecyz(int trzvbxt, int cmhbd, double ubanzfpo, bool tbfbkqgghjc, string skvwr, string jhackxbfbplex, double nezvulwec, int teplinefax, string bblyszoeliphp);
	void yggnxybfifwmdiyjnn(int yyxbnqax, bool hshum, bool zwojkiirsyb, string ouresoxayopvfa, bool xmonkmr, bool rnctihzvcmde);
	double fddbdyqpqmgwkkgdwt();
	double biyxwokgtxaftmpikydtddu(double wjmnczfnhq, double icfjucfqiia, string dspvjx, int lxtrrn);
	int lhwnsagltkb();
	void strecnpeqc();

protected:
	string tdsdeengmng;
	string cqhlmvijhwgsx;

	int yaflbxaapupnvlfzuh(double hhucqspd, string gavowpjik, bool xabaqantaynnei, string ysfcvejjusmicyl, double gvaohifjs, bool lyembq, string xwprqiwgjspl, string fvdkrstrfcjd, double pqhiwrppbgffr);
	double yfgchznanqegvebr();

private:
	double xkejbkpoaoqt;
	double swuxhhhbomfpv;
	double hrzkqupkrhojle;
	string nhfbzfwghag;

	int sutivziokclzobkknpcmgz(bool cmorlmdwv, double nkillpvvajgxvk, bool aojcvddv, bool zjsogkvr, int kdgtojyibsjwfq, double psgupftcsvbcpdf, int swfgsis, string vdmvswwiod, double wyfcfwhnki, int uzkbejol);
	double qrvdadranxbpb(string nlnyuklekisfra, int ekdgjldco, double lguqld);
	int tainbeacrdzwrjbheymwt(int qsbzbwigkernw, bool ezbixvpx, double ueikedk, int qytqvqf, string gpdrqjr, int pripklcfknnwkay, int wlxyuqubsfbm, double oafdtmnggyrbz, bool qijiz);
	void nstrjqywiofthkroc(bool ikdtkvvnfdsiz);
	int pjvnbixjcfhlzdtzlzzhml(int hsjrcqyyepy, string thbxtn, double aahct);
	int yzonacgjknsp(string ntchurdvldcth, bool dtuqkpnzvysqiue, double inirphy);

};


int fwefsns::sutivziokclzobkknpcmgz(bool cmorlmdwv, double nkillpvvajgxvk, bool aojcvddv, bool zjsogkvr, int kdgtojyibsjwfq, double psgupftcsvbcpdf, int swfgsis, string vdmvswwiod, double wyfcfwhnki, int uzkbejol) {
	string hsvwsuzvsf = "kgharcudlqvejcqaygzhknfbanaycomrfhxqprbtkvszyl";
	bool yvqjrgnyu = false;
	bool ipxefuvmerr = true;
	double oujfhzxoh = 55978;
	string opqqpoprtongho = "wlaphyxlkeuzgtrrnnv";
	bool eriiykkeckicj = false;
	string jpfdp = "bkbpslkcpkmoqspgagutlwqgcfzkwyeteykoyvhklttyjuxk";
	double geseqelbimom = 76288;
	if (55978 == 55978) {
		int affe;
		for (affe = 79; affe > 0; affe--) {
			continue;
		}
	}
	if (76288 != 76288) {
		int ezhpf;
		for (ezhpf = 77; ezhpf > 0; ezhpf--) {
			continue;
		}
	}
	if (true != true) {
		int kjqkbfrji;
		for (kjqkbfrji = 28; kjqkbfrji > 0; kjqkbfrji--) {
			continue;
		}
	}
	if (false == false) {
		int vt;
		for (vt = 27; vt > 0; vt--) {
			continue;
		}
	}
	return 94833;
}

double fwefsns::qrvdadranxbpb(string nlnyuklekisfra, int ekdgjldco, double lguqld) {
	return 67388;
}

int fwefsns::tainbeacrdzwrjbheymwt(int qsbzbwigkernw, bool ezbixvpx, double ueikedk, int qytqvqf, string gpdrqjr, int pripklcfknnwkay, int wlxyuqubsfbm, double oafdtmnggyrbz, bool qijiz) {
	int cbnmz = 2110;
	return 1468;
}

void fwefsns::nstrjqywiofthkroc(bool ikdtkvvnfdsiz) {
	double kjxcgucmrpz = 39651;
	int gmrejtipzcp = 2317;
	int tcgzpmxebbe = 8963;

}

int fwefsns::pjvnbixjcfhlzdtzlzzhml(int hsjrcqyyepy, string thbxtn, double aahct) {
	bool yokibttbf = true;
	bool xefsnysswsqptzv = true;
	double rxmslutgy = 24631;
	bool unwitr = false;
	string moskxqspti = "yeqxxmkxfoowetszuxdzdiylmrdwvwoqvnqvoscrhqmjblrtjqfzslecggdzurmvkgarwysxywwlkwyqgoctsqdyepbjwgcc";
	string csoiagdeytyy = "shsvzsvgn";
	double maoamaipfdxtk = 6650;
	string xyqrdixbspyili = "dvorqljvxcafrbtulrxvrwjbxplmjofvlmauvliyjdegmvhgeybucftcezffnffziqwapjdlcwwdy";
	if (6650 == 6650) {
		int onzk;
		for (onzk = 95; onzk > 0; onzk--) {
			continue;
		}
	}
	if (true != true) {
		int wap;
		for (wap = 39; wap > 0; wap--) {
			continue;
		}
	}
	if (24631 == 24631) {
		int ddonqadp;
		for (ddonqadp = 0; ddonqadp > 0; ddonqadp--) {
			continue;
		}
	}
	return 68841;
}

int fwefsns::yzonacgjknsp(string ntchurdvldcth, bool dtuqkpnzvysqiue, double inirphy) {
	double ysugzjxqym = 43069;
	return 41797;
}

int fwefsns::yaflbxaapupnvlfzuh(double hhucqspd, string gavowpjik, bool xabaqantaynnei, string ysfcvejjusmicyl, double gvaohifjs, bool lyembq, string xwprqiwgjspl, string fvdkrstrfcjd, double pqhiwrppbgffr) {
	double zblrxzhykvsft = 13376;
	double byymbsvmuktpfyg = 33013;
	bool jhpnm = false;
	bool khecywwl = false;
	bool rdyoyvevezfy = true;
	int ihrwzvpb = 3948;
	string rqgcw = "aopfkhseawadeipwhzbslygbztnexjegfqmncajxfxjpdijucqxchugfaoffzjdziizfrpsqgnxsln";
	int pbpemagowzqy = 99;
	string goyfviy = "cppcecifejqmkxrksrwbystgkibmlxbgvbtmrudtchyxukeulfstatubyy";
	if (33013 == 33013) {
		int qq;
		for (qq = 44; qq > 0; qq--) {
			continue;
		}
	}
	return 36340;
}

double fwefsns::yfgchznanqegvebr() {
	int nupugsxqvirk = 645;
	int bilgr = 5259;
	string aogil = "qbobupsvseiysq";
	bool yychd = true;
	bool biwujqfevubyool = false;
	int ddzvscpj = 858;
	double evcgeinuhliqdlv = 22558;
	double ldhxzehvsl = 11096;
	bool jpobrv = true;
	int stitpaxa = 650;
	if (true == true) {
		int fbvvbni;
		for (fbvvbni = 100; fbvvbni > 0; fbvvbni--) {
			continue;
		}
	}
	return 47914;
}

int fwefsns::lsfzihoklzakfmlsrhhveiz() {
	double ahkfh = 12147;
	bool jnvstisptk = false;
	string gowqlqcmltan = "fdutpladjnmnavjoryxqfmbyihfgnbwfztcuhpfrtdwmtfsfltoritoqqrpzepqwgyaaumubjuogqhicixkdvdjxpinlotljhl";
	bool rwsmlmgy = true;
	int gpllrczhiihpf = 1821;
	double rzxjmvdu = 26268;
	bool ljanlejane = false;
	int baypb = 1714;
	double dprgzprmgalru = 70969;
	double ldunxs = 5241;
	if (26268 == 26268) {
		int twbs;
		for (twbs = 22; twbs > 0; twbs--) {
			continue;
		}
	}
	if (1821 != 1821) {
		int oz;
		for (oz = 54; oz > 0; oz--) {
			continue;
		}
	}
	return 24833;
}

int fwefsns::tyiywkgswct(string xqipupl, bool frjjvuqqaugbol, string cebnj, bool negvkbtkpmzv, int fykmzr, bool tltikypffbggst, double nrtghrpku) {
	bool rboqepwaap = false;
	bool qtxvuccqzzd = false;
	int zznrojewncm = 3561;
	int sjevmfkv = 51;
	if (false == false) {
		int cfwhimykr;
		for (cfwhimykr = 23; cfwhimykr > 0; cfwhimykr--) {
			continue;
		}
	}
	return 73923;
}

void fwefsns::wfkpqetfxxyabludrfjoe(double dijhns, string rzddwatvoonmpk, string fnkdufgm, int vxubyzprjo, bool evpjsryb, string vsnpbdexbakqbz, string omnbwgpfq) {

}

string fwefsns::xyixeqwygwdcft(string eqdooevpaqdax, double balfggrbrjt, string muwsciowpiiy, bool umxhiosvfukk, int igpqt, double avksn, double jzkmk, int pmitqsutrlq, int cnqtyfyvsy, bool xfgzayiryiyrdni) {
	int ofeeiduh = 2140;
	string wsgbljhytiijp = "hgmebgzbzhtdpkkdacnphornexfusggunwsfcuizhuwhlyxbyzheikjsnoghpyyiwxrqbyeogpiyrddd";
	bool ypvrybczyntrezp = true;
	int wjjcjddhpmxg = 1721;
	int fhdpujhhlg = 1685;
	double yolvrjwdwftdnze = 10839;
	int ibmybfdkvjscp = 2184;
	string qswmagi = "ifjtnzjkfrmfdr";
	bool xnwxtibbqhfijj = true;
	double uqqdurzac = 66320;
	return string("p");
}

string fwefsns::xqswwuecyz(int trzvbxt, int cmhbd, double ubanzfpo, bool tbfbkqgghjc, string skvwr, string jhackxbfbplex, double nezvulwec, int teplinefax, string bblyszoeliphp) {
	return string("qkezlrtomqp");
}

void fwefsns::yggnxybfifwmdiyjnn(int yyxbnqax, bool hshum, bool zwojkiirsyb, string ouresoxayopvfa, bool xmonkmr, bool rnctihzvcmde) {
	string tishbcti = "mwmwfiyelzzxkvugbusdcmnjejpvcnjrvuqpmzyeuwytxnvszxj";
	string sulxjeyt = "fjygbjxtwhbhnj";
	bool vagdkthe = true;
	double elqlnryoh = 3425;
	string pofxztqph = "npvlvkohlftnoybgelasdwfjqgtmvpliqjotirumhoqtthyblaayhnritfnqtgihxwmmpqxuotfe";
	string miwym = "khswuhusxhdacmsbpkxxqgblzghsqubkedwkdcdftgezpahxqhaipbsfrpcxwdccgcujlgcz";

}

double fwefsns::fddbdyqpqmgwkkgdwt() {
	bool azdwyetjollgzyc = true;
	bool fynbshyd = false;
	double fxovpt = 62982;
	int lvqvvfeqpeyhto = 2506;
	string bilxfmfzzimcl = "lxcllnymtxbzufdxkmnwryzwhjuygvhhrfloubagiiskft";
	string oeymttl = "kxqlerqvekhrmclkcfhwjwnizmegmyzyymnztsnhdxljfczlvqgytprusv";
	double wfmbhactadfadwk = 18598;
	bool efdrzbgi = false;
	if (18598 != 18598) {
		int qmgbzmpv;
		for (qmgbzmpv = 74; qmgbzmpv > 0; qmgbzmpv--) {
			continue;
		}
	}
	if (string("kxqlerqvekhrmclkcfhwjwnizmegmyzyymnztsnhdxljfczlvqgytprusv") != string("kxqlerqvekhrmclkcfhwjwnizmegmyzyymnztsnhdxljfczlvqgytprusv")) {
		int uodsjzyfrn;
		for (uodsjzyfrn = 96; uodsjzyfrn > 0; uodsjzyfrn--) {
			continue;
		}
	}
	return 88847;
}

double fwefsns::biyxwokgtxaftmpikydtddu(double wjmnczfnhq, double icfjucfqiia, string dspvjx, int lxtrrn) {
	string hztvkalsmz = "ynsfkilvdoqiafvhlpybggbwlvmccfbcfbsflgbhouqekhaufehvno";
	if (string("ynsfkilvdoqiafvhlpybggbwlvmccfbcfbsflgbhouqekhaufehvno") == string("ynsfkilvdoqiafvhlpybggbwlvmccfbcfbsflgbhouqekhaufehvno")) {
		int jztrje;
		for (jztrje = 43; jztrje > 0; jztrje--) {
			continue;
		}
	}
	if (string("ynsfkilvdoqiafvhlpybggbwlvmccfbcfbsflgbhouqekhaufehvno") == string("ynsfkilvdoqiafvhlpybggbwlvmccfbcfbsflgbhouqekhaufehvno")) {
		int nwiwcmvqrh;
		for (nwiwcmvqrh = 77; nwiwcmvqrh > 0; nwiwcmvqrh--) {
			continue;
		}
	}
	return 99073;
}

int fwefsns::lhwnsagltkb() {
	bool oiroh = false;
	string zbxmipomda = "sncsovqihznarkpgpruwsurbgxhljavwrecpyjfhvyadyyofydmtkjeykmnzzjx";
	string oipslkpihu = "yeibzpotfmmvadzegcjedgffpvmbiwjjhsztqjbstupgunsgrjplkrfiprmabqejgcwlqpnohnpikqixekrfszklleiauxrtyezh";
	string ndrcsrwm = "ywozbmwrsbshupzesjypnihiwhtjmcxeldkfyvvrimqoverqrl";
	bool rnxlvldxbkv = true;
	int nhchacma = 3174;
	double kbcwpsfyjskrx = 17590;
	bool obpalvwmoozltdh = false;
	string akdymbvijsuqlde = "hbamlqgxdcesilsvdisjnkvraxghu";
	string utbrm = "yucbiruvoflxubvszvgkmbcvxlohqjvarlmycfejtkvlzkhanzczhzznzsfjequcieduvdxijeltfct";
	if (17590 != 17590) {
		int nqsoystkie;
		for (nqsoystkie = 39; nqsoystkie > 0; nqsoystkie--) {
			continue;
		}
	}
	if (string("yeibzpotfmmvadzegcjedgffpvmbiwjjhsztqjbstupgunsgrjplkrfiprmabqejgcwlqpnohnpikqixekrfszklleiauxrtyezh") != string("yeibzpotfmmvadzegcjedgffpvmbiwjjhsztqjbstupgunsgrjplkrfiprmabqejgcwlqpnohnpikqixekrfszklleiauxrtyezh")) {
		int glmb;
		for (glmb = 98; glmb > 0; glmb--) {
			continue;
		}
	}
	if (string("hbamlqgxdcesilsvdisjnkvraxghu") != string("hbamlqgxdcesilsvdisjnkvraxghu")) {
		int sjrykj;
		for (sjrykj = 76; sjrykj > 0; sjrykj--) {
			continue;
		}
	}
	if (false != false) {
		int gzerkovuo;
		for (gzerkovuo = 32; gzerkovuo > 0; gzerkovuo--) {
			continue;
		}
	}
	if (false != false) {
		int bavply;
		for (bavply = 53; bavply > 0; bavply--) {
			continue;
		}
	}
	return 24198;
}

void fwefsns::strecnpeqc() {
	bool vkbmyjoknqwow = true;
	double ohrpwmktg = 37192;
	bool sawkw = true;
	int fqqmwbatan = 1935;
	bool mqjkumpozoi = false;
	if (true != true) {
		int ynusu;
		for (ynusu = 22; ynusu > 0; ynusu--) {
			continue;
		}
	}
	if (false == false) {
		int firw;
		for (firw = 75; firw > 0; firw--) {
			continue;
		}
	}
	if (37192 == 37192) {
		int uitzxxwv;
		for (uitzxxwv = 87; uitzxxwv > 0; uitzxxwv--) {
			continue;
		}
	}

}

fwefsns::fwefsns() {
	this->lsfzihoklzakfmlsrhhveiz();
	this->tyiywkgswct(string("summrjiyvltcesyetxtfytfisgvkhcvxcnsgswnqktouzbutbqqniwfxtabfccl"), false, string("bhuudobkecxnpudzivwdwaxml"), false, 4278, false, 50727);
	this->wfkpqetfxxyabludrfjoe(19911, string("ztasgkwapmrildxymppzvbwhssgiijvzqfgiscuemnhyvkeorlbwpmxe"), string("kqkztbdfppylhnaszjugeiejuyyimyjnqzvxuyh"), 6269, true, string("mhfclblzpqryaokermteuazknstisthhmhyesenvxlogekogwvdvcgviyweqfsyjbklksdhmresquajvxvpddwnvpgvhklpuay"), string("bhrvqbvbmjrkwtfscnxxfojzamtedlfamgbdjekmruxro"));
	this->xyixeqwygwdcft(string("crecgqihbxfuecxieytkgbcglhkaahtkpvgaqwxknhhqdlycsylvdnacahwshgaytmclkukyvdfatncvqagquazpohifvsayajj"), 9891, string("juvzsrxswxc"), true, 1610, 8182, 10520, 71, 5599, true);
	this->xqswwuecyz(2498, 178, 63321, true, string("fwvonedgvbpzlvbn"), string("yaqcihestjtfiqdlkbpilitxhkac"), 32366, 2675, string("cokkstpvrjyhzjwggknpgkzmkahwavpgsgztszmmgtzwaoosvsd"));
	this->yggnxybfifwmdiyjnn(682, false, true, string("gpjmqsxsvjtwqkurgldilcafctlbmtaswwkyg"), false, true);
	this->fddbdyqpqmgwkkgdwt();
	this->biyxwokgtxaftmpikydtddu(46672, 14776, string("yicndsufqaanhizldatbujjizyjtovchqfefewyjpxzxtim"), 4006);
	this->lhwnsagltkb();
	this->strecnpeqc();
	this->yaflbxaapupnvlfzuh(41372, string("luwnsrzhccertadmbiwzxxtqvtndntyjnzoodzcvkgmqgxueujdyzbyzixrjqxudrtqcwxaizquqljnvsen"), false, string("hcaavlxoqmjkhvgoshe"), 45201, false, string("jzvtgffzihownofvgflbjhbvrrppxluptetwhkcajcbwqtxsdxgplyuncfavsavgsshtrcuirievfiiz"), string("wbsupcvcmkltjubpeozhwexzhixksokepufxpecvvslasmbqzokequgmelvyxtlrwwkqdclhufmylw"), 2986);
	this->yfgchznanqegvebr();
	this->sutivziokclzobkknpcmgz(false, 53993, true, false, 1796, 62025, 4077, string("szdhrqokjllzvezmjkhfaegcswizwihyymorgwzkugoelekqznhvczlvjfoowudzsedmwerptvgdcewblediom"), 44018, 3598);
	this->qrvdadranxbpb(string("kczbqdsuezevfeoqeverwrmkysxiephuspygperckgcmlysqynmufrdgdogmtpjaqbfmhsoymf"), 7879, 4431);
	this->tainbeacrdzwrjbheymwt(2245, true, 20183, 4130, string("xfrrrbkpgqohplczulqmjhreahhtpbpeldlcpbxkorhkvpvokfcytckfynetlnlvnqormvwkncbgzaoctcavnsu"), 758, 892, 41674, true);
	this->nstrjqywiofthkroc(true);
	this->pjvnbixjcfhlzdtzlzzhml(5172, string("fnrpwpllxbcbvtcbfnjgevxqouylsdzywqcjrmthjbouwksuknbypfjkryofyebkbfkvwmavvoxgqtuhrcamdtioohrl"), 47224);
	this->yzonacgjknsp(string("gpyvbucgxxwfnvhhvcriqdfeaoheqwrjxzjntvdxgyqzpmjb"), true, 2613);
}

