#pragma once;

#include <Common.h>
//#include "micromanagement/MicroManager.h"

class BaitManager;

class BaitManager 
{

public:
	BaitManager();
	~BaitManager() {}

	int				numBaitUnits;
	bool			hasBaited;
	bool			mapLoaded;
	int				baitID;
	int				waitTime;
	BWAPI::Position	baitPos;

	static	BaitManager & Instance();
	void	update(const std::set<BWAPI::Unit *> & baitUnits);
	void	BaitManager::initialize_map_points(std::string mapName);
	void	BaitManager::onUnitDestroy(BWAPI::Unit * unit);
	
private:
	//move the unit to the designated bait point
	void	 moveBait(BWAPI::Unit * baitUnit);
	
	//true if unit believes it is being chased
	bool	beingChased;
	
	//index of the current chase position to run to

	//the last unit to been seen near the bait unit
	BWAPI::Unit *	lastSeen;

	size_t	currentDest;

	//send the bait to the next destination if it is being chased
	void	runToNext(BWAPI::Unit * baitUnit);

	//return true if an enemy is near
	bool	nearbyEnemies(BWAPI::Unit * baitUnit);

	BWAPI::Position waitPoint;
	//std::vector<BWAPI::Position> waitPoints;
	std::vector<BWAPI::Position> chasePoints;
};