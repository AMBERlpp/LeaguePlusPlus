#include "PluginSDK.h"
#include "Vector3.h"
#include <string>

double Version = 0.001;

IMenu* mainMenu;
IMenu* comboMenu;
IMenu* harassMenu;
IMenu* laneclearMenu;
IMenu* itemMenu;
IMenu* spellMenu;
IMenu* qMenu;
IMenu* eMenu;
IMenu* drawMenu;
IMenu* ksMenu;
IMenu* gapMenu;
IMenu* interruptibleMenu;
IMenuOption* qCombo;
IMenuOption* qHarass;
IMenuOption* qLaneclear;
IMenuOption* qMana;
IMenuOption* eHarass;
IMenuOption* qCastPos;
IMenuOption* qAfterE;
IMenuOption* eCombo;
IMenuOption* eRange;
IMenuOption* eAfterAA;
IMenuOption* qRange;
IMenuOption* qLine;
IMenuOption* eKS;
IMenuOption* eGap;
IMenuOption* eInterruptible;
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

IUnit* myHero;
Vec3 myPos;

ISpell2* Q;
ISpell2* E;


void Load()
{
	myHero = GEntityList->Player();

	Q = GPluginSDK->CreateSpell2(kSlotQ, kTargetCast, false, false, kCollidesWithNothing);
	Q->SetOverrideRange(300);
	E = GPluginSDK->CreateSpell2(kSlotE, kLineCast, true, false, kCollidesWithYasuoWall);
	E->SetSkillshot(0.2f, 80, 1800, 710);

	Youmuu = GPluginSDK->CreateItemForId(3142, 0);
	Bilgewater = GPluginSDK->CreateItemForId(3144, 550);
	BOTRK = GPluginSDK->CreateItemForId(3153, 550);
	Quicksilver = GPluginSDK->CreateItemForId(3140, 0);
	Mercurial = GPluginSDK->CreateItemForId(3139, 600);
}

void Update_Variable()
{
	myPos = GEntityList->Player()->GetPosition();
}

void Load_Menu()
{
	mainMenu = GPluginSDK->AddMenu("Vayne - Gold Tiles");
	comboMenu = mainMenu->AddMenu(">> Combo <<");
	{
		qCombo = comboMenu->CheckBox("Use (Q)", true);
		eCombo = comboMenu->CheckBox("Use (E)", true);
	}
	harassMenu = mainMenu->AddMenu(">> Harass <<");
	{
		qHarass = harassMenu->AddSelection("(Q) Usage", 1, { "None", "Proc (W)", "Reset (AA)" });
		eHarass = harassMenu->CheckBox("Use (E) to stun", true);
	}
	laneclearMenu = mainMenu->AddMenu(">> Lane Clear <<");
	{
		qLaneclear = laneclearMenu->CheckBox("Use (Q)", true);
		qMana = laneclearMenu->AddInteger("Min mana to use (%)", 0, 100, 80);
	}
	spellMenu = mainMenu->AddMenu(">> Spells <<");
	{
		qMenu = spellMenu->AddMenu(">> (Q) <<");
		{
			qCastPos = qMenu->AddSelection("(Q) Position", 0, { "Mouse Position", "Logic" });
			qAfterE = qMenu->CheckBox("Combo - Cast (Q) after (E)", true);
		}
		eMenu = spellMenu->AddMenu(">> (E) <<");
		{
			eRange = eMenu->AddInteger("(E) max range", 300, 550, 450);
			eAfterAA = eMenu->CheckBox("Combo - Cast (E) after AA", true);
		}
	}
	ksMenu = mainMenu->AddMenu(">> Killsteal <<");
	{
		eKS = ksMenu->CheckBox("Use (E)", true);
	}
	gapMenu = mainMenu->AddMenu(">> Gapcloser <<");
	{
		eGap = gapMenu->CheckBox("Enable", true);
	}
	interruptibleMenu = mainMenu->AddMenu(">> Interrupt Spells <<");
	{
		eInterruptible = interruptibleMenu->CheckBox("Enable", true);
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
	drawMenu = mainMenu->AddMenu(">> Draw <<");
	{
		qRange = drawMenu->CheckBox("(Q) Range", true);
		qLine = drawMenu->CheckBox("(Q) Line", true);
	}
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

double Calcul_AA_Dmg(IUnit* Target)
{
	int aaDmg = myHero->TotalPhysicalDamage();
	return GDamage->CalcPhysicalDamage(myHero, Target, aaDmg);
}

double Calcul_E_Dmg(IUnit* Target)
{
	if (Target->IsInvulnerable())
		return 0;

	int basic[] = { 45, 80, 115, 150, 185 };
	int bonus = 0.5*myHero->BonusDamage();
	int total = basic[GEntityList->Player()->GetSpellLevel(kSlotE)] + bonus;
	int basicPassif[] = { 0.06, 0.075, 0.09, 0.105, 0.12 };
	int bonus2 = 0;
	if (Target->GetBuffCount("vaynesilvereddebuff") == 0)
	{
		bonus2 = basicPassif[GEntityList->Player()->GetSpellLevel(kSlotW)] * Target->GetMaxHealth();
	}

	return GDamage->CalcPhysicalDamage(myHero, Target, total) + bonus2 + Calcul_AA_Dmg(Target);
}

bool Combo_Q_Cast()
{
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo && qCombo->Enabled() && Q->IsReady())
		return true;

	return false;
}

bool Harass_Q_Cast(IUnit* Target)
{
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed && Q->IsReady())
	{
		if ((qHarass->GetInteger() == 1 && Target->GetBuffCount("vaynesilvereddebuff") == 1) || qHarass->GetInteger() == 2)
			return true;
	}

	return false;
}

bool Combo_E_Cast(IUnit* Target)
{
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo && eCombo->Enabled() && E->IsReady())
	{
		for (int i = 40; i < eRange->GetInteger(); i = i+20)
		{
			AdvPredictionOutput predOut;
			E->RunPrediction(Target, false, kCollidesWithYasuoWall, &predOut);
			Vec3 pos = predOut.TargetPosition + (predOut.TargetPosition - myPos).VectorNormalize() * i;
			int flagsOut = GNavMesh->GetCollisionFlagsForPoint(pos);
			if ((flagsOut == kWallMesh ||flagsOut == kBuildingMesh) && predOut.HitChance >= kHitChanceHigh)
				return true;
		
		}
		return false;
	}
	return false;
}

bool Harass_E_Cast(IUnit* Target)
{
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed && eHarass->Enabled() && E->IsReady())
	{
		for (int i = 40; i < eRange->GetInteger(); i = i + 20)
		{
			AdvPredictionOutput predOut;
			E->RunPrediction(Target, false, kCollidesWithYasuoWall, &predOut);
			Vec3 pos = predOut.TargetPosition + (predOut.TargetPosition - myPos).VectorNormalize() * i;
			int flagsOut = GNavMesh->GetCollisionFlagsForPoint(pos);
			if ((flagsOut == kWallMesh || flagsOut == kBuildingMesh) && predOut.HitChance >= kHitChanceHigh)
				return true;

		}
		return false;
	}
	return false;
}

int ranged_rand(int min, int max) {
	return min + (int)((double)(max - min) * (rand() / (RAND_MAX + 1.0)));
}

Vec3 Get_Q_Position(IUnit* Target)
{
	Vec3 pos = myPos + (Target->GetPosition() - myPos).VectorNormalize() * 100;
	int randomNum = (rand() % 3 + (-1)) * 100;
	Vec3 perpendicularPos = pos + (pos - Target->GetPosition()).VectorNormalize().Perpendicular() * (randomNum);
	return perpendicularPos;
}

void Cast_Q(IUnit* Target)
{
	Vec3 pos;
	if (qCastPos->GetInteger() == 0)
		pos = GGame->CursorPosition();
	else
		pos = Get_Q_Position(Target);

	Q->CastOnPosition(pos);
}

void Cast_Item(IUnit* Target)
{
	if (GOrbwalking->GetOrbwalkingMode() != kModeCombo)
		return;

	if (youmuuItem->Enabled() && Youmuu->IsOwned() && Youmuu->IsReady())
	{
		Youmuu->CastOnPlayer();
	}
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
	auto Target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, E->Range());
	if (Target == nullptr || !Target->IsValidTarget())
		return;

	if(!eAfterAA->Enabled() && Combo_E_Cast(Target))
	{
		E->CastOnUnit(Target);
	}
	
	Cast_Item(Target);
}

bool Laneclear_Q_Cast()
{
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear && qLaneclear->Enabled() && myHero->ManaPercent() > qMana->GetInteger())
		return true;

	return false;
}

void Render()
{
	if (Q->IsReady())
	{
		if (qRange->Enabled())
			GRender->DrawCircle(myPos,300, Vec4(0, 200, 100, 255));
		if (qLine->Enabled())
		{
			Vec3 pos = myPos + (GGame->CursorPosition() - myPos).VectorNormalize() * 300;
			Vec2 posOutStart;
			Vec2 posOutEnd;
			GGame->Projection(pos, &posOutEnd);
			GGame->Projection(myPos, &posOutStart);
			GRender->DrawLine(posOutEnd, posOutStart, Vec4(0, 200, 100, 255));
			GRender->DrawFilledCircle(pos, Vec4(0, 200, 100, 255), 20);
		}
	}
}

std::function<void()> E_Delay(IUnit* Target)
{
	E->CastOnUnit(Target);
	return nullptr;
}

void Gapcloser(GapCloserSpell const& Spell)
{
	if (eGap->Enabled() && E->IsReady() && Spell.Sender != nullptr && Spell.Sender->IsValidTarget() && Spell.Sender->IsEnemy(myHero) && GetDistance(Spell.EndPosition, myPos) < 200)
	{
		if (GetDistance(Spell.StartPosition, myPos) < E->Range())
			E->CastOnUnit(Spell.Sender);
		//else
		//	GPluginSDK->DelayFunctionCall(1000, E_Delay(Spell.Sender));
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

void Interruptible(InterruptibleSpell const& Spell)
{
	if (eInterruptible->Enabled() && E->IsReady() && Spell.Target->IsValidTarget() && GetDistance(myPos, Spell.Target->GetPosition()) < E->Range() && Spell.DangerLevel >= kMediumDanger)
		E->CastOnUnit(Spell.Target);
}

void SpellCast(CastedSpell const& Spell)
{
	if (qAfterE->Enabled() && Spell.Caster_ == myHero && Combo_Q_Cast() && std::string(Spell.Name_) == "VayneCondemnMissile")
	{
		Cast_Q(Spell.Target_);
	}
		
}

void GameUpdate()
{
	Update_Variable();
	Combo();
}

void OrbwalkAfterAttack(IUnit* From, IUnit* To)
{
	if (From == myHero && Laneclear_Q_Cast())
		Cast_Q(To);

	if (From == myHero && To->IsHero())
	{
		if ((eAfterAA->Enabled() && Combo_E_Cast(To)) || Harass_E_Cast(To) || (E->IsReady() && Calcul_E_Dmg(To) > To->GetHealth() && Calcul_AA_Dmg(To) < To->GetHealth() && eKS->Enabled()))
			E->CastOnUnit(To);
		else if(Combo_Q_Cast())
			Cast_Q(To);

		if (Harass_Q_Cast(To))
			Cast_Q(To);
	}
}

std::function<void()> Message_Delay = [&]() -> void {
	GGame->PrintChat("<font color=\"#AB47BC\"><b>Vayne Gold Tiles - </font><font color=\"#D50000\">New version available, Download it in the Database</b></font>");
};

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	GGame->PrintChat("<font color=\"#AB47BC\"><b>Vayne Gold Tiles - </font><font color=\"#CE93D8\">Successfuly loaded</b></font>");
	Load();
	Load_Menu();
	GEventManager->AddEventHandler(kEventOnBuffAdd, BuffAdd);
	GEventManager->AddEventHandler(kEventOnGameUpdate, GameUpdate);
	GEventManager->AddEventHandler(kEventOnRender, Render);
	GEventManager->AddEventHandler(kEventOrbwalkAfterAttack, OrbwalkAfterAttack);
	GEventManager->AddEventHandler(kEventOnSpellCast, SpellCast);
	GEventManager->AddEventHandler(kEventOnGapCloser, Gapcloser);
	GEventManager->AddEventHandler(kEventOnInterruptible, Interruptible);

	std::string file;
	GPluginSDK->ReadFileFromURL("https://raw.githubusercontent.com/AMBERlpp/LeaguePlusPlus/master/Vayne%20-%20Gold%20Tiles/Vayne%20-%20Gold%20Tiles/Vayne.version", file);
	double onlineVersion = std::stod(file);

	if (onlineVersion != Version)
	{
		GPluginSDK->DelayFunctionCall(1000, Message_Delay);
		GPluginSDK->DelayFunctionCall(3000, Message_Delay);
		GPluginSDK->DelayFunctionCall(5000, Message_Delay);
	}
	else
		GGame->PrintChat("<font color=\"#AB47BC\"><b>Vayne Gold Tiles - </font><font color=\"#D50000\">Latest version found. Have a good time !</b></font>");
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