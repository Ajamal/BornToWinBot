#include "Common.h"
#include "CombatCommander.h"

bool wait; 
bool charge;

CombatCommander::CombatCommander() 
	: attacking(false)
	, foundEnemy(false)
	, attackSent(false) 
{
	
}

bool CombatCommander::squadUpdateFrame()
{
	return BWAPI::Broodwar->getFrameCount() % 24 == 0;
}

void CombatCommander::update(std::set<BWAPI::Unit *> unitsToAssign)
{
	if(squadUpdateFrame())
	{
		// clear all squad data
		squadData.clearSquadData();

		//B2WB worker rush
		if (StrategyManager::Instance().getCurrentStrategy() == StrategyManager::WorkerRush) assignWorkerRush(unitsToAssign);
		else
		{
			// give back combat workers to worker manager
			WorkerManager::Instance().finishedWithCombatWorkers();

			// Assign defense and attack squads
			assignScoutDefenseSquads();
			assignDefenseSquads(unitsToAssign);
		}
		assignAttackSquads(unitsToAssign);
		assignIdleSquads(unitsToAssign);
	}

	squadData.update();
}
//used to return the closest position to base for a set of positions
struct StartPosComp
{
	bool operator()(const BWAPI::Position lhs, const BWAPI::Position rhs)
	{
		if (lhs.getDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition()) <
			rhs.getDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition())) return true;
		//ensure each position gets added to the set
		if (lhs.getDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition()) >
			rhs.getDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition())) return false;
		return true;
	}
};

//B2WB worker rush
void CombatCommander::assignWorkerRush(std::set<BWAPI::Unit *> & unitsToAssign)
{
	if (unitsToAssign.empty()) { return; }

	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
	BWTA::Region * enemyRegion = enemyBaseLocation ? enemyBaseLocation->getRegion() : NULL;

	if (enemyRegion && enemyRegion->getCenter().isValid())
	{
		//if the rushers have seen the base then left have the closest worker get a visual
		if (BWAPI::Broodwar->isExplored(enemyBaseLocation->getTilePosition()) && !(BWAPI::Broodwar->isVisible(enemyBaseLocation->getTilePosition())))
		{
			double minDistance = 0;
			BWAPI::Unit * closest = NULL;
			BOOST_FOREACH(BWAPI::Unit * unit, unitsToAssign)
			{
				double distance = enemyBaseLocation->getPosition().getDistance(unit->getPosition());
				if (!closest || (distance < minDistance))
				{
					minDistance = distance;
					closest = unit;
				}
			}
			unitsToAssign.erase(closest);
			closest->attack(enemyBaseLocation->getPosition());
		}
		UnitVector combatUnits(unitsToAssign.begin(), unitsToAssign.end());
		unitsToAssign.clear();

		//focus attack on on enemy workers next to enemy base
		BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
		{
			if (unit->getType().isWorker() && (unit->getDistance(enemyBaseLocation->getPosition()) < 200))
			{
				(*combatUnits.begin())->stop(); //ensures at least one unit stops what its doing and attacks
				squadData.addSquad(Squad(combatUnits, SquadOrder(SquadOrder::Attack, unit->getPosition(), 2, "Stomp Gatherers")));
				return;
			}
			//If a enemy combat unit is created this plan is bound to fail by attack anyway!
			else if (unit->getType().canAttack() && !unit->getType().isWorker())
			{
				BOOST_FOREACH(BWAPI::Unit * worker, unitsToAssign)
				{
					worker->attack(unit);
				}

				return;
			}
		}
		//if no workers near the visible base attack to closest units/buildings close to the enemy base loaction
		if (BWAPI::Broodwar->isExplored(enemyBaseLocation->getTilePosition()))
		{
			double minDistance = 0;
			BWAPI::Unit * closest = NULL;
			BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
			{
				double distance = enemyBaseLocation->getPosition().getDistance(unit->getPosition());
				if (!closest || (distance < minDistance) && unit->getType().isBuilding())
				{
					minDistance = distance;
					closest = unit;
				}
			}
			if (closest)
			{
				squadData.addSquad(Squad(combatUnits, SquadOrder(SquadOrder::Attack, closest->getPosition(), 200, "Destroy Base")));
				return;
			}
		}
		//send worker rush
		if (!BWAPI::Broodwar->isExplored(enemyBaseLocation->getTilePosition()))
		{
			squadData.addSquad(Squad(combatUnits, SquadOrder(SquadOrder::Attack, enemyBaseLocation->getPosition(), 100, "Worker Rush")));
		}
		//else return allowing the normal attack function to run with the workers
		return;
	}
	//if enemy base location is unknown search for it
	// for each start location in the level assign groups of workers to scout each are
	else if (!enemyRegion)
	{
		std::set<BWAPI::Position, StartPosComp> unexplored_locations;
		BOOST_FOREACH(BWTA::BaseLocation * startLocation, BWTA::getStartLocations())
		{
			// if we haven't explored it yet
			if (!BWAPI::Broodwar->isExplored(startLocation->getTilePosition()))
			{
				unexplored_locations.insert(startLocation->getPosition());
			}
		}
		BWAPI::Position next;
		UnitVector combatUnits(unitsToAssign.begin(), unitsToAssign.end());
		unitsToAssign.clear();
		//if its a 4 player map go to the possible base location closest to our base
		if (unexplored_locations.size() > 2)
		{
			next = *unexplored_locations.begin();
		}
		//if there are only 2 possible locations go to the location closest to one worker 
		else if (unexplored_locations.size() == 2)
		{
			if ((*combatUnits.begin())->getDistance(*unexplored_locations.begin()) <
				(*combatUnits.begin())->getDistance(*++unexplored_locations.begin()))
			{
				next = *unexplored_locations.begin();
			}
			else next = *++unexplored_locations.begin();
		}
		squadData.addSquad(Squad(combatUnits, SquadOrder(SquadOrder::Attack, next, 100, "Search")));
		return;
	}
	//if none of the above fires and unit is doing nothing return to defend main base
	if ((*unitsToAssign.begin())->isIdle())
	{
		UnitVector combatUnits(unitsToAssign.begin(), unitsToAssign.end());
		unitsToAssign.clear();
		squadData.addSquad(Squad(combatUnits, SquadOrder(SquadOrder::Attack, InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition(), 1000, "Defend")));
	}
}
void CombatCommander::assignIdleSquads(std::set<BWAPI::Unit *> & unitsToAssign)
{
	if (unitsToAssign.empty()) { return; }

	UnitVector combatUnits(unitsToAssign.begin(), unitsToAssign.end());
	unitsToAssign.clear();

	squadData.addSquad(Squad(combatUnits, SquadOrder(SquadOrder::Defend, BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()), 1000, "Defend Idle")));
}

void CombatCommander::assignAttackSquads(std::set<BWAPI::Unit *> & unitsToAssign)
{
	if (unitsToAssign.empty()) { return; }

	// added if statement. Want to attack together. May add && logic.
	// First 6 Zealots will wait at base when using ProtossCustomDragoons strategy.
	if ((unitsToAssign.size() < 6) && (StrategyManager::Instance().getCurrentStrategy() == StrategyManager::ProtossCustomDragoons) && !charge ) { return; }
	
	bool workersDefending = false;
	BOOST_FOREACH (BWAPI::Unit * unit, unitsToAssign)
	{
		if (unit->getType().isWorker() && !StrategyManager::Instance().getCurrentStrategy() == StrategyManager::WorkerRush)
		{
			workersDefending = true;
		}
	}

	// do we have workers in combat
	bool attackEnemy = !unitsToAssign.empty() && !workersDefending && StrategyManager::Instance().doAttack(unitsToAssign);

	// if we are attacking, what area are we attacking?
	if (attackEnemy) 
	{	
		assignAttackRegion(unitsToAssign);				// attack occupied enemy region
		assignAttackKnownBuildings(unitsToAssign);		// attack known enemy buildings
		assignAttackVisibleUnits(unitsToAssign);			// attack visible enemy units
		assignAttackExplore(unitsToAssign);				// attack and explore for unknown units
	} 
}

BWTA::Region * CombatCommander::getClosestEnemyRegion()
{
	BWTA::Region * closestEnemyRegion = NULL;
	double closestDistance = 100000;

	// for each region that our opponent occupies
	BOOST_FOREACH (BWTA::Region * region, InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->enemy()))
	{
		double distance = region->getCenter().getDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));

		if (!closestEnemyRegion || distance < closestDistance)
		{
			closestDistance = distance;
			closestEnemyRegion = region;
		}
	}

	return closestEnemyRegion;
}

void CombatCommander::assignScoutDefenseSquads() 
{
	// for each of our occupied regions
	BOOST_FOREACH(BWTA::Region * myRegion, InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self()))
	{
		BWAPI::Position regionCenter = myRegion->getCenter();
		if (!regionCenter.isValid())
		{
			continue;
		}

		// all of the enemy units in this region
		std::set<BWAPI::Unit *> enemyUnitsInRegion;
		BOOST_FOREACH(BWAPI::Unit * enemyUnit, BWAPI::Broodwar->enemy()->getUnits())
		{
			if (BWTA::getRegion(BWAPI::TilePosition(enemyUnit->getPosition())) == myRegion)
			{
				enemyUnitsInRegion.insert(enemyUnit);
			}
		}

		// special case: figure out if the only attacker is a worker, the enemy is scouting
		if (enemyUnitsInRegion.size() == 1 && (*enemyUnitsInRegion.begin())->getType().isWorker())
		{
			// the enemy worker that is attacking us
			BWAPI::Unit * enemyWorker = *enemyUnitsInRegion.begin();

			// get our worker unit that is mining that is closest to it
			BWAPI::Unit * workerDefender = WorkerManager::Instance().getClosestMineralWorkerTo(enemyWorker);

			// grab it from the worker manager
			WorkerManager::Instance().setCombatWorker(workerDefender);

			// put it into a unit vector
			UnitVector workerDefenseForce;
			workerDefenseForce.push_back(workerDefender);

			// make a squad using the worker to defend
			squadData.addSquad(Squad(workerDefenseForce, SquadOrder(SquadOrder::Defend, regionCenter, 1000, "Get That Scout!")));

			// seeing that not all workers go after one enemy.
			//workerDefenseForce.clear();
			return;
		}
	}
}

void CombatCommander::assignDefenseSquads(std::set<BWAPI::Unit *> & unitsToAssign) 
{
	if (unitsToAssign.empty()) { return; }

	// for each of our occupied regions
	BOOST_FOREACH(BWTA::Region * myRegion, InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self()))
	{
		BWAPI::Position regionCenter = myRegion->getCenter();
		if (!regionCenter.isValid())
		{
			continue;
		}

		// start off assuming all enemy units in region are just workers
		int numDefendersPerEnemyUnit = 1;

		// all of the enemy units in this region
		std::set<BWAPI::Unit *> enemyUnitsInRegion;
		BOOST_FOREACH (BWAPI::Unit * enemyUnit, BWAPI::Broodwar->enemy()->getUnits())
		{			
			if (BWTA::getRegion(BWAPI::TilePosition(enemyUnit->getPosition())) == myRegion)
			{
				enemyUnitsInRegion.insert(enemyUnit);
				// if the enemy isn't a worker, increase the amount of defenders for it
				if (!enemyUnit->getType().isWorker())
				{
					numDefendersPerEnemyUnit = 3;
				}
			}
		}

		// figure out how many units we need on defense
		const int numFlyingNeeded = numDefendersPerEnemyUnit * InformationManager::Instance().numEnemyFlyingUnitsInRegion(myRegion);
		const int numGroundNeeded = numDefendersPerEnemyUnit * InformationManager::Instance().numEnemyUnitsInRegion(myRegion);

		if(numGroundNeeded > 0 || numFlyingNeeded > 0)
		{
			// our defenders
			std::set<BWAPI::Unit *> flyingDefenders;
			std::set<BWAPI::Unit *> groundDefenders;

			BOOST_FOREACH (BWAPI::Unit * unit, unitsToAssign)
			{
				if (unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
				{
					flyingDefenders.insert(unit);
				}
				else if (unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)
				{
					groundDefenders.insert(unit);
				}
			}

			// the defense force we want to send
			UnitVector defenseForce;

			// get flying defenders
			for (int i=0; i<numFlyingNeeded && !flyingDefenders.empty(); ++i)
			{
				BWAPI::Unit * flyingDefender = findClosestDefender(enemyUnitsInRegion, flyingDefenders);
				defenseForce.push_back(flyingDefender);
				unitsToAssign.erase(flyingDefender);
				flyingDefenders.erase(flyingDefender);
			}

			// get ground defenders
			for (int i=0; i<numGroundNeeded && !groundDefenders.empty(); ++i)
			{
				BWAPI::Unit * groundDefender = findClosestDefender(enemyUnitsInRegion, groundDefenders);

				if (groundDefender->getType().isWorker())
				{
					WorkerManager::Instance().setCombatWorker(groundDefender);
				}

				defenseForce.push_back(groundDefender);
				unitsToAssign.erase(groundDefender);
				groundDefenders.erase(groundDefender);
			}

			// if we need a defense force, make the squad and give the order
			if (!defenseForce.empty()) 
			{
				squadData.addSquad(Squad(defenseForce, SquadOrder(SquadOrder::Defend, regionCenter, 1000, "Defend Region")));
				return;
			}
		}
	}
}

void CombatCommander::assignAttackRegion(std::set<BWAPI::Unit *> & unitsToAssign) 
{
	if (unitsToAssign.empty()) { return; }

	BWTA::Region * enemyRegion = getClosestEnemyRegion();

	if (enemyRegion && enemyRegion->getCenter().isValid()) 
	{
		UnitVector oppUnitsInArea, ourUnitsInArea;
		MapGrid::Instance().GetUnits(oppUnitsInArea, enemyRegion->getCenter(), 800, false, true);
		MapGrid::Instance().GetUnits(ourUnitsInArea, enemyRegion->getCenter(), 200, true, false);

		if (!oppUnitsInArea.empty())
		{
			UnitVector combatUnits(unitsToAssign.begin(), unitsToAssign.end());
			// print how many combat units in squad
			//BWAPI::Broodwar->printf("Combat Units: %d", combatUnits.size());
			unitsToAssign.clear();

			squadData.addSquad(Squad(combatUnits, SquadOrder(SquadOrder::Attack, enemyRegion->getCenter(), 1000, "Attack Region")));
		}
		// added else if statement. Want to make units attack together. Happens only once.
		// First 6 Zealots will attack together when using ProtossCustomDragoon strategy.
		else if (unitsToAssign.size() >= 6 && (StrategyManager::Instance().getCurrentStrategy() == StrategyManager::ProtossCustomDragoons) && !charge) {
			charge = 1;
			UnitVector combatUnits(unitsToAssign.begin(), unitsToAssign.end());
			unitsToAssign.clear();
			squadData.addSquad(Squad(combatUnits, SquadOrder(SquadOrder::Attack, enemyRegion->getCenter(), 1000, "Attack together!!")));
		}
	}
}

void CombatCommander::assignAttackVisibleUnits(std::set<BWAPI::Unit *> & unitsToAssign) 
{
	if (unitsToAssign.empty()) { return; }

	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->isVisible())
		{
			//B2WB Bait
			if (BaitManager::Instance().baitID != 0)
			{
				UnitVector nearbyAllies;
				MapGrid::Instance().GetUnits(nearbyAllies, BaitManager::Instance().baitPos, 320, true, false);
				if (nearbyAllies.size() <= 1 && (unit->getDistance(BaitManager::Instance().baitPos) < 300))
				{
					return;
				}
			}
			UnitVector combatUnits(unitsToAssign.begin(), unitsToAssign.end());
			unitsToAssign.clear();

			squadData.addSquad(Squad(combatUnits, SquadOrder(SquadOrder::Attack, unit->getPosition(), 1000, "Attack Visible")));
			return;
		}
	}
}

void CombatCommander::assignAttackKnownBuildings(std::set<BWAPI::Unit *> & unitsToAssign) 
{
	if (unitsToAssign.empty()) { return; }

	FOR_EACH_UIMAP_CONST (iter, InformationManager::Instance().getUnitInfo(BWAPI::Broodwar->enemy()))
	{
		const UnitInfo ui(iter->second);
		if(ui.type.isBuilding())
		{
			UnitVector combatUnits(unitsToAssign.begin(), unitsToAssign.end());
			unitsToAssign.clear();

			squadData.addSquad(Squad(combatUnits, SquadOrder(SquadOrder::Attack, ui.lastPosition, 1000, "Attack Known")));
			return;	
		}
	}
}

void CombatCommander::assignAttackExplore(std::set<BWAPI::Unit *> & unitsToAssign) 
{
	if (unitsToAssign.empty()) { return; }

	UnitVector combatUnits(unitsToAssign.begin(), unitsToAssign.end());
	unitsToAssign.clear();

	squadData.addSquad(Squad(combatUnits, SquadOrder(SquadOrder::Attack, MapGrid::Instance().getLeastExplored(), 1000, "Attack Explore")));
}

BWAPI::Unit* CombatCommander::findClosestDefender(std::set<BWAPI::Unit *> & enemyUnitsInRegion, const std::set<BWAPI::Unit *> & units) 
{
	BWAPI::Unit * closestUnit = NULL;
	double minDistance = 1000000;

	BOOST_FOREACH (BWAPI::Unit * enemyUnit, enemyUnitsInRegion) 
	{
		BOOST_FOREACH (BWAPI::Unit * unit, units)
		{
			double dist = unit->getDistance(enemyUnit);
			if (!closestUnit || dist < minDistance) 
			{
				closestUnit = unit;
				minDistance = dist;
			}
		}
	}

	return closestUnit;
}

BWAPI::Position CombatCommander::getDefendLocation()
{
	return BWTA::getRegion(BWTA::getStartLocation(BWAPI::Broodwar->self())->getTilePosition())->getCenter();
}

void CombatCommander::drawSquadInformation(int x, int y)
{
	squadData.drawSquadInformation(x, y);
}