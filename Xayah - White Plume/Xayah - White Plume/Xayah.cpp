#include <string>
#include "PluginSDK.h"
#include "Vector3.h"
#include "SpellDatabase.h"

struct SPlume {
	IUnit* Plume;
	double startTime;
};
std::vector<SPlume> VPlume;
std::map<std::string, IMenuOption*> MenuOptions;

IUnit* myHero;
IMenu* rMenu;
IMenu* mainMenu;
IMenu* Draw;
IMenu* Combo;
IMenu* JungleClear;
IMenu* itemMenu;
IMenu* KillSteal;
IMenu* LaneClear;
IMenuOption* eKill;
IMenuOption* qSteal;
IMenuOption* rEnable;
IMenuOption* eSteal;
IMenuOption* qeSteal;
IMenuOption* jungleEnable;
IMenuOption* jungleQ;
IMenuOption* jungleW;
IMenuOption* jungleMana;
IMenuOption* jungleDraw;
IMenuOption* qCombo;
IMenuOption* qType;
IMenuOption* qRange;
IMenuOption* wCombo;
IMenuOption* wRange;
IMenuOption* eCombo;
IMenuOption* eHit;
IMenuOption* eKillLane;
IMenuOption* eRoot;
IMenuOption* drawPlume;
IMenuOption* drawQ;
IMenuOption* timePlume;
IMenuOption* dmgPlume;
IMenuOption* youmuuItem;
IMenuOption* bilgewaterItem;
IMenuOption* botrkItem;
IMenuOption* quicksilverItem;
IMenuOption* mercurialItem;
IMenuOption* humanizerItem;
IMenuOption* eLaneClear;
IMenuOption* qLaneClear;
IMenuOption* qHitLane;
IMenuOption* LaneClearMana;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;
ISpell2* tempSpell;

IInventoryItem* Youmuu;
IInventoryItem* Bilgewater;
IInventoryItem* BOTRK;
IInventoryItem* Quicksilver;
IInventoryItem* Mercurial;

void AddCheckBox(std::string name, std::string title, bool value)
{
	MenuOptions[name] = rMenu->CheckBox(title.c_str(), value);
}

bool GetMenuBoolean(std::string name)
{
	return MenuOptions[name]->Enabled();
}

void Load_Menu()
{
	mainMenu = GPluginSDK->AddMenu("Xayah - White Plume");
	Combo = mainMenu->AddMenu(">> Combo <<");
	{
		qCombo = Combo->CheckBox("Use (Q)", true);
		wCombo = Combo->CheckBox("Use (W)", true);
		eCombo = Combo->CheckBox("Use (E)", true);
		qRange = Combo->AddInteger("(Q) Range: Maximum", 300, 1050, 1050);
		qType = Combo->AddSelection("(Q): Type", 0, { "After AA", "Direct" });
		wRange = Combo->AddInteger("(W) Range: Maximum", 300, 1100, 650);
		eHit = Combo->AddInteger("(E) Cast: Min Number Feathers hit Target", 1, 10, 5);
		eRoot = Combo->AddInteger("(E) Cast: Min Number Enemy Rootable", 1, 5, 2);
		eKill = Combo->AddInteger("(E) Cast: Min Number Enemy Killable", 1, 5, 1);
	}
	rMenu = mainMenu->AddMenu(">> Ultimate <<");
	{
		rEnable = rMenu->CheckBox("Use (R) dodge important Spell", true);
		for (auto unit : GEntityList->GetAllHeros(false, true))
		{
			for (auto champion : SpellDatabase::Champions)
			{
				if (std::string(champion.second.DisplayName) == std::string(unit->ChampionName()))
				{
					auto spells = champion.second.Spells;
					for (auto Spell2 : spells)
					{		
						std::string Content = std::string(champion.second.DisplayName) + " " + SpellDatabase::Slots[Spell2.Slot] + ": " + std::string(Spell2.Name);
						AddCheckBox(std::string(Spell2.Name), Content, true);
					}
				}
			}
		}
	}
	KillSteal = mainMenu->AddMenu(">> Kill Steal <<");
	{
		qSteal = KillSteal->CheckBox("Use (Q)", true);
		eSteal = KillSteal->CheckBox("Use (Q)", true);
		qeSteal = KillSteal->CheckBox("Check for (Q) + (E)", true);
	}
	LaneClear = mainMenu->AddMenu(">> Lane Clear <<");
	{
		LaneClearMana = LaneClear->AddInteger("Mana: Min % to use spells", 0, 100, 60);
		qLaneClear = LaneClear->CheckBox("Use (Q)", true);
		qHitLane = LaneClear->AddInteger("(Q) Cast: Min Number Minions Hit", 1, 10, 3);
		eLaneClear = LaneClear->CheckBox("Use (E)", true);
		eKillLane = LaneClear->AddInteger("(E) Cast: Min Number Minions Killable", 1, 10, 3);
	}
	JungleClear = mainMenu->AddMenu(">> Jungle Clear <<");
	{
		jungleQ = JungleClear->CheckBox("Use (Q)", true);
		jungleW = JungleClear->CheckBox("Use (W)", true);
		jungleMana = JungleClear->AddInteger("Minimum Mana", 0, 100, 60);
		jungleEnable = JungleClear->CheckBox("Use (E) to kill big mobs", true);
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
	Draw = mainMenu->AddMenu(">> Draw <<");
	{
		drawQ = Draw->CheckBox("Draw (Q) Range", true);
		drawPlume = Draw->AddSelection("Feather: Draw Type", 1, { "snone", "line", "circle" });
		timePlume = Draw->CheckBox("Feather: Draw Timer", true);
		dmgPlume = Draw->CheckBox("Feather: Draw Damage Unit", true);
		jungleDraw = Draw->CheckBox("Feather: Draw Damage Jungle", true);
	}
}

void Load_Variable()
{
	myHero = GEntityList->Player();

	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, true, kCollidesWithYasuoWall);
	Q->SetSkillshot(0.f, 120.f, 1200, 1000.f);
	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, false, kCollidesWithNothing);
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, false, false, kCollidesWithNothing);
	R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, false, false, kCollidesWithNothing);
	tempSpell = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, false, static_cast<eCollisionFlags> (kCollidesWithYasuoWall | kCollidesWithMinions | kCollidesWithHeroes));
	tempSpell->SetSkillshot(0.f, 120.f, 1200, 1000.f);

	Youmuu = GPluginSDK->CreateItemForId(3142, 0);
	Bilgewater = GPluginSDK->CreateItemForId(3144, 550);
	BOTRK = GPluginSDK->CreateItemForId(3153, 550);
	Quicksilver = GPluginSDK->CreateItemForId(3140, 0);
	Mercurial = GPluginSDK->CreateItemForId(3139, 600);
}

bool Is_Big_Mob(IUnit* minion)
{
	return (strstr(minion->GetObjectName(), "SRU_Blue1") || strstr(minion->GetObjectName(), "SRU_Blue7")
		|| strstr(minion->GetObjectName(), "SRU_Gromp14") || strstr(minion->GetObjectName(), "SRU_Gromp13")
		|| strstr(minion->GetObjectName(), "SRU_Murkwolf8") || strstr(minion->GetObjectName(), "SRU_Murkwolf2")
		|| strstr(minion->GetObjectName(), "SRU_Razorbeak3") || strstr(minion->GetObjectName(), "SRU_Razorbeak9")
		|| strstr(minion->GetObjectName(), "SRU_Red4") || strstr(minion->GetObjectName(), "SRU_Red10")
		|| strstr(minion->GetObjectName(), "SRU_Krug11") || strstr(minion->GetObjectName(), "SRU_Krug5")
		|| strstr(minion->GetObjectName(), "Dragon") || strstr(minion->GetObjectName(), "Baron") 
		|| strstr(minion->GetObjectName(), "RiftHerald") || strstr(minion->GetObjectName(), "SRU_Krab"));
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

double Calcul_E_Damage(IUnit* Target)
{
	int eLevel = GEntityList->Player()->GetSpellLevel(kSlotE);
	int base[6] = { 0, 50, 60, 70, 80, 90 };
	int bonus = 0.60 * myHero->BonusDamage();
	int crit[6] = { 0, 0.5, 0.6, 0.7, 0.8, 0.9 };
	int bonusCrit = myHero->Crit() * 100 * crit[eLevel]; 
	int total = base[eLevel] + bonus + bonusCrit;
	return GDamage->CalcPhysicalDamage(myHero, Target, total);
}

double Calcul_Q_Damage(IUnit* Target)
{
	AdvPredictionOutput predOut;
	tempSpell->RunPrediction(Target, false, static_cast<eCollisionFlags> (kCollidesWithMinions | kCollidesWithHeroes), &predOut);
	int base[6] = { 0, 80, 120, 160, 200, 240 };
	double bonus = myHero->BonusDamage();
	double total = GDamage->CalcPhysicalDamage(myHero, Target, base[GEntityList->Player()->GetSpellLevel(kSlotQ)] + bonus);
	
	if (predOut.HitChance == kHitChanceCollision)
		total = total*0.5;

	return total;
}

double Calcul_Multiple_E_Damage(IUnit* Target, int Number)
{
	if (Number == 0) return 0;

	int damage = Calcul_E_Damage(Target);
	int total = Calcul_E_Damage(Target);
	for (int i = 1; i < Number; i++)
	{
		double diviser = 1 -(i / 10);
		if (diviser < 0.10) diviser = 0.10;
		total = total + damage*diviser;
	}
	return total;
}

int Count_E_Hit(IUnit* Target)
{
	int nbr = 0;
	for (auto data : VPlume)
	{
		if (Is_Inside_Rectangle(data.Plume->GetPosition(), myHero->GetPosition(), Target->GetPosition(), (40 + Target->BoundingRadius() * 2)))
			nbr++;
	}
	return nbr;
}

int Count_Enemy_Root()
{
	int nbr = 0;
	for (auto unit : GEntityList->GetAllHeros(false, true))
	{
		if (unit != nullptr && unit->IsValidTarget() && Count_E_Hit(unit) >= 3)
			nbr++;
	}
	return nbr;
}

int Count_Enemy_Killable()
{
	int nbr = 0;
	for (auto unit : GEntityList->GetAllHeros(false, true))
	{
		if (unit != nullptr && unit->IsValidTarget())
		{
			int dmg = Calcul_Multiple_E_Damage(unit, Count_E_Hit(unit));
			if (dmg > unit->GetHealth())
				nbr++;
		}
	}
	return nbr;
}

int Count_Minion_Killable()
{
	int nbr = 0;
	for (auto unit : GEntityList->GetAllMinions(false, true, false))
	{
		if (unit != nullptr && unit->IsValidTarget())
		{
			int dmg = Calcul_Multiple_E_Damage(unit, Count_E_Hit(unit))/2;
			if (dmg > unit->GetHealth())
				nbr++;
		}
	}
	return nbr;
}

void Cast_Item(IUnit* Target)
{
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

void Load_Combo()
{
	if (GOrbwalking->GetOrbwalkingMode() != kModeCombo)
		return;

	if (Count_Enemy_Root() >= eRoot->GetInteger())
		E->CastOnPlayer();

	if (Count_Enemy_Killable() >= eKill->GetInteger())
		E->CastOnPlayer();

	IUnit* Target = GOrbwalking->GetLastTarget();

	if(Target == nullptr || !Target->IsValidTarget())
		Target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, Q->Range());
	
	if (Target == nullptr || !Target->IsValidTarget())
		return;

	Cast_Item(Target);

	if (Q->IsReady() && GetDistance(Target->GetPosition(), myHero->GetPosition()) < qRange->GetInteger() && qType->GetInteger() == 1)
		Q->CastOnTarget(Target, kHitChanceHigh);

	if (W->IsReady() && GetDistance(Target->GetPosition(), myHero->GetPosition()) < wRange->GetInteger())
		W->CastOnPlayer();

	if (E->IsReady() && Target->IsValidTarget() && Count_E_Hit(Target) >= eHit->GetInteger())
		E->CastOnPlayer();

}

void Update_Plume()
{
	std::vector<SPlume>::iterator it;
	for (it = VPlume.begin(); it < VPlume.end(); it++)
	{
		if ((it)->Plume->IsDead())
			VPlume.erase(it);
	}
}

void Load_JungleClear()
{
	for (auto minion : GEntityList->GetAllMinions(false, false, true))
	{
		if (minion != nullptr && minion->IsValidTarget() && Is_Big_Mob(minion))
		{
			if (jungleEnable->Enabled())
			{
				int Hit = Count_E_Hit(minion);
				double Damage = Calcul_Multiple_E_Damage(minion, Hit);
				int Percentage = (Damage * 100) / minion->GetHealth();
				if (Damage >= minion->GetHealth())
					E->CastOnPlayer();
			}

			if (myHero->ManaPercent() >= (float)jungleMana->GetInteger() && GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
			{
				if (jungleQ->Enabled() && Q->IsReady() && GetDistance(minion->GetPosition(), myHero->GetPosition()) < Q->Range())
					Q->CastOnTarget(minion, kHitChanceMedium);
				if (jungleW->Enabled() && W->IsReady() && GetDistance(minion->GetPosition(), myHero->GetPosition()) < 650)
					W->CastOnPlayer();
			}
		}
	}
}

void Load_LaneClear()
{
	if (GOrbwalking->GetOrbwalkingMode() != kModeLaneClear || myHero->ManaPercent() < LaneClearMana->Enabled())
		return;

	if (eLaneClear->Enabled() && E->IsReady() && Count_Minion_Killable() > eKillLane->GetInteger())
		E->CastOnPlayer();

	if (qLaneClear->Enabled())
	{
		Vec3 predPos; int Hit;
		Q->FindBestCastPosition(true, true, predPos, Hit);
		if (Hit > qHitLane->GetInteger() && GetDistance(predPos, myHero->GetPosition()) < Q->Range())
			Q->CastOnPosition(predPos);
	}

}

void Load_KillSteal()
{
	for (auto unit : GEntityList->GetAllHeros(false, true))
	{
		if (unit != nullptr && unit->IsValidTarget())
		{
			int eHit = Count_E_Hit(unit);
			if (E->IsReady() && eSteal->Enabled() && Calcul_Multiple_E_Damage(unit, eHit) > unit->GetHealth())
				E->CastOnPlayer();

			if (GetDistance(unit->GetPosition(), myHero->GetPosition()) < Q->Range() + 100)
			{
				if (Q->IsReady() && qSteal->Enabled() && Calcul_Q_Damage(unit) > unit->GetHealth())
				{
					Q->CastOnTarget(unit, kHitChanceHigh);
				}
				if (Q->IsReady() && E->IsReady() && qSteal->Enabled() && eSteal->Enabled() && qeSteal->Enabled())
				{
					double dmg = Calcul_Q_Damage(unit) + Calcul_Multiple_E_Damage(unit, eHit + 2);
					if(dmg > unit->GetHealth())
						Q->CastOnTarget(unit, kHitChanceHigh);
				}
			}
		}
	}
}

void GameUpdate()
{
	Update_Plume();
	Load_Combo();
	Load_JungleClear();
	Load_LaneClear();
	Load_KillSteal();
}

void Render()
{
	if(drawQ->Enabled())
		GRender->DrawCircle(myHero->GetPosition(), qRange->GetInteger(), Vec4(0, 77, 64, 255));
	if (drawPlume->GetInteger() == 1)
	{
		for (auto data : VPlume)
		{
			Vec3 C = data.Plume->GetPosition() + (data.Plume->GetPosition() - myHero->GetPosition()).VectorNormalize().Perpendicular() * (-35);
			Vec3 D = data.Plume->GetPosition() + (data.Plume->GetPosition() - myHero->GetPosition()).VectorNormalize().Perpendicular() * (35);
			Vec2 A2; Vec2 B2; Vec2 C2; Vec2 D2; Vec2 From; GGame->Projection(C, &C2); GGame->Projection(D, &D2); GGame->Projection(myHero->GetPosition(), &From);
			GRender->DrawLine(From, D2, Vec4(0, 150, 136, 255));
			GRender->DrawLine(From, C2, Vec4(0, 150, 136, 255));
			GRender->DrawLine(C2, D2, Vec4(0, 150, 136, 255));
		}
	} 
	else if(drawPlume->GetInteger() == 2)
	{
		for (auto data : VPlume)
		{
			GRender->DrawCircle(data.Plume->GetPosition(), 50, Vec4(0, 150, 136, 255));
		}
	}

	if (timePlume->Enabled())
	{
		for (auto data : VPlume)
		{
			float Delay = 6.5 - (GGame->Time() - data.startTime);
			Vec3 tempPos = data.Plume->GetPosition();
			Vec2 textPos; GGame->Projection(tempPos, &textPos);
			GRender->DrawTextW(textPos, Vec4(255, 255, 255, 255), "%.2f", Delay);
		}
	}

	if (dmgPlume->Enabled())
	{
		for (auto unit : GEntityList->GetAllHeros(false, true))
		{
			if (unit != nullptr && unit->IsValidTarget())
			{
				int Hit = Count_E_Hit(unit);
				double Dmg = Calcul_Multiple_E_Damage(unit, Hit);
				double percent = Dmg * 100 / unit->GetHealth();
				Vec2 textPos; unit->GetHPBarPosition(textPos);
				std::string temp = "%.2f%% - " + std::to_string(Hit) + "hit";
				GRender->DrawTextW(textPos, Vec4(255, 255, 255, 255), temp.c_str(), percent);
			}
		}
	}

	if (jungleDraw->Enabled())
	{
		for (auto minion : GEntityList->GetAllMinions(false, false, true))
		{
			if (minion->IsValidTarget() && Is_Big_Mob(minion))
			{
				int Hit2 = Count_E_Hit(minion);
				double Damage2 = Calcul_Multiple_E_Damage(minion, Hit2);
				double percent2 = (Damage2 * 100) / minion->GetHealth();
				Vec2 textPos; minion->GetHPBarPosition(textPos);
				std::string temp = "%.2f%% - " + std::to_string(Hit2) + "hit";
				GRender->DrawTextW(textPos, Vec4(255, 255, 255, 255), temp.c_str(), percent2);
			}
		}
	}
}

void OnCreateObject(IUnit* Object)
{
	if (strstr(Object->GetObjectName(), "Feather") && !Object->IsEnemy(myHero))
	{
		SPlume data;
		data.Plume = Object;
		data.startTime = GGame->Time();
		VPlume.push_back(data);
	}
}

void OrbwalkAfterAttack(IUnit* From, IUnit* To)
{
	if (From == myHero && To->IsHero())
	{
		if (Q->IsReady() && GetDistance(To->GetPosition(), myHero->GetPosition()) < qRange->GetInteger() && qType->GetInteger() == 0 && GOrbwalking->GetOrbwalkingMode() == kModeCombo)
			Q->CastOnTarget(To, kHitChanceMedium);
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
}

void SpellCast(CastedSpell const& Spell)
{
	if (!R->IsReady() || !rEnable->Enabled())
		return;
	if (Spell.Caster_ != nullptr && Spell.Caster_->IsHero() && Spell.Caster_->IsEnemy(myHero))
	{
		for (auto champion : SpellDatabase::Champions)
		{
			if (std::string(Spell.Caster_->ChampionName()) == std::string(champion.second.DisplayName))
			{
				auto spells = champion.second.Spells;
				for (auto Spell2 : spells)
				{
					if (std::string(Spell.Name_) == std::string(Spell2.Name))
					{
						if (Spell.Target_ != nullptr &&  Spell.Target_->GetNetworkId() == myHero->GetNetworkId())
						{
							R->CastOnPosition(Spell.Caster_->GetPosition());
							return;
						}
						if (GetMenuBoolean(std::string(Spell2.Name)) && GetDistance(Spell.EndPosition_, myHero->GetPosition()) < Spell2.Radius)
							R->CastOnPosition(Spell.Caster_->GetPosition());
					}
				}
			}
		}
	}
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Load_Menu();
	Load_Variable();
	GGame->PrintChat("<font color=\"#009688\"><b>Xayah White Plume</font><font color=\"#4DB6AC\"> is successfuly loaded</b></font>");
	GEventManager->AddEventHandler(kEventOnGameUpdate, GameUpdate);
	GEventManager->AddEventHandler(kEventOnRender, Render);
	GEventManager->AddEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->AddEventHandler(kEventOnBuffAdd, BuffAdd);
	GEventManager->AddEventHandler(kEventOrbwalkAfterAttack, OrbwalkAfterAttack);
	GEventManager->AddEventHandler(kEventOnSpellCast, SpellCast);
}

PLUGIN_API void OnUnload()
{
	mainMenu->SaveSettings();
	mainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, GameUpdate);
	GEventManager->RemoveEventHandler(kEventOnRender, Render);
	GEventManager->RemoveEventHandler(kEventOnBuffAdd, BuffAdd);
	GEventManager->RemoveEventHandler(kEventOnSpellCast, SpellCast);
	GEventManager->RemoveEventHandler(kEventOrbwalkAfterAttack, OrbwalkAfterAttack);
}
