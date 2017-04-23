#include "PluginSDK.h"
#include "Vector3.h"
#include <string>

IMenu* mainMenu;
IMenu* comboMenu;
IMenu* spellMenu;
IMenu* ksMenu;
IMenu* harassMenu;
IMenu* laneclearMenu;
IMenu* itemMenu;
IMenu* drawMenu;
IMenu* gapMenu;
IMenu* fleeMenu;
IMenu* interruptibleMenu;
IMenuOption* fleeKey;
IMenuOption* drawTarget;
IMenuOption* trapOnImobile;
IMenuOption* gapEnable;
IMenuOption* interruptEnable;
IMenuOption* qCombo;
IMenuOption* qKs;
IMenuOption* wCombo;
IMenuOption* eRange;
IMenuOption* eCombo;
IMenuOption* useR;
IMenuOption* rRange;
IMenuOption* qOutRange;
IMenuOption* wCastDelay;
IMenuOption* youmuuItem;
IMenuOption* bilgewaterItem;
IMenuOption* botrkItem;
IMenuOption* quicksilverItem;
IMenuOption* mercurialItem;
IMenuOption* humanizerItem;

IInventoryItem* Youmuu;
IInventoryItem* Bilgewater;
IInventoryItem* BOTRK;
IInventoryItem* Quicksilver;
IInventoryItem* Mercurial;

IUnit* currTarget;
IUnit* myHero;

float lastTrap;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;

bool Can_W()
{
	return (GGame->Time() * 1000) - lastTrap > wCastDelay->GetInteger();
}

double Calcul_Q_Damage(IUnit* Target)
{
	int base[6] = { 0, 30, 70, 110, 150, 190 };
	int bonus[6] = { 0, 130, 140, 150, 160, 170 };
	int total = base[GEntityList->Player()->GetSpellLevel(kSlotQ)] + bonus[GEntityList->Player()->GetSpellLevel(kSlotQ)];
	
	AdvPredictionOutput predOut;
	Q->RunPrediction(Target, true, static_cast<eCollisionFlags> (kCollidesWithMinions | kCollidesWithHeroes), &predOut);
	if (predOut.HitChance == kHitChanceCollision)
		total = total*0.67;

	return GDamage->CalcPhysicalDamage(myHero, Target, total);
}

double Calcul_AA_Damage(IUnit* Target)
{
	return GDamage->CalcPhysicalDamage(myHero, Target, myHero->TotalPhysicalDamage());
}

double Calcul_R_Damage(IUnit* Target)
{
	int base[4] = {0, 250, 475, 700 };
	int bonus = 2 * myHero->BonusDamage();
	int total = base[GEntityList->Player()->GetSpellLevel(kSlotR)] + bonus;
	return GDamage->CalcPhysicalDamage(myHero, Target, total);
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
	lastTrap = GGame->Time();
	myHero = GEntityList->Player();

	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, true, kCollidesWithYasuoWall);
	Q->SetSkillshot(0.3, 70, 2000, 1250);

	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, false, kCollidesWithNothing);
	W->SetSkillshot(0.8, 70, FLT_MAX, 800);

	E = GPluginSDK->CreateSpell2(kSlotE, kLineCast, true, false, static_cast<eCollisionFlags> (kCollidesWithYasuoWall | kCollidesWithMinions | kCollidesWithHeroes));
	E->SetSkillshot(0.25, 80, 1600, 800);

	R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, true, false, kCollidesWithYasuoWall);

	Youmuu = GPluginSDK->CreateItemForId(3142, 0);
	Bilgewater = GPluginSDK->CreateItemForId(3144, 550);
	BOTRK = GPluginSDK->CreateItemForId(3153, 550);
	Quicksilver = GPluginSDK->CreateItemForId(3140, 0);
	Mercurial = GPluginSDK->CreateItemForId(3139, 600);
}

void Load_Menu()
{
	mainMenu = GPluginSDK->AddMenu("Caitlyn - Foot Breaker");
	comboMenu = mainMenu->AddMenu(">> Combo <<");
	{
		qCombo = comboMenu->CheckBox("Use (Q)", true);
		wCombo = comboMenu->CheckBox("Use (W)", true);
		eCombo = comboMenu->CheckBox("Use (E)", true);
	}
	spellMenu = mainMenu->AddMenu(">> Spells Settings <<");
	{
		qOutRange = spellMenu->CheckBox("Only (Q) when Target out of AA Range", true);
		wCastDelay = spellMenu->AddInteger("Delay Between each Trap (ms)", 0, 5000, 1500);
		trapOnImobile = spellMenu->CheckBox("Auto Trap on unit Snare/Stun", true);
		eRange = spellMenu->AddInteger("(E) max range", 200, 800, 800);
		useR = spellMenu->CheckBox("Auto Cast (R)", true);
		rRange = spellMenu->AddInteger("(R) minimum range", 0, 2000, 1100);
	}
	ksMenu = mainMenu->AddMenu(">> Kill Steal <<");
	{
		qKs = ksMenu->CheckBox("Use (Q)", true);
	}
	gapMenu = mainMenu->AddMenu(">> Gapcloser <<");
	{
		gapEnable = gapMenu->CheckBox("Use (E)", true);
	}
	interruptibleMenu = mainMenu->AddMenu(">> Interrupt Spells <<");
	{
		interruptEnable = interruptibleMenu->CheckBox("Use (W)", true);
	}
	itemMenu = mainMenu->AddMenu(">> Item <<");
	{
		youmuuItem = itemMenu->CheckBox("Youmuu's Ghostblade - Enable", true);
		bilgewaterItem = itemMenu->CheckBox("Bilgewater Cutlass - Enable", true);
		botrkItem = itemMenu->CheckBox("Blade of the Ruined Kinge - Enable", true);
		quicksilverItem = itemMenu->CheckBox("Quicksilver Sash - Enable", true);
		mercurialItem = itemMenu->CheckBox("Mercurial Scimitar - Enable", true);
		humanizerItem = itemMenu->AddInteger("Humanizer (ms)", 0, 1000, 300);

	}
	fleeMenu = mainMenu->AddMenu(">> Flee <<");
	{
		fleeKey = fleeMenu->AddKey("Flee Key", 0x54);
	}
	drawMenu = mainMenu->AddMenu(">> Draw <<");
	{
		drawTarget = drawMenu->CheckBox("Draw Target", true);
	}
}

void Cast_Item(IUnit* Target)
{
	if (GOrbwalking->GetOrbwalkingMode() != kModeCombo)
		return;

	if (youmuuItem->Enabled() && Youmuu->IsOwned() && Youmuu->IsReady())
	{
		Youmuu->CastOnPlayer();
	}

	if (GetDistance(myHero->GetPosition(), Target->GetPosition()) > 600)
		return;

	if (bilgewaterItem->Enabled() && Bilgewater->IsOwned() && Bilgewater->IsReady())
	{
		Bilgewater->CastOnTarget(Target);
	}
	if (botrkItem->Enabled() && BOTRK->IsOwned() && BOTRK->IsReady())
	{
		BOTRK->CastOnTarget(Target);
	}
}

void Combo()
{
	auto Target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, Q->Range());
	
	currTarget = Target;
	if (Target == nullptr || !Target->IsValidTarget())
		return;

	if (GOrbwalking->GetOrbwalkingMode() != kModeCombo)
		return;

	Cast_Item(Target);

	if (E->IsReady() && eCombo->Enabled())
	{
		E->CastOnTarget(Target, kHitChanceVeryHigh);
		return;
	}

	if (W->IsReady() && Can_W() && wCombo->Enabled() && (Calcul_AA_Damage(Target)*3) < Target->GetHealth())
	{
		W->CastOnTarget(Target, kHitChanceLow);
		return;
	}

	if (Q->IsReady() && qCombo->Enabled() && !qOutRange->Enabled())
	{
		Q->CastOnTarget(Target, kHitChanceHigh);
		return;
	}
	if (Q->IsReady() && qCombo->Enabled() && qOutRange->Enabled() && GetDistance(myHero->GetPosition(), Target->GetPosition()) > 900)
	{
		Q->CastOnTarget(Target, kHitChanceHigh);
		return;
	}
}

void KillSteal()
{
	if (!qKs->Enabled() || !Q->IsReady())
		return;

	for (auto unit : GEntityList->GetAllHeros(false, true))
	{
		if (unit != nullptr && unit->IsValidTarget() && GetDistance(unit->GetPosition(), myHero->GetPosition()) > 800 && Calcul_Q_Damage(unit) > unit->GetHealth())
		{
			AdvPredictionOutput Pred;
			Q->RunPrediction(unit, false, kCollidesWithNothing, &Pred);
			if (Pred.HitChance >= kHitChanceMedium)
			{
				Q->CastOnPosition(Pred.CastPosition);
				return;
			}
		}
	}
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

void Cast_R()
{
	if (!useR->Enabled() || !R->IsReady())
		return;

	int range[4] = { 0, 2000, 2500, 3000 };
	auto Target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, range[GEntityList->Player()->GetSpellLevel(kSlotR)]);
	if (Target == nullptr || !Target->IsValidTarget() || GetDistance(Target->GetPosition(), myHero->GetPosition()) < 1100)
		return;

	if (Calcul_R_Damage(Target) > Target->GetHealth() && GetDistance(Target->GetPosition(), myHero->GetPosition()) > rRange->GetInteger())
	{
		int collisionCheck = 0;
		for (auto unit : GEntityList->GetAllHeros(false, true))
		{
			if (unit != nullptr && unit->GetNetworkId() != Target->GetNetworkId())
			{
				if (Is_Inside_Rectangle(myHero->GetPosition(), Target->GetPosition(), unit->GetPosition(), 500))
					collisionCheck++;
			}
		}
		if (collisionCheck == 0)
			R->CastOnUnit(Target);
	}
}

void Render()
{
	if (drawTarget->Enabled() && currTarget != nullptr && currTarget->IsValidTarget())
	{
		Vec2 targetPos; Vec2 myPos;
		GGame->Projection(currTarget->GetPosition(), &targetPos); GGame->Projection(myHero->GetPosition(), &myPos);
		GRender->DrawLine(targetPos, myPos, Vec4(118, 255, 0, 255));
		GRender->DrawFilledCircle(currTarget->GetPosition(), Vec4(118,255,0,255), 20);
	}
}

void Gapcloser(GapCloserSpell const& Spell)
{
	if (gapEnable->Enabled() && E->IsReady() && Spell.Sender != nullptr && Spell.Sender->IsValidTarget() && Spell.Sender->IsEnemy(myHero) && GetDistance(Spell.EndPosition, myHero->GetPosition()) < 200)
	{
		if (GetDistance(Spell.StartPosition, myHero->GetPosition()) < E->Range())
			E->CastOnTarget(Spell.Sender, kHitChanceHigh);
	}
}

std::function<void()> Item_Delay = [&]() -> void {
	if (quicksilverItem->Enabled() && Quicksilver->IsOwned() && Quicksilver->IsReady())
	{
		Quicksilver->CastOnPlayer();
		return;
	}
	if (mercurialItem->Enabled() && Mercurial->IsOwned() && Mercurial->IsReady())
	{
		Mercurial->CastOnPlayer();
		return;
	}
};

void BuffAdd(IUnit* Source)
{
	if (Source->GetNetworkId() == myHero->GetNetworkId()
		&& (Source->HasBuffOfType(BUFF_Charm) || Source->HasBuffOfType(BUFF_Fear) || Source->HasBuffOfType(BUFF_Snare)
			|| Source->HasBuffOfType(BUFF_Stun) || Source->HasBuffOfType(BUFF_Taunt)))
	{
		GPluginSDK->DelayFunctionCall(humanizerItem->GetInteger(), Item_Delay);
	}

	if (trapOnImobile->Enabled() && Source != nullptr && Source->IsValidTarget() && Source->IsHero() && Source->IsEnemy(myHero) && GetDistance(Source->GetPosition(), myHero->GetPosition()) < W->Range() &&
		(Source->HasBuffOfType(BUFF_Taunt) || Source->HasBuffOfType(BUFF_Stun) || Source->HasBuffOfType(BUFF_Snare)))
	{
		W->CastOnPosition(Source->GetPosition());
	}
	if (Source == myHero && myHero->HasBuff("caitlynheadshotrangecheck"))
	{
		GOrbwalking->ResetAA();
		GGame->PrintChat("Reset AA CD");
	}
}

void Interruptible(InterruptibleSpell const& Spell)
{
	if (interruptEnable->Enabled() && W->IsReady() && Spell.Target->IsValidTarget() && GetDistance(myHero->GetPosition(), Spell.Target->GetPosition()) < W->Range())
	{
		AdvPredictionOutput Pred;
		W->RunPrediction(Spell.Target, false, kCollidesWithNothing, &Pred);
		if (Pred.HitChance >= kHitChanceMedium)
		{
			W->CastOnPosition(Pred.CastPosition);
			return;
		}
	}
}

void SpellCast(CastedSpell const& Spell)
{
	if (Spell.Caster_ == myHero && std::string(Spell.Name_) == "CaitlynYordleTrap")
		lastTrap = GGame->Time() * 1000;
}

void Load_Flee()
{
	if (GetAsyncKeyState(fleeKey->GetInteger()) >= 0)
		return;

	GGame->IssueOrder(myHero, kMoveTo, GGame->CursorPosition());

	if (E->IsReady())
	{
		Vec3 vector = myHero->GetPosition() - (GGame->CursorPosition() - myHero->GetPosition()).VectorNormalize() * 300;
		E->CastOnPosition(vector);
	}
}

void GameUpdate()
{
	Combo();
	KillSteal();
	Cast_R();
	E->SetOverrideRange(eRange->GetInteger());
	Load_Flee();
}

void OrbwalkAfterAttack(IUnit* From, IUnit* To)
{
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Load_Menu();
	Load_Variable();
	GGame->PrintChat("<font color=\"#009688\"><b>Caitlyn Foot Breaker</font><font color=\"#4DB6AC\"> is successfuly loaded</b></font>");
	GEventManager->AddEventHandler(kEventOnBuffAdd, BuffAdd);
	GEventManager->AddEventHandler(kEventOnGameUpdate, GameUpdate);
	GEventManager->AddEventHandler(kEventOnRender, Render);
	GEventManager->AddEventHandler(kEventOrbwalkAfterAttack, OrbwalkAfterAttack);
	GEventManager->AddEventHandler(kEventOnSpellCast, SpellCast);
	GEventManager->AddEventHandler(kEventOnGapCloser, Gapcloser);
	GEventManager->AddEventHandler(kEventOnInterruptible, Interruptible);
}

PLUGIN_API void OnUnload()
{
	mainMenu->SaveSettings();
	mainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, GameUpdate);
	GEventManager->RemoveEventHandler(kEventOnBuffAdd, BuffAdd);
	GEventManager->RemoveEventHandler(kEventOnRender, Render);
	GEventManager->RemoveEventHandler(kEventOrbwalkAfterAttack, OrbwalkAfterAttack);
	GEventManager->RemoveEventHandler(kEventOnSpellCast, SpellCast);
	GEventManager->RemoveEventHandler(kEventOnGapCloser, Gapcloser);
	GEventManager->RemoveEventHandler(kEventOnInterruptible, Interruptible);
}