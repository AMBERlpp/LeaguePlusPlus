#include "PluginSDK.h"
#include "Vector3.h"
#include <string>

/// MySelf
IUnit* myHero;
Vec3 myPos;

// Ball
Vec3 ballPos;
IUnit* ballMissile;
bool ballFly;

// Spells
ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;

IMenu* mainMenu;
// Menu Combo
IMenu* Combo;
IMenuOption* rHit;
IMenuOption* rForce;
IMenuOption* rLogic;
IMenuOption* wHit;
IMenuOption* wForce;
IMenuOption* eHit;
IMenuOption* eForce;
IMenuOption* qCombo;
IMenuOption* wCombo;
IMenuOption* eCombo;
IMenuOption* rCombo;
//Menu Harass
IMenu* Harass;
IMenuOption* qHarass;
IMenuOption* wHarass;
IMenuOption* eHarass;
IMenuOption* qHarassMana;
IMenuOption* wHarassMana;
IMenuOption* eHarassMana;

double Calcul_W_Damage(IUnit* Target)
{
	int base[6] = {0, 70, 115, 160, 205, 250};
	int bonus = 0.7 * myHero->TotalMagicDamage();
	int total = base[GEntityList->Player()->GetSpellLevel(kSlotW)] + bonus;
	return GDamage->CalcMagicDamage(myHero, Target, total);
}

double Calcul_R_Damage(IUnit* Target)
{
	int base[4] = {0, 125, 225, 300};
	int bonus = 0.7 * myHero->TotalMagicDamage();
	int total = base[GEntityList->Player()->GetSpellLevel(kSlotR)] + bonus;
	return GDamage->CalcMagicDamage(myHero, Target, total);
}

double Calcul_Q_Damage(IUnit* Target)
{
	int base[6] = { 0, 60, 90, 120, 150, 180 };
	int bonus = 0.5 * myHero->TotalMagicDamage();
	int total = base[GEntityList->Player()->GetSpellLevel(kSlotQ)] + bonus;
	return GDamage->CalcMagicDamage(myHero, Target, total);
}

Vec2 vect2d(Vec2 p1, Vec2 p2)
{
	Vec2 temp;
	temp.x = (p2.x - p1.x);
	temp.y = -1 * (p2.y - p1.y);
	return temp;
}

bool pointInRectangle(Vec2 A, Vec2 B, Vec2 C, Vec2 D, Vec2 m)
{
	Vec2 AB = vect2d(A, B);  float C1 = -1 * (AB.y*A.x + AB.x*A.y); float  D1 = (AB.y*m.x + AB.x*m.y) + C1;
	Vec2 AD = vect2d(A, D);  float C2 = -1 * (AD.y*A.x + AD.x*A.y); float D2 = (AD.y*m.x + AD.x*m.y) + C2;
	Vec2 BC = vect2d(B, C);  float C3 = -1 * (BC.y*B.x + BC.x*B.y); float D3 = (BC.y*m.x + BC.x*m.y) + C3;
	Vec2 CD = vect2d(C, D);  float C4 = -1 * (CD.y*C.x + CD.x*C.y); float D4 = (CD.y*m.x + CD.x*m.y) + C4;
	return     0 >= D1 && 0 >= D4 && 0 <= D2 && 0 >= D3;
}

bool Is_Inside_Rectangle(Vec3 From, Vec3 To, Vec3 pointTest, int radius)
{
	int mediumRadius = radius / 2;
	Vec2 pointTest2 = pointTest.To2D();
	Vec3 A = From + (From - To).VectorNormalize().Perpendicular() * (-mediumRadius); Vec2 A1 = A.To2D();
	Vec3 B = From + (From - To).VectorNormalize().Perpendicular() * (mediumRadius); Vec2 B1 = B.To2D();
	Vec3 C = To + (To - From).VectorNormalize().Perpendicular() * (-mediumRadius); Vec2 C1 = C.To2D();
	Vec3 D = To + (To - From).VectorNormalize().Perpendicular() * (mediumRadius); Vec2 D1 = D.To2D();
	return pointInRectangle(A1, B1, C1, D1, pointTest2);
}

int GetDistance(Vec3 From, Vec3 To)
{
	float x1 = From.x;
	float y1 = From.y;
	float z1 = From.z;
	float x2 = To.x;
	float y2 = To.y;
	float z2 = To.z;
	return static_cast<int>(sqrt(pow((x2 - x1), 2.0) + pow((y2 - y1), 2.0) + pow((z2 - z1), 2.0)));
}

void Load_Variable()
{
	myHero = GEntityList->Player();

	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, true, kCollidesWithYasuoWall);
	Q->SetSkillshot(0.f, 100.f, 1000.f, 800.f);
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, true, kCollidesWithNothing);
	W->SetSkillshot(0.3f, 250.f, HUGE_VAL, 800.f);
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, true, false, kCollidesWithNothing);
	E->SetSkillshot(0.f, 0.f, 1800.f, 1050.f);
	R = GPluginSDK->CreateSpell2(kSlotR, kCircleCast, false, true, kCollidesWithNothing);
	R->SetSkillshot(0.5f, 350, HUGE_VAL, 800.f);
}

void Load_Menu()
{
	mainMenu =  GPluginSDK->AddMenu("Orianna - Crazy Ballz");
	Combo = mainMenu->AddMenu(">> Combo <<");
	{
		qCombo = Combo->CheckBox("Use (Q)", true);
		wCombo = Combo->CheckBox("Use (W)", true);
		eCombo = Combo->CheckBox("Use (E)", true);
		rCombo = Combo->CheckBox("Use (R)", true);
		wHit = Combo->AddInteger("Minimum (W) Hit to Cast", 1, 5, 3);
		wForce = Combo->CheckBox("Force (W) if hit all unit present", true);
		eHit = Combo->AddInteger("Minimum (E) Hit to Cast", 1, 5, 2);
		eForce = Combo->CheckBox("Force (E) if hit all unit present", true);
		rHit = Combo->AddInteger("Minimum (R) Hit to Cast", 1, 5, 3);
		rForce = Combo->CheckBox("Force (R) if hit all unit present", true);
		rLogic = Combo->CheckBox("Added (R) Cast Logic to not waste it", true);
	}
	Harass = mainMenu->AddMenu(">> Harass <<");
	{
		qHarass = Harass->CheckBox("Use (Q)", true);
		qHarassMana = Harass->AddInteger("Min mana percent to use", 0, 100, 70);
		wHarass = Harass->CheckBox("Use (W)", true);
		wHarassMana = Harass->AddInteger("Min mana percent to use", 0, 100, 70);
		eHarass = Harass->CheckBox("Use (E)", true);
		eHarassMana = Harass->AddInteger("Min mana percent to use", 0, 100, 70);
	}
}

void Update_Location()
{
	myPos = myHero->GetPosition();
	Q->SetFrom(ballPos);
	W->SetFrom(ballPos);
	E->SetFrom(ballPos);
	R->SetFrom(ballPos);

	if (ballMissile != nullptr && ballMissile->IsValidObject())
	{
		ballPos = ballMissile->GetPosition();
		return;
	}

	for (auto unit : GEntityList->GetAllHeros(true, false))
	{
		if (unit->HasBuff("orianaghost") || unit->HasBuff("orianaghostself"))
		{
			ballPos = unit->GetPosition();
			return;
		}
	}
}

int Count_Enemy_In_Range(int range)
{
	int unitCount = 0;
	for (auto unit : GEntityList->GetAllHeros(false, true))
	{
		if (unit->IsValidTarget())
		{
			if (GetDistance(myPos, unit->GetPosition()) < range)
				unitCount++;
		}
	}
	return unitCount;
}

int Count_Enemy_Hit_From_Ball(int range, int delay, Vec3 from)
{
	int unitHit = 0;
	for (auto unit : GEntityList->GetAllHeros(false, true))
	{
		if (unit->IsValidTarget())
		{
			Vec3 predPos;
			GPrediction->GetFutureUnitPosition(unit, delay, true, predPos);
			if (GetDistance(predPos, from) < range)
				unitHit++;
		}
	}
	return unitHit;
}

int Count_Enemy_Hit_From_Ball_R(int range, int delay, Vec3 from)
{
	int unitHit = 0;
	IUnit* isUnit = nullptr;
	for (auto unit : GEntityList->GetAllHeros(false, true))
	{
		if (unit->IsValidTarget())
		{
			Vec3 predPos;
			GPrediction->GetFutureUnitPosition(unit, delay, true, predPos);
			if (GetDistance(predPos, from) < range)
			{
				unitHit++;
				isUnit = unit;
			}
		}
	}
	if (unitHit == 1 && rLogic->Enabled() && (Calcul_R_Damage(isUnit) + Calcul_W_Damage(isUnit) + Calcul_Q_Damage(isUnit)) < isUnit->GetHealth())
		unitHit = 0;

	return unitHit;
}

std::vector<IUnit*> Get_Unit_In_R()
{
	std::vector<IUnit*> unitArray;

	for (auto unit : GEntityList->GetAllHeros(false, true))
	{
		if (unit->IsValidTarget())
		{
			Vec3 predPos;
			GPrediction->GetFutureUnitPosition(unit, R->GetDelay(), true, predPos);
			if (GetDistance(predPos, ballPos) < R->Radius())
				unitArray.push_back(unit);
		}
	}
	return unitArray;
}

void Cast_R()
{
	std::vector<IUnit*> unitHit = Get_Unit_In_R();
	int hit = unitHit.size();
	for (auto unit : unitHit)
	{

	}
}

void Cast_E_To_Best_Ally()
{
	for (auto ally : GEntityList->GetAllHeros(true, false))
	{
		int distance = GetDistance(ally->GetPosition(), myPos);
		if (distance < E->Range())
		{
			Vec3 predPos;
			double Delay = distance / E->Speed();
			GPrediction->GetFutureUnitPosition(ally, Delay, true, predPos);
			int unitHit = Count_Enemy_Hit_From_Ball_R(R->Radius(), Delay, ally->GetPosition());
			int unitPresent = Count_Enemy_In_Range(3000);
			if (R->IsReady() && ((unitHit == unitPresent && rForce->Enabled()) || unitHit >= rHit->GetInteger()))
				E->CastOnUnit(ally);
		}
	}
}

void Cast_E_To_Damage()
{
	if (!E->IsReady())
		return;

	int maxHit = 0;
	IUnit* isAlly = nullptr;
	for (auto ally : GEntityList->GetAllHeros(true,false))
	{
		if (GetDistance(ally->GetPosition(), myPos) < E->Range())
		{
			int currHit = 0;
			for (auto unit : GEntityList->GetAllHeros(false, true))
			{
				if (Is_Inside_Rectangle(ally->GetPosition(), ballPos, unit->GetPosition(), E->Radius() + (unit->BoundingRadius()*2)))
					currHit++;
			}
			if (currHit > maxHit)
			{
				maxHit = currHit;
				isAlly = ally;
			}
		}
	} 

	int unitPresent = Count_Enemy_In_Range(3000);
	if (isAlly != nullptr && (maxHit == unitPresent && eForce->Enabled()) || maxHit >= eHit->GetInteger())
		E->CastOnUnit(isAlly);
}

void Load_Combo()
{
	if (GOrbwalking->GetOrbwalkingMode() != kModeCombo)
		return;

	Vec3 predPos; int maxHit;
	if (R->IsReady() && rCombo->Enabled())
	{
		if(E->IsReady() && eCombo->Enabled())
			Cast_E_To_Best_Ally();

		GPrediction->FindBestCastPosition(Q->Range(), R->Radius(), false, false, true, predPos, maxHit);
		if(maxHit > 0 && GetDistance(predPos, myPos) < Q->Range())
			Q->CastOnPosition(predPos);
	}
	else if (W->IsReady() && wCombo->Enabled())
	{
		GPrediction->FindBestCastPosition(Q->Range(), W->Radius(), false, false, true, predPos, maxHit);
		if (maxHit > 0 && GetDistance(predPos, myPos) < Q->Range())
			Q->CastOnPosition(predPos);
	}
	else
	{
		auto Target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
		if (Q->IsReady() && qCombo->Enabled() && Target != nullptr && Target->IsValidTarget())
			Q->CastOnTarget(Target, kHitChanceHigh);
	}

	if (!R->IsReady() || !rCombo->Enabled())
	{
		Cast_E_To_Damage();
	}

	int unitPresent = Count_Enemy_In_Range(3000);
	int rUnitHit = Count_Enemy_Hit_From_Ball_R(R->Radius(), R->GetDelay(), ballPos);
	int wUnitHit = Count_Enemy_Hit_From_Ball(W->Radius(), W->GetDelay(), ballPos);
	if (R->IsReady() && ((rUnitHit == unitPresent && rForce->Enabled() && unitPresent > 0) || rUnitHit >= rHit->GetInteger()))
		R->CastOnPlayer();
	if (W->IsReady() && ((wUnitHit == unitPresent && wForce->Enabled() && unitPresent > 0) || wUnitHit >= wHit->GetInteger()))
		W->CastOnPlayer();
}

void Load_Harass()
{
	if (GOrbwalking->GetOrbwalkingMode() != kModeMixed)
		return;

	auto Target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
	if (Q->IsReady() && qHarass->Enabled() && Target != nullptr && Target->IsValidTarget() && (float)qHarassMana->GetInteger() < myHero->ManaPercent())
		Q->CastOnTarget(Target, kHitChanceHigh);

	int wUnitHit = Count_Enemy_Hit_From_Ball(W->Radius(), W->GetDelay(), ballPos);
	if (W->IsReady() && wHarass->Enabled() && wUnitHit > 0 && (float)wHarassMana->GetInteger() < myHero->ManaPercent())
		W->CastOnPlayer();

	if (eHarass->Enabled() && (float)eHarassMana->GetInteger() < myHero->ManaPercent())
		Cast_E_To_Damage();
}

void OnRender()
{
	Vec2 startPos; Vec2 endPos; GGame->Projection(myPos, &startPos);  GGame->Projection(ballPos, &endPos);
	GRender->DrawFilledCircle(ballPos, Vec4(33, 150, 243, 255), 30);
	GRender->DrawCircle(ballPos, 80, Vec4(33, 150, 243, 255), 10.f);
	GRender->DrawLine(startPos, endPos, Vec4(33, 150, 243, 255));
}

void OnCreateObject(IUnit* Object)
{
	if (strstr(Object->GetObjectName(), "missile") && ballFly)
	{
		ballMissile = Object;
		ballFly = false;
	}
	if (strstr(Object->GetObjectName(), "TheDoomBall"))
	{
		ballPos = Object->GetPosition();
	}
}

void GameUpdate()
{
	Load_Variable();
	Update_Location();
	Load_Combo();
	Load_Harass();
}

void SpellCast(CastedSpell const& Spell)
{
	if (Spell.Caster_ == myHero && std::string(Spell.Name_) == "OrianaIzunaCommand")
	{
		ballFly = true;
	}
	if (Spell.Caster_ == myHero && std::string(Spell.Name_) == "OrianaRedactCommand" && GetDistance(ballPos, Spell.EndPosition_) > 50)
	{
		ballFly = true;
	}
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	GGame->PrintChat("<font color=\"#009688\"><b>Orianna Crazy Ballz</font><font color=\"#4DB6AC\"> is successfuly loaded</b></font>");
	
	Load_Variable();
	Load_Menu();

	GEventManager->AddEventHandler(kEventOnGameUpdate, GameUpdate);
	GEventManager->AddEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);
	GEventManager->AddEventHandler(kEventOnSpellCast, SpellCast);
}

PLUGIN_API void OnUnload()
{
	mainMenu->SaveSettings();
	mainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, GameUpdate);
	GEventManager->RemoveEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnSpellCast, SpellCast);
}