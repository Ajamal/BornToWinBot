#include "Common.h"
#include "BaitManager.h"
#include "InformationManager.h"
#include "MicroManager.h"

BaitManager::BaitManager() : beingChased(false), currentDest(0)
{ 
	//std::string mapName = BWAPI::Broodwar->mapName();
	initialize_map_points(BWAPI::Broodwar->mapFileName());
	
}

/*
void BaitManager::executeMicro(const UnitVector & targets)
{
	const UnitVector & baitUnits = getUnits();

	// for each baitUnit
	BOOST_FOREACH(BWAPI::Unit * baitUnit, baitUnits)
	{

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG)
		{
			BWAPI::Broodwar->drawLineMap(baitUnit->getPosition().x(), baitUnit->getPosition().y(),
				baitUnit->getTargetPosition().x(), baitUnit->getTargetPosition().y(), Options::Debug::COLOR_LINE_TARGET);
		}
	}
}
*/

void BaitManager::initialize_map_points(std::string mapName)
{
	if (mapName == "(2)Benzene.scx"){
		BWAPI::Broodwar->printf("Found Map name Benzene!");
		waitPoints.push_back(BWAPI::Position(400, 1953));
		waitPoints.push_back(BWAPI::Position(1800, 1291));
		waitPoints.push_back(BWAPI::Position(3180, 1790));

		chasePoints.push_back(BWAPI::Position(1410, 795));
		chasePoints.push_back(BWAPI::Position(945, 795));
		chasePoints.push_back(BWAPI::Position(980, 490));
		chasePoints.push_back(BWAPI::Position(1405, 290));
		chasePoints.push_back(BWAPI::Position(1980, 500));
	}
	else
	{
		BWAPI::Broodwar->printf("Following map filename not recognized");
		BWAPI::Broodwar->printf(&(BWAPI::Broodwar->mapFileName())[0]);
	}
}

void BaitManager::update(const std::set<BWAPI::Unit *> & baitUnits)
{
	if (baitUnits.size() == 1){
		BWAPI::Unit * baitUnit = *baitUnits.begin();
		moveBait(baitUnit);
	}
}
bool BaitManager::nearbyEnemies(BWAPI::Unit * baitUnit)
{
	UnitVector enemyNear;
	MapGrid::Instance().GetUnits(enemyNear, baitUnit->getPosition(), 100, false, true);

	if (enemyNear.size() > 0) return true;
	else { return false; }
}

void BaitManager::runToNext(BWAPI::Unit * baitUnit)
{
	beingChased = true;
	if (currentDest > chasePoints.size() - 1) currentDest = -1;
	baitUnit->move(chasePoints[currentDest++]);
}

void BaitManager::moveBait(BWAPI::Unit * baitUnit)
{
	//if at a chasepoint run to the next one if enemies are nearby
	if ((baitUnit->getPosition().x() > chasePoints[currentDest].x() - 2) && (baitUnit->getPosition().x() < chasePoints[currentDest].x() + 2) 
		&& (baitUnit->getPosition().y() > chasePoints[currentDest].y() - 2) && (baitUnit->getPosition().y() < chasePoints[currentDest].y() + 2))
	{
		if (nearbyEnemies(baitUnit)) runToNext(baitUnit);
		return;
	}

	//wait for pursuers to catch up or continue running
	if (beingChased)
	{
		if (!nearbyEnemies(baitUnit))
		{
			baitUnit->holdPosition(true);
		}
		else if (baitUnit->isHoldingPosition())
		{
			baitUnit->move(chasePoints[currentDest]);
		}
		return;
	}

	//if not being chased head to the nearest wait point
	if (!beingChased){

		if (nearbyEnemies(baitUnit))
		{
			runToNext(baitUnit);
			return;
		}

		double minDistance = 99999999999999;
		BWAPI::Position closestWait;

		BOOST_FOREACH(BWAPI::Position wait, waitPoints)
		{
			double distance = baitUnit->getDistance(wait);
			if (distance < minDistance)
			{
				minDistance = distance;
				closestWait = wait;
			}
		}
		baitUnit->move(closestWait);

		//BWAPI::Broodwar->printf("(%d, %d)",baitUnit->getPosition().x(),baitUnit->getPosition().y());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(baitUnit->getPosition().x(), baitUnit->getPosition().y(), "Bait");
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(closestWait.x(), closestWait.y(), "WaitPosition");
		BWAPI::Broodwar->drawLineMap(baitUnit->getPosition().x(), baitUnit->getPosition().y(),
			closestWait.x(), closestWait.y(),
			BWAPI::Colors::Purple);
	}
}

