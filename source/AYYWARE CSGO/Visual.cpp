#include "Visuals.h"
#include "Interfaces.h"
#include "RenderManager.h"

void CVisuals::Init()
{
}

void CVisuals::Move(CUserCmd *pCmd, bool &bSendPacket) {}



void CVisuals::Draw()
{
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Menu::Window.VisualsTab.OtherNoScope.GetState() && pLocal->IsAlive() && pLocal->IsScoped())
		NoScopeCrosshair();

	switch (Menu::Window.VisualsTab.OtherCrosshair.GetIndex())
	{
	case 0:
		Interfaces::Engine->ClientCmd_Unrestricted("crosshair 1");
		break;
	case 1:
		Interfaces::Engine->ClientCmd_Unrestricted("crosshair 0");
		DrawRecoilCrosshair();
		break;
	case 2:
		Interfaces::Engine->ClientCmd_Unrestricted("crosshair 0");
		break;
	}

	if (Menu::Window.VisualsTab.SniperCrosshair.GetState())
		DefaultCrosshair();

	if (Menu::Window.VisualsTab.Lines.GetState())
		haccc();

}

void CVisuals::NoScopeCrosshair()
{
	RECT View = Render::GetViewport();
	int MidX = View.right / 2;
	int MidY = View.bottom / 2;

	IClientEntity* pLocal = hackManager.pLocal();
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (GameUtils::IsSniper(pWeapon))
	{
		Render::Line(MidX - 1000, MidY, MidX + 1000, MidY, Color(0, 0, 0, 255));
		Render::Line(MidX, MidY - 1000, MidX, MidY + 1000, Color(0, 0, 0, 255));
	}
}

void CVisuals::DrawCrosshair()
{
	RECT View = Render::GetViewport();
	int MidX = View.right / 2;
	int MidY = View.bottom / 2;

	Render::Line(MidX - 10, MidY, MidX + 10, MidY, Color(0, 255, 0, 255));
	Render::Line(MidX, MidY - 10, MidX, MidY + 10, Color(0, 255, 0, 255));
}

void CVisuals::DrawRecoilCrosshair()
{
	IClientEntity *pLocal = hackManager.pLocal();

	Vector ViewAngles;
	Interfaces::Engine->GetViewAngles(ViewAngles);
	ViewAngles += pLocal->localPlayerExclusive()->GetAimPunchAngle() * 2.f;

	Vector fowardVec;
	AngleVectors(ViewAngles, &fowardVec);
	fowardVec *= 10000;

	Vector start = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector end = start + fowardVec, endScreen;

	if (Render::WorldToScreen(end, endScreen) && pLocal->IsAlive())
	{
		Render::Line(endScreen.x - 10, endScreen.y, endScreen.x + 10, endScreen.y, Color(0, 255, 0, 255));
		Render::Line(endScreen.x, endScreen.y - 10, endScreen.x, endScreen.y + 10, Color(0, 255, 0, 255));
	}
}

void CVisuals::DefaultCrosshair()
{
	IClientEntity *pLocal = hackManager.pLocal();

	if (!pLocal->IsScoped() && pLocal->IsAlive())
	{
		ConVar* cross = Interfaces::CVar->FindVar("weapon_debug_spread_show");
		SpoofedConvar* cross_spoofed = new SpoofedConvar(cross);
		cross_spoofed->SetInt(3);
	}
}

void CVisuals::haccc()
{
	 float lineRealAngle;
	 float lineFakeAngle;
	IClientEntity *pLocal = hackManager.pLocal();

	Vector src3D, dst3D, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = hackManager.pLocal();

	AngleVectors(QAngle(0, pLocal->GetLowerBodyYaw(), 0), &forward);
	src3D = hackManager.pLocal()->GetOrigin();
	dst3D = src3D + (forward * 35.f); //replace 50 with the length you want the line to have

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, 0, &filter, &tr);

	if (!Render::WorldToScreen(src3D, src) || !Render::WorldToScreen(tr.endpos, dst))
		return;

	Render::Line(src.x, src.y, dst.x, dst.y, Color(255, 165, 0, 255));
	Render::Text(dst.x, dst.y, Color(255, 165.f, 0, 255), Render::Fonts::ESP, "lby");

	AngleVectors(QAngle(0, lineRealAngle, 0), &forward);
	dst3D = src3D + (forward * 40.f); //replace 50 with the length you want the line to have

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, 0, &filter, &tr);

	if (!Render::WorldToScreen(src3D, src) || !Render::WorldToScreen(tr.endpos, dst))
		return;

	Render::Line(src.x, src.y, dst.x, dst.y, Color(0, 255, 0, 255));
	Render::Text(dst.x, dst.y, Color(0, 255.f, 0, 255), Render::Fonts::ESP, "real");

	AngleVectors(QAngle(0, lineFakeAngle, 0), &forward);
	dst3D = src3D + (forward * 45.f); //replace 50 with the length you want the line to have

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, 0, &filter, &tr);

	if (!Render::WorldToScreen(src3D, src) || !Render::WorldToScreen(tr.endpos, dst))
		return;

	Render::Line(src.x, src.y, dst.x, dst.y, Color(255, 0, 0, 255));
	Render::Text(dst.x, dst.y, Color(255, 0.f, 0, 255), Render::Fonts::ESP, "fake");
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class bbkgxvc {
public:
	string bfyjtawkcrgnkqk;
	double tasfpu;
	bool phqbc;
	bbkgxvc();
	int zuixogekrdsaubq(int xvmndjom, int dlwbaug, int focyv, string qwcyrywpwljmdri, string xyzhlzixgbjckxs, bool vbzlldqm, int uqlrimgzk, string sykehx, bool dxmyw);
	string fhfpxdlqcwicnjlemfoqs(string ckpafpushqvchv, bool ezphslrpocq, string qzprihapdarcms, string gjupmdodhpevilx, string sbnbhwmpioippoe);
	int gqbfdrmuwmrqwukcmlbvccud(double nnwzkgwwpjzvvzx, double dhmcgv, double iixpih, int knuiidsciasrx, double qtfrcgbmzs, int vzcndvssuydiug);
	string uoaywlmqfzgfgzcseyyejnil();
	bool pvoeswzyswedqstlypeutuul(int bhdvfw, bool tpsoq, bool katoalgf, string iivperhdrdzv, double rewcyobazoqtktg, string gsrylyimv, string pxvkmtpbnr);

protected:
	bool jztegmrpoyxj;
	string nifdtqh;

	int uziehogbiyoywl(bool itskajso);
	bool jpldpxmwokszap(bool scxescxsksy, string ieymbkafwlcww, double mkhuiexy, string yqomu);
	int zrhuiahriyy(string xclofhei, string navkypeynwcbhcb, int cfvwbmzhgwm, double fcjfi, int fhwresbkogehy, double wumqqnhtzz);
	double eaelwbizvbkfsmigqlweye();
	string ziihhrcvzeheyswb(int wuxbowogpp, double jbifvqlzgzqtyei, int hglswkhfnvhhpyq, string vetnjprlpkye, int faeuf, string esacuvv, string jlpzyqjgauutivr, double efjqdmsdhvpgqv, double zntckbytqk, string mtjuuljg);
	string bvriwjnhsvraqbm(int vlvugwc, string grbzta, bool mwhynozklgk, double kxcmpzec, int akxzmixhvq, bool annrcpzw, string qbfzqawdvnw, string rdjmxtwrzyaste, double qmewiyvusps);
	bool sgpkqygalmldk(bool hllsqewkyejyz, bool vahsvoenucat, double vnubrbdtnokbec);
	string gcvkymwvevsfudlbmhzehbci();

private:
	int qabwowzwvxe;
	int dyuyt;
	bool sbrromswaq;

	int aktjqcpslbtbuwfclb(double pvbkkqrmqf, double lrrdvifex, int gtzktieusydo, int mxeeathpcka, double lbqzqp, bool qtnzwvbec, double metsgnkcmnov);

};


int bbkgxvc::aktjqcpslbtbuwfclb(double pvbkkqrmqf, double lrrdvifex, int gtzktieusydo, int mxeeathpcka, double lbqzqp, bool qtnzwvbec, double metsgnkcmnov) {
	string kmwdskmwxeeaea = "jpsrsjbwvgivxwhcqmvskxosephdlnbqscxlnenfairpbdqjgckrebks";
	double jcshbakkgsfqtxz = 29758;
	if (29758 == 29758) {
		int apxksrvw;
		for (apxksrvw = 46; apxksrvw > 0; apxksrvw--) {
			continue;
		}
	}
	if (29758 != 29758) {
		int eojmrusvv;
		for (eojmrusvv = 83; eojmrusvv > 0; eojmrusvv--) {
			continue;
		}
	}
	if (29758 == 29758) {
		int dckyt;
		for (dckyt = 43; dckyt > 0; dckyt--) {
			continue;
		}
	}
	if (string("jpsrsjbwvgivxwhcqmvskxosephdlnbqscxlnenfairpbdqjgckrebks") != string("jpsrsjbwvgivxwhcqmvskxosephdlnbqscxlnenfairpbdqjgckrebks")) {
		int kdswpr;
		for (kdswpr = 85; kdswpr > 0; kdswpr--) {
			continue;
		}
	}
	if (29758 != 29758) {
		int lfzol;
		for (lfzol = 16; lfzol > 0; lfzol--) {
			continue;
		}
	}
	return 59299;
}

int bbkgxvc::uziehogbiyoywl(bool itskajso) {
	string lsfepzyumbjbl = "ttdsjvrzhgwqylgqnbcoeckklhemxhvgbkgfpitlytjoracffapmndkoxzakfzoodgtycbudvcqlwoagrbho";
	if (string("ttdsjvrzhgwqylgqnbcoeckklhemxhvgbkgfpitlytjoracffapmndkoxzakfzoodgtycbudvcqlwoagrbho") != string("ttdsjvrzhgwqylgqnbcoeckklhemxhvgbkgfpitlytjoracffapmndkoxzakfzoodgtycbudvcqlwoagrbho")) {
		int zk;
		for (zk = 91; zk > 0; zk--) {
			continue;
		}
	}
	if (string("ttdsjvrzhgwqylgqnbcoeckklhemxhvgbkgfpitlytjoracffapmndkoxzakfzoodgtycbudvcqlwoagrbho") != string("ttdsjvrzhgwqylgqnbcoeckklhemxhvgbkgfpitlytjoracffapmndkoxzakfzoodgtycbudvcqlwoagrbho")) {
		int tqnb;
		for (tqnb = 85; tqnb > 0; tqnb--) {
			continue;
		}
	}
	if (string("ttdsjvrzhgwqylgqnbcoeckklhemxhvgbkgfpitlytjoracffapmndkoxzakfzoodgtycbudvcqlwoagrbho") == string("ttdsjvrzhgwqylgqnbcoeckklhemxhvgbkgfpitlytjoracffapmndkoxzakfzoodgtycbudvcqlwoagrbho")) {
		int odcxkeaws;
		for (odcxkeaws = 99; odcxkeaws > 0; odcxkeaws--) {
			continue;
		}
	}
	if (string("ttdsjvrzhgwqylgqnbcoeckklhemxhvgbkgfpitlytjoracffapmndkoxzakfzoodgtycbudvcqlwoagrbho") != string("ttdsjvrzhgwqylgqnbcoeckklhemxhvgbkgfpitlytjoracffapmndkoxzakfzoodgtycbudvcqlwoagrbho")) {
		int at;
		for (at = 59; at > 0; at--) {
			continue;
		}
	}
	return 53053;
}

bool bbkgxvc::jpldpxmwokszap(bool scxescxsksy, string ieymbkafwlcww, double mkhuiexy, string yqomu) {
	return false;
}

int bbkgxvc::zrhuiahriyy(string xclofhei, string navkypeynwcbhcb, int cfvwbmzhgwm, double fcjfi, int fhwresbkogehy, double wumqqnhtzz) {
	int iafng = 4391;
	int bgpifyrbqcfoqg = 4056;
	bool wggtucgpcpxi = true;
	string btqlqibkcjlsgjb = "grixkaitnffpkdldrehhyclzpxmsiujtzmoggrdvjrvvc";
	int zlhqdvf = 328;
	string npvzdzfwrpbdo = "fjiubcslgwmhzlknbhkuhztkehzanvhavoyexonvvpxdwpzmqoqrjpiaddqmqxwcb";
	double pgpddwpjvqmt = 7857;
	int xwqoyyjisusf = 1609;
	string hcvherlobuvjc = "niyrkqhrpmfhpunilhuxukhiyhaygqlzktouttvetqsjacp";
	if (4391 == 4391) {
		int eq;
		for (eq = 52; eq > 0; eq--) {
			continue;
		}
	}
	return 88835;
}

double bbkgxvc::eaelwbizvbkfsmigqlweye() {
	bool ksgkfyabxhta = true;
	bool ipwzpnrf = true;
	bool itriignenrjeml = true;
	bool vqmztgr = false;
	double xgruqdwicdfeoqz = 60160;
	bool kcdicds = true;
	int dljgkbldcpb = 1372;
	string qqrilgofnwtg = "nwflwvvqutjtbnmxjmmsrnorelkagixwvkyozgjxqahbhktntaoudllynzoxnrdvkhmpqnqrxjneaehojzzyiruhozby";
	double nhjgqvjhbvbijrq = 33773;
	int pvxyuskrmfc = 438;
	if (438 == 438) {
		int gjvnfqlcg;
		for (gjvnfqlcg = 63; gjvnfqlcg > 0; gjvnfqlcg--) {
			continue;
		}
	}
	if (true != true) {
		int qgs;
		for (qgs = 26; qgs > 0; qgs--) {
			continue;
		}
	}
	if (true != true) {
		int yip;
		for (yip = 29; yip > 0; yip--) {
			continue;
		}
	}
	return 69119;
}

string bbkgxvc::ziihhrcvzeheyswb(int wuxbowogpp, double jbifvqlzgzqtyei, int hglswkhfnvhhpyq, string vetnjprlpkye, int faeuf, string esacuvv, string jlpzyqjgauutivr, double efjqdmsdhvpgqv, double zntckbytqk, string mtjuuljg) {
	string vfuynokoa = "sucecejcmbixgsupljuah";
	double wcdbpvgw = 2262;
	string ftrjxintycs = "wlxlhuldqehpvwokujfkdoieeenvkybspklhdhwekmdyombduwkmsgookjkqhjcuewtcttbn";
	bool rlmagmyppoey = false;
	bool oqpbntyy = false;
	double rddalpzwlyaquae = 2620;
	double bqqinr = 74709;
	bool oajyi = true;
	if (string("wlxlhuldqehpvwokujfkdoieeenvkybspklhdhwekmdyombduwkmsgookjkqhjcuewtcttbn") == string("wlxlhuldqehpvwokujfkdoieeenvkybspklhdhwekmdyombduwkmsgookjkqhjcuewtcttbn")) {
		int dmdlpc;
		for (dmdlpc = 24; dmdlpc > 0; dmdlpc--) {
			continue;
		}
	}
	if (true != true) {
		int cmfoecu;
		for (cmfoecu = 40; cmfoecu > 0; cmfoecu--) {
			continue;
		}
	}
	if (string("wlxlhuldqehpvwokujfkdoieeenvkybspklhdhwekmdyombduwkmsgookjkqhjcuewtcttbn") == string("wlxlhuldqehpvwokujfkdoieeenvkybspklhdhwekmdyombduwkmsgookjkqhjcuewtcttbn")) {
		int xmyrdcltyt;
		for (xmyrdcltyt = 94; xmyrdcltyt > 0; xmyrdcltyt--) {
			continue;
		}
	}
	return string("pietrpjmkft");
}

string bbkgxvc::bvriwjnhsvraqbm(int vlvugwc, string grbzta, bool mwhynozklgk, double kxcmpzec, int akxzmixhvq, bool annrcpzw, string qbfzqawdvnw, string rdjmxtwrzyaste, double qmewiyvusps) {
	bool tvsvtjcteutrxo = true;
	if (true == true) {
		int aquvcotaa;
		for (aquvcotaa = 58; aquvcotaa > 0; aquvcotaa--) {
			continue;
		}
	}
	if (true != true) {
		int lfslnt;
		for (lfslnt = 74; lfslnt > 0; lfslnt--) {
			continue;
		}
	}
	if (true == true) {
		int gwzby;
		for (gwzby = 33; gwzby > 0; gwzby--) {
			continue;
		}
	}
	return string("mwu");
}

bool bbkgxvc::sgpkqygalmldk(bool hllsqewkyejyz, bool vahsvoenucat, double vnubrbdtnokbec) {
	bool dvlyxsdka = false;
	int hmmthckui = 1241;
	bool xfipqusdzp = true;
	bool jhvktfsailawjo = false;
	bool vzoajifyrgy = true;
	string mqfucuytqe = "nrxwqaaastmlqtufrnjspqoangaoxylbdsnlukbpnoqgvoknbdapvkmplrup";
	string frdjg = "zrhzjioxbzgbxswtkgbhpklilzkkkzmslsxsywawcnpobqewsehxpgvtakqilmuzgnyybui";
	if (true == true) {
		int yyzvmxrkbp;
		for (yyzvmxrkbp = 16; yyzvmxrkbp > 0; yyzvmxrkbp--) {
			continue;
		}
	}
	return false;
}

string bbkgxvc::gcvkymwvevsfudlbmhzehbci() {
	double kqgsoe = 10985;
	double gplvwb = 17536;
	double nahfxxukdqclwsk = 26861;
	if (17536 != 17536) {
		int ly;
		for (ly = 74; ly > 0; ly--) {
			continue;
		}
	}
	if (26861 != 26861) {
		int cckcqbi;
		for (cckcqbi = 79; cckcqbi > 0; cckcqbi--) {
			continue;
		}
	}
	return string("bxjjiekbjtrjnqgmn");
}

int bbkgxvc::zuixogekrdsaubq(int xvmndjom, int dlwbaug, int focyv, string qwcyrywpwljmdri, string xyzhlzixgbjckxs, bool vbzlldqm, int uqlrimgzk, string sykehx, bool dxmyw) {
	string jfvhizgde = "esirlbfuculrgpkfkjadjd";
	string goisxrwp = "cnacczqilekvirpjcnjuafpexmiwigjzhqaefwgmyckigfsgerbfcvteaqhiptnrretrramimdrvbxbofiaetf";
	int nyhxkpsliy = 2168;
	int dlvpisodirqwuy = 7775;
	if (2168 != 2168) {
		int ndo;
		for (ndo = 63; ndo > 0; ndo--) {
			continue;
		}
	}
	if (string("esirlbfuculrgpkfkjadjd") != string("esirlbfuculrgpkfkjadjd")) {
		int jdyjigrczw;
		for (jdyjigrczw = 81; jdyjigrczw > 0; jdyjigrczw--) {
			continue;
		}
	}
	if (string("esirlbfuculrgpkfkjadjd") != string("esirlbfuculrgpkfkjadjd")) {
		int oyomt;
		for (oyomt = 7; oyomt > 0; oyomt--) {
			continue;
		}
	}
	return 91613;
}

string bbkgxvc::fhfpxdlqcwicnjlemfoqs(string ckpafpushqvchv, bool ezphslrpocq, string qzprihapdarcms, string gjupmdodhpevilx, string sbnbhwmpioippoe) {
	string adubortjc = "lcbgqklgxuqoimykv";
	double kohwjkyw = 13621;
	bool sweuih = false;
	double ydpyu = 74457;
	bool peyymxjbnslwvhh = false;
	string ghgtcnvo = "zmedpazwxtzwiivfiopbnfuxxdylpinfjaqqame";
	double jczdsrwcuyavqpj = 14637;
	double jmbihxfjuh = 15576;
	bool yppcogqblafdjlb = false;
	double zzfyaf = 68167;
	if (false != false) {
		int mpacslxhp;
		for (mpacslxhp = 22; mpacslxhp > 0; mpacslxhp--) {
			continue;
		}
	}
	if (14637 != 14637) {
		int aui;
		for (aui = 56; aui > 0; aui--) {
			continue;
		}
	}
	if (14637 != 14637) {
		int bsztnof;
		for (bsztnof = 88; bsztnof > 0; bsztnof--) {
			continue;
		}
	}
	if (false == false) {
		int gxd;
		for (gxd = 21; gxd > 0; gxd--) {
			continue;
		}
	}
	if (string("zmedpazwxtzwiivfiopbnfuxxdylpinfjaqqame") == string("zmedpazwxtzwiivfiopbnfuxxdylpinfjaqqame")) {
		int wiza;
		for (wiza = 43; wiza > 0; wiza--) {
			continue;
		}
	}
	return string("vnfxo");
}

int bbkgxvc::gqbfdrmuwmrqwukcmlbvccud(double nnwzkgwwpjzvvzx, double dhmcgv, double iixpih, int knuiidsciasrx, double qtfrcgbmzs, int vzcndvssuydiug) {
	bool bivzj = false;
	if (false == false) {
		int arm;
		for (arm = 41; arm > 0; arm--) {
			continue;
		}
	}
	if (false != false) {
		int upuih;
		for (upuih = 80; upuih > 0; upuih--) {
			continue;
		}
	}
	return 29088;
}

string bbkgxvc::uoaywlmqfzgfgzcseyyejnil() {
	string ydiwgyrfwdvvrw = "gcfxndqznftossdmoarehvurnzonoxnsgjjjbzpyrvqtdvbdadwujuaqnqbneizimthofozgwativuswkhnsnpz";
	double pqtcsmo = 21146;
	string fysjqaqrbajxmdq = "iulklrklnmqcfiiqyqnsihhdxjtbkjgokoozfzstjojyvqlnthwvjqvddgahfuvprxtnskyisbqelhitcgnf";
	int ivcptvutsjqeiwl = 1304;
	string hjnctcautuiivoi = "nlobgjz";
	if (21146 != 21146) {
		int zjbubkhhj;
		for (zjbubkhhj = 75; zjbubkhhj > 0; zjbubkhhj--) {
			continue;
		}
	}
	if (string("iulklrklnmqcfiiqyqnsihhdxjtbkjgokoozfzstjojyvqlnthwvjqvddgahfuvprxtnskyisbqelhitcgnf") == string("iulklrklnmqcfiiqyqnsihhdxjtbkjgokoozfzstjojyvqlnthwvjqvddgahfuvprxtnskyisbqelhitcgnf")) {
		int jmmpv;
		for (jmmpv = 77; jmmpv > 0; jmmpv--) {
			continue;
		}
	}
	if (string("gcfxndqznftossdmoarehvurnzonoxnsgjjjbzpyrvqtdvbdadwujuaqnqbneizimthofozgwativuswkhnsnpz") == string("gcfxndqznftossdmoarehvurnzonoxnsgjjjbzpyrvqtdvbdadwujuaqnqbneizimthofozgwativuswkhnsnpz")) {
		int tdgrv;
		for (tdgrv = 72; tdgrv > 0; tdgrv--) {
			continue;
		}
	}
	if (string("gcfxndqznftossdmoarehvurnzonoxnsgjjjbzpyrvqtdvbdadwujuaqnqbneizimthofozgwativuswkhnsnpz") != string("gcfxndqznftossdmoarehvurnzonoxnsgjjjbzpyrvqtdvbdadwujuaqnqbneizimthofozgwativuswkhnsnpz")) {
		int rsrtlzikjc;
		for (rsrtlzikjc = 13; rsrtlzikjc > 0; rsrtlzikjc--) {
			continue;
		}
	}
	return string("");
}

bool bbkgxvc::pvoeswzyswedqstlypeutuul(int bhdvfw, bool tpsoq, bool katoalgf, string iivperhdrdzv, double rewcyobazoqtktg, string gsrylyimv, string pxvkmtpbnr) {
	return false;
}

bbkgxvc::bbkgxvc() {
	this->zuixogekrdsaubq(1201, 1480, 4820, string("tqvtaziajqtzynmiahftkkgnnuxsxpuqtrauektvbkhakquheensqmectdcryozvpetxkciozdvserizcqktaxozwgkxwq"), string("caxhltejqgunfsdqzhuakjp"), true, 7557, string("rxzvffbupxhezxiygyizgdmehxpumyaddkljiddtk"), false);
	this->fhfpxdlqcwicnjlemfoqs(string("zlvvpquqrfojxbsahbingctdayuazgmhuzwtlbseetammwtnwozykkondaqbprshkehwvqjcqrbavoka"), false, string("smovheumprzjrrcdcyxikeurxfgwdubqywhtzsrdfdzrfdhnvshqyebwvm"), string("ldazjakelfawlspojtvxjfnxychrxvapiweucicqhgfzcdlsxuwwngimxppxvkfztyziynlsjxeuguqm"), string("hinwfqxkihvftgsvdnhukctggdytcvpuqnrnnowgspnomuvpinyrlipyqexpwdegycuqsgs"));
	this->gqbfdrmuwmrqwukcmlbvccud(30242, 28239, 58671, 1290, 47749, 884);
	this->uoaywlmqfzgfgzcseyyejnil();
	this->pvoeswzyswedqstlypeutuul(1700, true, false, string("ecdzsopknhsgbwx"), 9575, string("tbtqblxisjqutgugzazcihdpvevubwlisbsahaozouookifyewnqhlldbdoxgjgeoczopsmfhfyuxsmqugo"), string("dhbyonthogkuulcuwsekxakzejhgmbkpeikpzqvytmwowlmufbaofnzghf"));
	this->uziehogbiyoywl(false);
	this->jpldpxmwokszap(false, string("necphzfhnakkallzsqieokninrzjmehhcbxqrdtaxnapdyguhzpgrrtrkbnwupfweseet"), 14400, string("pmmlegfkyvrrgjuvuncmehwvdhfuvhnzrnd"));
	this->zrhuiahriyy(string("hiuqcainohmqydbgqhqvzuezrptemi"), string("yqldmbscejsjqbsinxaabiocvnefqcwksnskaducwvxnkpgazjxmdt"), 1605, 39464, 1127, 8194);
	this->eaelwbizvbkfsmigqlweye();
	this->ziihhrcvzeheyswb(1493, 15055, 1751, string("vzflodeaxhprozndiivjugkynoazbcmihfqnnlwvjhgbanajrrsmqykhrtzxipmmxtqcmwga"), 2101, string("qpycsoan"), string("ajipnpssetpeijmfwhhoqvvwvkhgcgwcrgufrsnxkuyqambsrvkwkbbnbhblthiohtvnznwnwxpxae"), 19716, 59578, string("gaptwbqvcjyqxhafivd"));
	this->bvriwjnhsvraqbm(3016, string("ujswxumogzjvjmojnaqqurpdbiehfsqciapvkthtmcyrwbwgltesgmrg"), true, 64132, 7831, false, string("csskcfxeeauglgmbkgaqcsm"), string("isujxhximxtewylwozarapxqlgagszoecyuoxntbsyvlgxadqyjkgilezsxdebogegjkvwm"), 51310);
	this->sgpkqygalmldk(true, false, 28997);
	this->gcvkymwvevsfudlbmhzehbci();
	this->aktjqcpslbtbuwfclb(37897, 41460, 4190, 4498, 63483, false, 26979);
}


#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class fwyfmij {
public:
	double sjjlzc;
	string xeflkdnrvkzvqv;
	fwyfmij();
	bool uueihubnkrzxx(string madkrsjkw, double jvppqlu, int vvxdxmjmb, int uxdxvvhpxgq, bool pgfnrc);

protected:
	string aqobzpfwv;
	double zslhlql;
	double kefvblfol;
	bool ufurfd;

	double ikiezuoorxrpgdlfgjcis(bool gshkkclaigcdhzc, int cyfrwgfbv, string lrgcdxjjtr, bool ekayydkdmhnbq, double hooflhieki, double wseip);
	double vfhoeiprjqb();
	string ijcjgctjftgtc(int fljauv, double midunnlzcpvc, string tganyi, bool jllghkzqts, int reeehocdslgg);
	double utxrurffunilzyptfsqksbydg(string ntygpb, bool dbwefuqmsxi, string crxsbxgpcuaey, string jntkrvshojmej, bool tepbgqw);
	void tyayhhviddbpe(bool ceuydomgfcpdafq, int uapgxg, bool vdhxxhkkq, string fxwpexb, double txztiasoj, bool pvguqdupqdmar, string ukrtbpgok, int vpezmjnf, bool yhvngo);
	bool crrfjnvmwhonzplrhkkhzfn(string jobgfkatre, string sxthaesb, bool osytbrgjr);
	bool xcmquyxmoiavfgrbqunmrwc(string pvhuugnlyanmuc, double mffrxwmykxtriwn, int trhhckoxuaopbki, int wamej, int iutiqlqmoc, double pgjshalh, string ywfvhebw, string uwysq, double xavcaxwsepa, double freclszzfkq);

private:
	string kranvlepow;
	bool ldqfhwwkcuh;

	double vfjzlbicsqzuup(double ngokmvuawsqzfzs, bool jvvfzr, double ltxtrhuoysy, string kzufmoceilljn, string wdenzdabr, int ztmph, string cmfrjydsxj);
	int tfaqwctnqxoscpnjde(string uarmceh, int jyzajvy, string romelcw, bool tksiieyjorggqz, double ekvgypaavsbd, string fvvlsoaxazflcg, double bamysr);
	string vixfofabdf(bool sedujijqsb);
	int qpsgeuzoojptukkozqlh(double wrwahfoufmjbzm);

};


double fwyfmij::vfjzlbicsqzuup(double ngokmvuawsqzfzs, bool jvvfzr, double ltxtrhuoysy, string kzufmoceilljn, string wdenzdabr, int ztmph, string cmfrjydsxj) {
	return 36276;
}

int fwyfmij::tfaqwctnqxoscpnjde(string uarmceh, int jyzajvy, string romelcw, bool tksiieyjorggqz, double ekvgypaavsbd, string fvvlsoaxazflcg, double bamysr) {
	bool bedrtyxrtapqxx = false;
	bool kacwdc = true;
	double phkedohlez = 4290;
	int ntpptlpynvahl = 8140;
	bool goxmqch = false;
	bool anorbpczgvebysp = true;
	string nmmgaak = "xjyjgkspawljeoyetdboppbzklrpkkzdkjhxymibotznpfsxcwddkttrfmyqwuyonxoiqnekvvlbzrkkvvknigrf";
	bool xezbmybrlmhgmn = false;
	double gdvevxbdx = 26231;
	return 33411;
}

string fwyfmij::vixfofabdf(bool sedujijqsb) {
	double qcmcbewboi = 23976;
	double yybzbyrxxvbbp = 29502;
	int fziteoxqboyhue = 7486;
	double fpohgehsxw = 4076;
	bool sarzs = false;
	if (23976 != 23976) {
		int wmu;
		for (wmu = 29; wmu > 0; wmu--) {
			continue;
		}
	}
	if (7486 == 7486) {
		int akfqdmzfkv;
		for (akfqdmzfkv = 11; akfqdmzfkv > 0; akfqdmzfkv--) {
			continue;
		}
	}
	if (23976 != 23976) {
		int lvehsyh;
		for (lvehsyh = 41; lvehsyh > 0; lvehsyh--) {
			continue;
		}
	}
	if (7486 == 7486) {
		int obtwxfovm;
		for (obtwxfovm = 35; obtwxfovm > 0; obtwxfovm--) {
			continue;
		}
	}
	return string("xjd");
}

int fwyfmij::qpsgeuzoojptukkozqlh(double wrwahfoufmjbzm) {
	string ixluveofkrjhd = "jhsnkmesdoqxxvtdjzubbtgmaeoaoobjxobadavbjlkopsuenaqryhnyse";
	string unfyzfwzsfyrrww = "jzmcqtsjzepchofaegflonfthrxygwsiqbwyots";
	string rezoxbqtx = "tmlbi";
	bool owobzvvn = true;
	bool vsddpac = false;
	bool abbctjannyl = false;
	if (string("jhsnkmesdoqxxvtdjzubbtgmaeoaoobjxobadavbjlkopsuenaqryhnyse") == string("jhsnkmesdoqxxvtdjzubbtgmaeoaoobjxobadavbjlkopsuenaqryhnyse")) {
		int ya;
		for (ya = 62; ya > 0; ya--) {
			continue;
		}
	}
	if (string("jzmcqtsjzepchofaegflonfthrxygwsiqbwyots") == string("jzmcqtsjzepchofaegflonfthrxygwsiqbwyots")) {
		int olownix;
		for (olownix = 9; olownix > 0; olownix--) {
			continue;
		}
	}
	return 46276;
}

double fwyfmij::ikiezuoorxrpgdlfgjcis(bool gshkkclaigcdhzc, int cyfrwgfbv, string lrgcdxjjtr, bool ekayydkdmhnbq, double hooflhieki, double wseip) {
	bool snwxuxan = true;
	int rujnlavtgqhfqo = 761;
	string ajsix = "nvumbvthskjqycwmerervlizmrixhnjarxzctkgajvlnzn";
	string haral = "jgvonwjvlndicfugbnravhyoctofaqqyhnyyratkkefkjurmkrzqdkeasaejjngrsenwzoxgnfkgewm";
	string kjinzno = "ljepshqagfdsrtlrqctvoyrgxrfvwivrjuyaqhwjtgkvebidiqximqubzqjmrrdzirxzxcbnqmxifqzhwrjffgc";
	bool kjplfzs = false;
	double thcsrlovnpkgg = 26430;
	bool uexbn = true;
	string xqwhxnydms = "qdfeffdxicaiihsdvwshpiylmpeaomrokhmwevxxtdilwihx";
	bool lktfufjxgpxh = false;
	if (761 != 761) {
		int fyppcgqe;
		for (fyppcgqe = 58; fyppcgqe > 0; fyppcgqe--) {
			continue;
		}
	}
	if (string("ljepshqagfdsrtlrqctvoyrgxrfvwivrjuyaqhwjtgkvebidiqximqubzqjmrrdzirxzxcbnqmxifqzhwrjffgc") != string("ljepshqagfdsrtlrqctvoyrgxrfvwivrjuyaqhwjtgkvebidiqximqubzqjmrrdzirxzxcbnqmxifqzhwrjffgc")) {
		int xqazovox;
		for (xqazovox = 5; xqazovox > 0; xqazovox--) {
			continue;
		}
	}
	if (26430 != 26430) {
		int dozj;
		for (dozj = 1; dozj > 0; dozj--) {
			continue;
		}
	}
	return 34542;
}

double fwyfmij::vfhoeiprjqb() {
	string gssfwnbyqjxgw = "rgtzpevgmuursyblheyawrozxjxmqteloqsrivimohecgerl";
	string vxlhjuarfpz = "bbmmqttqejwlcawgukfzwnyogviofgqxncypvezeowegjrtuzenbbtijdljskbdxjnzxeuwvfwmzokaffuqyadxbfdkb";
	bool cxotkgmvcdn = false;
	double fzixyowxscrhb = 13888;
	double brpbx = 19151;
	int wxrvppqxwwwo = 2092;
	int rrjqcilonde = 3310;
	string eqbiptd = "chepjaaxwcbhsjadnnszniyqhebshxrjhxiodovqzcwyhwto";
	string kwyzfyxnktv = "fflstzgjmlgpiplujobnvsklyxaalvnxfwovhhcq";
	if (13888 != 13888) {
		int vsimyx;
		for (vsimyx = 35; vsimyx > 0; vsimyx--) {
			continue;
		}
	}
	if (3310 != 3310) {
		int gjawoi;
		for (gjawoi = 98; gjawoi > 0; gjawoi--) {
			continue;
		}
	}
	if (2092 == 2092) {
		int yspo;
		for (yspo = 37; yspo > 0; yspo--) {
			continue;
		}
	}
	return 6095;
}

string fwyfmij::ijcjgctjftgtc(int fljauv, double midunnlzcpvc, string tganyi, bool jllghkzqts, int reeehocdslgg) {
	int iqrynpsxgghav = 2234;
	bool owgxzi = false;
	double ztcpabpqdscnyw = 26825;
	int wjlfosocg = 5148;
	string kadzoqftxhbgqds = "jrljanlqndbzpjwtvxuyvzstsootbizudpfbyhkrzmruigqxshprvxqhzurcqw";
	double ykxnzxpta = 49503;
	int blkqqt = 2918;
	if (2234 == 2234) {
		int ikkiz;
		for (ikkiz = 77; ikkiz > 0; ikkiz--) {
			continue;
		}
	}
	if (5148 != 5148) {
		int siyfgxy;
		for (siyfgxy = 54; siyfgxy > 0; siyfgxy--) {
			continue;
		}
	}
	if (string("jrljanlqndbzpjwtvxuyvzstsootbizudpfbyhkrzmruigqxshprvxqhzurcqw") == string("jrljanlqndbzpjwtvxuyvzstsootbizudpfbyhkrzmruigqxshprvxqhzurcqw")) {
		int zcjp;
		for (zcjp = 85; zcjp > 0; zcjp--) {
			continue;
		}
	}
	if (false == false) {
		int wavtrrc;
		for (wavtrrc = 93; wavtrrc > 0; wavtrrc--) {
			continue;
		}
	}
	if (5148 != 5148) {
		int fzvfp;
		for (fzvfp = 6; fzvfp > 0; fzvfp--) {
			continue;
		}
	}
	return string("");
}

double fwyfmij::utxrurffunilzyptfsqksbydg(string ntygpb, bool dbwefuqmsxi, string crxsbxgpcuaey, string jntkrvshojmej, bool tepbgqw) {
	int bkvsjaormm = 6479;
	bool kkoreuxhesecv = false;
	string njddfrvunmfhasv = "bcipbadutpxdmnvcpxrosvvitgydvrmhcclryardskkrdzylspskwwjlmlqmagwrmsjemqzqimlitbdzbs";
	int werhknj = 6414;
	int ogcbbdfrbivpm = 4479;
	bool eqqilqva = true;
	bool ngtcimgkdum = false;
	string dyqgxemwfimongj = "nrkannyfrgifbhobybfnkzwzfmkn";
	string ayzhkxsjykfqckt = "brhdgqnpibrzagqiqveddryvkrpkjfeuzybgwenbajftxcibeunqcobkvtsxqmnkwysowdiskwnxgtvwxlzdhutywizxlruq";
	string ppfppgnpyrv = "hjsrevwdptasjrjazczrndxjkevsmzpnlggyemudyclwgvfglgaaunddzgormurkgoctmnlyxdmjrzcpkcjpfaoble";
	if (6414 != 6414) {
		int lehi;
		for (lehi = 66; lehi > 0; lehi--) {
			continue;
		}
	}
	return 17435;
}

void fwyfmij::tyayhhviddbpe(bool ceuydomgfcpdafq, int uapgxg, bool vdhxxhkkq, string fxwpexb, double txztiasoj, bool pvguqdupqdmar, string ukrtbpgok, int vpezmjnf, bool yhvngo) {
	bool gazmxcrinzpcwnf = false;
	bool bjsls = true;
	bool twjklgd = true;

}

bool fwyfmij::crrfjnvmwhonzplrhkkhzfn(string jobgfkatre, string sxthaesb, bool osytbrgjr) {
	double eusewfzpssoevis = 34569;
	bool itsmndimli = true;
	bool prxponapztpmzs = true;
	int vqojffzbqylhwq = 2118;
	bool iefvrry = false;
	string rksmavsukizi = "msyikm";
	double bpzpdepzaptnna = 27333;
	double hidgsflfkn = 23276;
	if (false == false) {
		int xjezfkb;
		for (xjezfkb = 44; xjezfkb > 0; xjezfkb--) {
			continue;
		}
	}
	if (false != false) {
		int ycaniegsmb;
		for (ycaniegsmb = 45; ycaniegsmb > 0; ycaniegsmb--) {
			continue;
		}
	}
	if (27333 == 27333) {
		int eg;
		for (eg = 79; eg > 0; eg--) {
			continue;
		}
	}
	if (27333 == 27333) {
		int nlls;
		for (nlls = 14; nlls > 0; nlls--) {
			continue;
		}
	}
	return true;
}

bool fwyfmij::xcmquyxmoiavfgrbqunmrwc(string pvhuugnlyanmuc, double mffrxwmykxtriwn, int trhhckoxuaopbki, int wamej, int iutiqlqmoc, double pgjshalh, string ywfvhebw, string uwysq, double xavcaxwsepa, double freclszzfkq) {
	bool bbtjqboagz = false;
	double jaefehs = 72538;
	return true;
}

bool fwyfmij::uueihubnkrzxx(string madkrsjkw, double jvppqlu, int vvxdxmjmb, int uxdxvvhpxgq, bool pgfnrc) {
	bool jwrlmetirly = false;
	int yfunntxynxl = 6510;
	if (6510 != 6510) {
		int kjlqygeal;
		for (kjlqygeal = 45; kjlqygeal > 0; kjlqygeal--) {
			continue;
		}
	}
	if (false != false) {
		int ou;
		for (ou = 51; ou > 0; ou--) {
			continue;
		}
	}
	if (6510 == 6510) {
		int jsktukj;
		for (jsktukj = 17; jsktukj > 0; jsktukj--) {
			continue;
		}
	}
	if (6510 != 6510) {
		int jtf;
		for (jtf = 23; jtf > 0; jtf--) {
			continue;
		}
	}
	return true;
}

fwyfmij::fwyfmij() {
	this->uueihubnkrzxx(string("yhzkdcfuhcq"), 43000, 4700, 1951, true);
	this->ikiezuoorxrpgdlfgjcis(false, 2625, string("gbhkjnbrgwqudcmegjtgepscyuajsovrjkqpolcbjhapuquyvqrxiarerbyricyyv"), false, 1442, 51935);
	this->vfhoeiprjqb();
	this->ijcjgctjftgtc(5845, 5278, string("agtghqmjeoexqrejqwrbgvgatdeuagoc"), true, 5894);
	this->utxrurffunilzyptfsqksbydg(string("sdnaldojodtcpqolasoescacwtwwmvzbazlzrwybmbfsxjdsuicctvbyqysfejmzmjuasuqtgjrbw"), false, string("mdyxhffdn"), string("woflgilavnlorynvxholkletxdgfmrpxabdmdayanyytlvaskzd"), true);
	this->tyayhhviddbpe(false, 829, false, string("zlztbgc"), 11246, true, string("rqtycqxxxdqiffnakonmxxyaqwpivlslrbxzcwfrf"), 5362, false);
	this->crrfjnvmwhonzplrhkkhzfn(string("olsnknmcfpxxnaq"), string("ojfkrzmcxriqlkqzmwsqjwrzobzgapoqvjmhsulwz"), false);
	this->xcmquyxmoiavfgrbqunmrwc(string(""), 20277, 1340, 6705, 1064, 71090, string("xpsctmjukuixuvzpyabemvbnkraxlwajnjazslpmqhhlwpmvegkwighnfrwqojrgrmmkvufbuwkawbmihtjotsqgu"), string("pkohffybmyimhihajtrdabvucztbtbskcuilzfczedhtdtxwgveupejcts"), 63184, 51441);
	this->vfjzlbicsqzuup(13735, false, 1944, string("ppubyrdnnjvkrtxat"), string("vipjutlnvntxrvunvnjkzilvyuzoahwokahodiisurpqibdgsocdonumhcefcpikxsefvwwqncjknbidutsqwmsxqzean"), 6440, string("reuvvdfvgfuuxwtzasoxhlfzzsfxikmlxfhclnguuxakwkp"));
	this->tfaqwctnqxoscpnjde(string("xeyhgpbbqislaoqcvkcmrhkcoqeerfhulzahzvyjvfxwtelwozplthfbkjgzieuzngs"), 448, string("jdwezqezeuudwhpkpvokfqzbuqeiatractwltuveqacuqikbahinrtyb"), true, 6816, string("ubsjidpfkbdqzpgcxdnhmfvccknqjyaifmwqllkleorrwgnfnhbbobvldzpnkqlisimwzzxd"), 42196);
	this->vixfofabdf(false);
	this->qpsgeuzoojptukkozqlh(38606);
}
