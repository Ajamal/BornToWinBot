#pragma once;

#include <Common.h>
//#include "micromanagement/MicroManager.h"

class BaitManager;

class BaitManager 
{

public:

	BaitManager();
	~BaitManager() {}
	//void executeMicro(const UnitVector & targets);
	void BaitManager::initialize_map_points(std::string mapName);
	
	void update(const std::set<BWAPI::Unit *> & baitUnits);
	
	void moveBait(BWAPI::Unit * baitUnit);
	
	//true if unit believes it is being chased
	bool beingChased;
	
	//index of the current chase position to run to
	int currentDest;

	//send the bait to the next destination if it is being chased
	void runToNext(BWAPI::Unit * baitUnit);

	//return true if an enemy is near
	bool nearbyEnemies(BWAPI::Unit * baitUnit);

	std::vector<BWAPI::Position> waitPoints;
	std::vector<BWAPI::Position> chasePoints;
};