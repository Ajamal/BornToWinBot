#include "Common.h"
#include "BaitManager.h"
#include "InformationManager.h"
#include "MicroManager.h"

BaitManager::BaitManager() : numBaitUnits(0), hasBaited(false), beingChased(false), currentDest(0), mapLoaded(false), baitID(0), waitTime(0), baitPos(BWAPI::Position(-500,-500))
{}

// get an instance of this
BaitManager & BaitManager::Instance()
{
	static BaitManager instance;
	return instance;
}

//used to return the closest position to enemybase for a set of positions
struct posComp
{
	bool operator()(const BWAPI::Position lhs, const BWAPI::Position rhs)
	{
		if (lhs.getDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy())->getPosition()) <
			rhs.getDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy())->getPosition())) return true;
		//ensure each position gets added to the set
		if (lhs.getDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy())->getPosition()) >
			rhs.getDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy())->getPosition())) return false;
		return true;
	}
};

void BaitManager::initialize_map_points(std::string mapName)
{
	//BWAPI::Position mainBaseLocation = 
	//BWTA::BaseLocation * mainBaseLocation = BWTA::getStartLocation(BWAPI::Broodwar->self());
	BWAPI::Position mainBasePosition = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition();
	//BWAPI::Position mainBasePositon = mainBaseLocation->getPosition();
	mapLoaded = true;
	if (mapName == "(2)Benzene.scx")
	{	
		//set the point where the bait unit will wait for enemies
		if (mainBasePosition.getDistance(BWAPI::Position(400, 1953)) > mainBasePosition.getDistance(BWAPI::Position(3180, 1790)))
		{
			waitPoint = BWAPI::Position(400, 1953);
		}
		else
		{
			waitPoint = BWAPI::Position(3180, 1790);
		}
		//set the points where the bait unit will flee to
		chasePoints.push_back(BWAPI::Position(1400, 960));
		chasePoints.push_back(BWAPI::Position(1050, 790));
		chasePoints.push_back(BWAPI::Position(1110, 500));
		chasePoints.push_back(BWAPI::Position(1400, 320));
		chasePoints.push_back(BWAPI::Position(1880, 604));	

		BWAPI::Broodwar->printf("Map Benzene Loaded");
		return;
	}
	else if (mapName == "(2)Destination.scx")
	{
		//set the point where the bait unit will wait for enemies
		if (mainBasePosition.getDistance(BWAPI::Position(1155, 3515)) > mainBasePosition.getDistance(BWAPI::Position(1890, 525)))
		{
			waitPoint = BWAPI::Position(1155, 3515);
		}
		else
		{
			waitPoint = BWAPI::Position(1890, 525);
		}
		//set the points where the bait unit will flee to
		chasePoints.push_back(BWAPI::Position(1115, 1848));
		chasePoints.push_back(BWAPI::Position(1170, 1143));
		chasePoints.push_back(BWAPI::Position(1200, 2255));
		chasePoints.push_back(BWAPI::Position(958, 2770));
		chasePoints.push_back(BWAPI::Position(270, 1857));

		BWAPI::Broodwar->printf("Map Destination Loaded");
		return;
	}
	else if (mapName == "(2)Heartbreak Ridge.scx")
	{
		//set the point where the bait unit will wait for enemies
		if (mainBasePosition.getDistance(BWAPI::Position(3320, 930)) > mainBasePosition.getDistance(BWAPI::Position(720, 2175)))
		{
			waitPoint = BWAPI::Position(3320, 930);
		}
		else
		{
			waitPoint = BWAPI::Position(720, 2175);
		}
		//set the points where the bait unit will flee to
		chasePoints.push_back(BWAPI::Position(2050, 11));
		chasePoints.push_back(BWAPI::Position(3340, 34));
		chasePoints.push_back(BWAPI::Position(349, 175));
		chasePoints.push_back(BWAPI::Position(3180, 237));
		chasePoints.push_back(BWAPI::Position(690, 197));
		chasePoints.push_back(BWAPI::Position(373, 376));
		chasePoints.push_back(BWAPI::Position(215, 221));
		chasePoints.push_back(BWAPI::Position(395, 89));

		BWAPI::Broodwar->printf("Map Heartbreak Ridge Loaded");
		return;
	}

	//BWAPI::Position enemyBasePosition = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy())->getPosition();
	if (!InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy())->getPosition().isValid())
	{
		mapLoaded = false;
		BWAPI::Broodwar->printf("Cannot load 3-4 player map until enemy base is found");
		return;
	}
	//create a set to add positions to sorted such the the closest one to the enemy base is first
	std::set<BWAPI::Position, posComp> basePoints;

	if (mapName == "(3)Aztec.scx")
	{
		//set the point where the bait unit will wait for enemies
		basePoints.insert(BWAPI::Position(3097, 366));
		basePoints.insert(BWAPI::Position(3321, 3798));
		basePoints.insert(BWAPI::Position(294, 1994));
		
		waitPoint = *basePoints.begin();

		//set the points where the bait unit will flee to
		chasePoints.push_back(BWAPI::Position(2180, 3374));
		chasePoints.push_back(BWAPI::Position(1812, 2014));
		chasePoints.push_back(BWAPI::Position(2410, 1915));

		BWAPI::Broodwar->printf("Map Aztec Loaded");
	}
	else if (mapName == "(3)Tau Cross.scx")
	{
		//set the point where the bait unit will wait for enemies
		basePoints.insert(BWAPI::Position(2000, 3719));
		basePoints.insert(BWAPI::Position(563, 757));
		basePoints.insert(BWAPI::Position(3610, 1185));

		waitPoint = *basePoints.begin();

		//set the points where the bait unit will flee to
		chasePoints.push_back(BWAPI::Position(2413, 2340));
		chasePoints.push_back(BWAPI::Position(1800, 2338));
		chasePoints.push_back(BWAPI::Position(1900, 1942));

		BWAPI::Broodwar->printf("Map Tau Cross Loaded");
	}
	else if (mapName == "(4)Andromeda.scx")
	{
		//set the point where the bait unit will wait for enemies
		basePoints.insert(BWAPI::Position(850, 951));
		basePoints.insert(BWAPI::Position(3248, 909));
		basePoints.insert(BWAPI::Position(3269, 3180));
		basePoints.insert(BWAPI::Position(875, 3153));

		waitPoint = *basePoints.begin();

		//set the points where the bait unit will flee to
		chasePoints.push_back(BWAPI::Position(2030, 1180));
		chasePoints.push_back(BWAPI::Position(1095, 2035));
		chasePoints.push_back(BWAPI::Position(2068, 2780));
		chasePoints.push_back(BWAPI::Position(3000, 2017));

		BWAPI::Broodwar->printf("Map Andromeda Loaded");
	}
	else if (mapName == "(4)Circuit Breaker.scx")
	{
		//set the point where the bait unit will wait for enemies
		basePoints.insert(BWAPI::Position(3570, 1066));
		basePoints.insert(BWAPI::Position(3559, 3037));
		basePoints.insert(BWAPI::Position(500, 3030));
		basePoints.insert(BWAPI::Position(390, 1085));

		waitPoint = *basePoints.begin();

		//set the points where the bait unit will flee to
		chasePoints.push_back(BWAPI::Position(3570, 2305));
		chasePoints.push_back(BWAPI::Position(3900, 2045));
		chasePoints.push_back(BWAPI::Position(3480, 1806));
		chasePoints.push_back(BWAPI::Position(3326, 2075));

		BWAPI::Broodwar->printf("Map Circuit Breaker Loaded");
	}
	else if (mapName == "(4)Empire of the Sun.scm")
	{
		//set the point where the bait unit will wait for enemies
		basePoints.insert(BWAPI::Position(1204, 3426));
		basePoints.insert(BWAPI::Position(1158, 592));
		basePoints.insert(BWAPI::Position(2943, 3508));
		basePoints.insert(BWAPI::Position(2910, 592));

		waitPoint = *basePoints.begin();

		//set the points where the bait unit will flee to
		chasePoints.push_back(BWAPI::Position(817, 1140));
		chasePoints.push_back(BWAPI::Position(200, 1569));
		chasePoints.push_back(BWAPI::Position(720, 1876));
		chasePoints.push_back(BWAPI::Position(1094, 1461));

		BWAPI::Broodwar->printf("Map Empire of the Sun Loaded");
	}
	else if (mapName == "(4)Fortress.scx")
	{
		//set the point where the bait unit will wait for enemies
		basePoints.insert(BWAPI::Position(1693, 3516));
		basePoints.insert(BWAPI::Position(980, 1692));
		basePoints.insert(BWAPI::Position(2356, 735));
		basePoints.insert(BWAPI::Position(3129, 2398));

		waitPoint = *basePoints.begin();

		//set the points where the bait unit will flee to
		chasePoints.push_back(BWAPI::Position(2094, 3116));
		chasePoints.push_back(BWAPI::Position(1453, 1214));
		chasePoints.push_back(BWAPI::Position(1352, 2581));
		chasePoints.push_back(BWAPI::Position(4696, 1573));

		BWAPI::Broodwar->printf("Map Fortress Loaded");
	}
	else if (mapName == "(4)Python.scx")
	{
		//set the point where the bait unit will wait for enemies
		basePoints.insert(BWAPI::Position(699, 2051));
		basePoints.insert(BWAPI::Position(1818, 693));
		basePoints.insert(BWAPI::Position(3351, 2040));
		basePoints.insert(BWAPI::Position(2154, 3335));

		waitPoint = *basePoints.begin();

		//set the points where the bait unit will flee to
		chasePoints.push_back(BWAPI::Position(2919, 3389));
		chasePoints.push_back(BWAPI::Position(3740, 4051));
		chasePoints.push_back(BWAPI::Position(3892, 3851));
		chasePoints.push_back(BWAPI::Position(3388, 3221));
		chasePoints.push_back(BWAPI::Position(310, 3000));

		BWAPI::Broodwar->printf("Map Python Loaded");
	}


	else
	{
		mapLoaded = false;
		BWAPI::Broodwar->printf("Unrecognized Map File Name: %s", &(BWAPI::Broodwar->mapFileName())[0]);
		//BWAPI::Broodwar->printf(&(BWAPI::Broodwar->mapFileName())[0]);
	}



}

void BaitManager::update(const std::set<BWAPI::Unit *> & baitUnits)
{
	//BWAPI::Broodwar->printf("BaitUnits:%d, Position:%d,%d", baitUnits.size(), baitPos.x(),baitPos.y());
	if (baitUnits.size() == 1 && mapLoaded)
	{
		BWAPI::Unit * baitUnit = *baitUnits.begin();
		//BWAPI::Broodwar->printf("Moving Bait Unit");
		baitPos = baitUnit->getPosition();
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(baitUnit->getPosition().x() - 3,
			baitUnit->getPosition().y() - 12, baitUnit->getPosition().x() + 3, baitUnit->getPosition().y() + 12, BWAPI::Colors::Purple, true);
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawCircleMap(baitUnit->getPosition().x(), baitUnit->getPosition().y(),
			100, BWAPI::Colors::Purple, false);
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawCircleMap(baitUnit->getPosition().x(), baitUnit->getPosition().y(),
			320, BWAPI::Colors::Green, false);
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawCircleMap(baitUnit->getPosition().x(), baitUnit->getPosition().y(),
			300, BWAPI::Colors::Red, false);

		moveBait(baitUnit);
	}
}
bool BaitManager::nearbyEnemies(BWAPI::Unit * baitUnit)
{
	UnitVector nearbyEnemies;
	MapGrid::Instance().GetUnits(nearbyEnemies, baitUnit->getPosition(), 300, false, true);
	bool retreat = false;
	BOOST_FOREACH(BWAPI::Unit * unit, nearbyEnemies)
	{
		if ((!unit->getType().isBuilding() && unit->getType().groundWeapon().maxRange() > 32) && (baitUnit->getDistance(unit) < 200))
		{
			lastSeen = unit;
			retreat = true;
		}
		else if (!unit->getType().isBuilding() && !unit->getType().isWorker() && baitUnit->getDistance(unit) < 100)
		{
			lastSeen = unit;
			retreat = true;
		}
	}
	return retreat;
}

void BaitManager::runToNext(BWAPI::Unit * baitUnit)
{
	beingChased = true;
	//if (currentDest+1 > chasePoints.size()) currentDest = 0;
	//BWAPI::Broodwar->printf("%d modulo %d is %d", currentDest + 1, chasePoints.size(), (currentDest+1) % chasePoints.size());
	baitUnit->move(chasePoints[currentDest]);
	currentDest = (currentDest + 1) % chasePoints.size();
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(baitUnit->getPosition().x(), baitUnit->getPosition().y(),
		chasePoints[currentDest].x(), chasePoints[currentDest].y(),
		BWAPI::Colors::Purple);
	//BWAPI::Broodwar->printf("new chase dest #%d", currentDest);
}

void BaitManager::moveBait(BWAPI::Unit * baitUnit)
{
	//BWAPI::Broodwar->printf("Inside of Move Bait");
	//if at a chasepoint run to the next one if enemies are nearby
	//BWAPI::Broodwar->printf("current dest is #%d", currentDest);
	if (beingChased && (baitUnit->getDistance(chasePoints[currentDest]) < 8))
	{
		if (nearbyEnemies(baitUnit))
		{
			runToNext(baitUnit);
		}
		return;
	}

	//wait for pursuers to catch up or continue running
	if (beingChased)
	{
		if (!nearbyEnemies(baitUnit) && !baitUnit->isUnderAttack())
		{
			if (lastSeen)
			{
				if (lastSeen->exists())
				{
					if (lastSeen->getType().groundWeapon().maxRange() > 32)
					{
						baitUnit->holdPosition();
						waitTime++;
					}
					else if (baitUnit->isHoldingPosition() && waitTime > 250)
					{
						waitTime = 0;
						currentDest = 0;
						beingChased = false;
						baitUnit->move(waitPoint);
					}
					else
					{
						baitUnit->move(lastSeen->getPosition());
						BWAPI::Broodwar->drawLineMap(baitUnit->getPosition().x(), baitUnit->getPosition().y(),
							lastSeen->getPosition().x(), lastSeen->getPosition().y(),
							BWAPI::Colors::Purple);
					}
				}
			}
		}
		else 
		{
			baitUnit->move(chasePoints[currentDest]);
			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(baitUnit->getPosition().x(), baitUnit->getPosition().y(),
				chasePoints[currentDest].x(), chasePoints[currentDest].y(),
				BWAPI::Colors::Purple);
		}
		return;
	}

	//if not being chased head to the wait point
	if (!beingChased && !baitUnit->isUnderAttack()){
		if (nearbyEnemies(baitUnit))
		{
			runToNext(baitUnit);
			return;
		}
		if (baitUnit->getDistance(waitPoint) < 5)
		{
			if (++waitTime > 500)
			{
				baitUnit->move(waitPoint);
				if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(baitUnit->getPosition().x(), baitUnit->getPosition().y(),
					waitPoint.x(), waitPoint.y(), BWAPI::Colors::Purple);
				waitTime = 0;
			}
			return;
		}
		baitUnit->move(waitPoint);
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(baitUnit->getPosition().x(), baitUnit->getPosition().y(),
			waitPoint.x(), waitPoint.y(), BWAPI::Colors::Purple);
		return;
	}
	else if (baitUnit->isUnderAttack())
	{
		runToNext(baitUnit);
	}
}

void BaitManager::onUnitDestroy(BWAPI::Unit * unit)
{
	if (unit == NULL)
	{
		assert(false);
	}
	// if the unit that was destroyed is the bait
	if (unit->getID() == baitID)
	{
		BWAPI::Broodwar->printf("Bait Unit Destroyed");
		//bait unit stays dead
		//--numBaitUnits; 
		beingChased = false;
		currentDest = 0;
		baitID = 0;
		baitPos = BWAPI::Position(-500, -500);
	}
}