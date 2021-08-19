/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/

// -- - Includes-- -
#include "EfiniteStateMachine.cpp"
//#include "IExamInterface.h"
#include "SteeringBehaviors.cpp"

//functions
bool IsInHouse(const HouseInfo& houseInfo, const AgentInfo& agentInfo);
Elite::Vector2 GetClosestItemPos(const std::vector<ItemInfo>& items, const AgentInfo& agentInfo);
Elite::Vector2 GetClosestPosToWorldCenter(const WorldInfo& worldInfo, const HouseInfo& houseInfo);
bool SeeAndCanGoOutHouse(Elite::Blackboard* pBlackBoard);
bool SeeAndCanGoInHouseF(Elite::Blackboard* pBlackBoard);
//--STATES--
//----------
//store positions of previous houses we went it?
//store useful items we didnt pick up?
//make flee behavior(when seeing enemies or getting attacked)/ face behaviour + shoot (discard gun if empty ammo), set invbool to false
//if while fleeing we see house, also go in it(even if looted)
//check if not moving oustide of bounds
	
class SeekItemState : public Elite::FSMState
{
public:
	SeekItemState() : Elite::FSMState()
		, m_pSeek{ new Seek() } {}

	~SeekItemState()
	{
		SAFE_DELETE(m_pSeek);
	}
	virtual void OnEnter(Elite::Blackboard* pBlackBoard) override
	{
		//set correct target
		pBlackBoard->GetData("nrItemsInSight", m_NrItemsInSight);
		Elite::Vector2 itemPos;
		pBlackBoard->GetData("itemInSightPos", itemPos);
		m_pSeek->SetTarget(itemPos);

		ISteeringBehavior* newSteering{ m_pSeek };
		pBlackBoard->AddData("currentSteering", newSteering);
	}
	virtual void Update(Elite::Blackboard* pBlackBoard, float deltaTime) override
	{
		size_t nrItemsInSight;
		pBlackBoard->GetData("nrItemsInSight", nrItemsInSight);
		//if an item was picked up or destroyed, reset target to next item in sight
		if (nrItemsInSight != m_NrItemsInSight)
		{
			m_NrItemsInSight = nrItemsInSight;
			std::vector<ItemInfo> itemsInSight;
			pBlackBoard->GetData("itemsInSight", itemsInSight);
			AgentInfo agentInfo;
			pBlackBoard->GetData("agentInfo", agentInfo);
			if (!itemsInSight.empty())
			{
				Elite::Vector2 closestItem = GetClosestItemPos(itemsInSight, agentInfo);
				m_pSeek->SetTarget(closestItem);
			}
		}
	}
private:
	Seek* m_pSeek = nullptr;
	size_t m_NrItemsInSight = 0;
};

class SeekGoalState : public Elite::FSMState
{
public:
	SeekGoalState() : Elite::FSMState()
		, m_pSeek{ new Seek() } {}

	~SeekGoalState()
	{
		SAFE_DELETE(m_pSeek);
	}
	virtual void OnEnter(Elite::Blackboard* pBlackBoard) override
	{
		//set goal
		pBlackBoard->GetData("currentGoal", m_Goal);

		ISteeringBehavior* newSteering{ m_pSeek };
		pBlackBoard->AddData("currentSteering", newSteering);
		//set timer to zero
	}
	virtual void Update(Elite::Blackboard* pBlackBoard, float deltaTime) override
	{
		Elite::Vector2 newPathPos{};
		pBlackBoard->GetData("nextPathPos", newPathPos, false);//state gets updated right after creation so set warning to false...
		if (newPathPos != m_NextPathPos)
		{
			m_NextPathPos = newPathPos;
			//std::cout << "nextPathPos: " << m_NextPathPos.x << ", " << m_NextPathPos.y << std::endl;
			m_pSeek->SetTarget(m_NextPathPos);
		}
	}
private:
	Seek* m_pSeek = nullptr;
	Elite::Vector2 m_Goal;
	Elite::Vector2 m_NextPathPos;
};

class WanderState : public Elite::FSMState
{
public:
	WanderState() : Elite::FSMState()
		, m_pWander{ new Wander() } {}
	
	~WanderState()
	{
		SAFE_DELETE(m_pWander);
	}
	virtual void OnEnter(Elite::Blackboard* pBlackBoard) override
	{
		ISteeringBehavior* newSteering{ m_pWander };
		pBlackBoard->AddData("currentSteering", newSteering);
		m_ElapsedSec = 0;
	}
	virtual void Update(Elite::Blackboard* pBlackBoard, float deltaTime) override
	{
		m_ElapsedSec += deltaTime;
		pBlackBoard->AddData("elapsedTimeWandering", m_ElapsedSec);
	}

private:
	Wander* m_pWander = nullptr;
	float m_ElapsedSec = 0;
};

class FleeState : public Elite::FSMState
{
public:
	FleeState() : Elite::FSMState()
		, m_pFlee{ new Flee() } {}

	~FleeState()
	{
		SAFE_DELETE(m_pFlee);
	}
	virtual void OnEnter(Elite::Blackboard* pBlackBoard) override
	{
		pBlackBoard->AddData("currentSteering", m_pFlee);
	}

private:
	Flee* m_pFlee = nullptr;
};

class FaceState : public Elite::FSMState
{
public:
	FaceState() : Elite::FSMState()
		, m_pFace{ new Face() } {}

	~FaceState()
	{
		SAFE_DELETE(m_pFace);
	}
	virtual void OnEnter(Elite::Blackboard* pBlackBoard) override
	{
		m_ElapsedTimeSinceLastAttack = 0;

		ISteeringBehavior* newSteering{ m_pFace };
		pBlackBoard->AddData("currentSteering", newSteering);
	}
	virtual void Update(Elite::Blackboard* pBlackBoard, float deltaTime) override
	{
		m_ElapsedTimeSinceLastAttack += deltaTime;
		bool isAttacked{};
		pBlackBoard->GetData("isAttacked", isAttacked);
		if (isAttacked)m_ElapsedTimeSinceLastAttack = 0;
		pBlackBoard->AddData("elapsedTimeSinceLastAttack", m_ElapsedTimeSinceLastAttack);

		std::vector<EnemyInfo> enemies;
		pBlackBoard->GetData("enemiesInSight", enemies);
		if (enemies.empty())
		{
			//if not zero we know we are rotating to enemy
			if(m_pFace->GetRotation() == 0)m_pFace->SetRotation(1);
		}
		else
		{
			//get closest enemy(not distance but smallest angle diff)
			m_pFace->SetTarget(enemies[0].Location);
			m_pFace->SetRotation(0);
		}
	}

private:
	Face* m_pFace = nullptr;
	float m_ElapsedTimeSinceLastAttack = 0;
};

//--TRANSITIONS--
//---------------
class NoEnemiesNear : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackBoard)const override
	{
		float elapsedTimeSinceLastAttack{}, maxTimeFacingNoAttacks{5.f};
		pBlackBoard->GetData("elapsedTimeSinceLastAttack", elapsedTimeSinceLastAttack);
		if (elapsedTimeSinceLastAttack >= maxTimeFacingNoAttacks)return true;
		return false;
	}
};

class NoPistolsLeft : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackBoard)const override
	{
		int nrPistols{ };
		pBlackBoard->GetData("nrPistols", nrPistols);
		if (nrPistols == 0)return true;
		return false;
	}
};
class EnemiesNearAndHasPistol : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackBoard)const override
	{
		int nrPistols{ };
		pBlackBoard->GetData("nrPistols", nrPistols);
		/*std::cout << nrPistols << std::endl;*/
		if (nrPistols == 0)return false;

		bool isAttacked{};
		pBlackBoard->GetData("isAttacked", isAttacked);
		if (isAttacked)return true;
		
		std::vector<EnemyInfo> enemies;
		pBlackBoard->GetData("enemiesInSight", enemies);
		if (!enemies.empty())return true;

		return false;
	}
};



class SeeAndCanGoInHouse : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackBoard)const override
	{
		return SeeAndCanGoInHouseF(pBlackBoard);
	}

};

class SeeAndCanGoOutHouseIfLooted : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackBoard)const override
	{
		float elapsedTimeWandering{};
		pBlackBoard->GetData("elapsedTimeWandering", elapsedTimeWandering, false);
		if (elapsedTimeWandering < m_MinWanderTime)return false;
		return SeeAndCanGoOutHouse(pBlackBoard);
	}
private:
	float m_MinWanderTime = 6;
};

class ItemInSight : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackBoard)const override
	{
		std::vector<ItemInfo> itemsInSight;
		pBlackBoard->GetData("itemsInSight", itemsInSight);
		AgentInfo agentInfo;
		pBlackBoard->GetData("agentInfo", agentInfo);

		if (!itemsInSight.empty())
		{
			Elite::Vector2 closestItem = GetClosestItemPos(itemsInSight,agentInfo);
			pBlackBoard->AddData("itemInSightPos", closestItem);
			return true;
		}
		return false;
	}
};
class NothingInSight : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackBoard)const override
	{
		//entities
		std::vector<EntityInfo> entitiesInSight;
		pBlackBoard->GetData("entitiesInSight", entitiesInSight);
		if (!entitiesInSight.empty())return false;

		//houses
		std::vector<HouseInfo> housesInSight;
		pBlackBoard->GetData("housesInSight", housesInSight);
		if (!housesInSight.empty())return false;

		//no entities nor houses in sight
		return true;
	}
};
class NoItemInSight : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackBoard)const override
	{
		std::vector<ItemInfo> itemsInSight;
		pBlackBoard->GetData("itemsInSight", itemsInSight);
		if (itemsInSight.empty())
		{
			return true;
		}
		return false;
	}
};
class GoalReached : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackBoard)const override
	{
		Elite::Vector2 goal;
		AgentInfo agentInfo;
		pBlackBoard->GetData("agentInfo", agentInfo);
		pBlackBoard->GetData("currentGoal", goal);
		if (Elite::Distance(agentInfo.Position, goal) <= 2) return true;
		return false;
	}
};

//--FUNCTION DEFENITIONS--
//---------------
inline bool IsInHouse(const HouseInfo& houseInfo, const AgentInfo& agentInfo)
{
	if ((agentInfo.Position.x >= houseInfo.Center.x - houseInfo.Size.x / 2 && agentInfo.Position.x <= houseInfo.Center.x + houseInfo.Size.x / 2)
		&& (agentInfo.Position.y >= houseInfo.Center.y - houseInfo.Size.y / 2 && agentInfo.Position.y <= houseInfo.Center.y + houseInfo.Size.y / 2))
	{
		return true;
	}
	return false;
}

inline Elite::Vector2 GetClosestItemPos(const std::vector<ItemInfo>& items, const AgentInfo& agentInfo)
{
	Elite::Vector2 closestItemPos{items[0].Location};
	if (items.size() == 1)return  closestItemPos;
	float currentClosestItemDistance{ Elite::Distance(closestItemPos,agentInfo.Position) };

	for (const ItemInfo& item : items)
	{
		float currentItemDistance{ Elite::Distance(item.Location,agentInfo.Position) };
		if (currentItemDistance < currentClosestItemDistance)
		{
			closestItemPos = item.Location;
			currentClosestItemDistance = currentItemDistance;
		}
	}
	return closestItemPos;
}

inline Elite::Vector2 GetClosestPosToWorldCenter(const WorldInfo& worldInfo, const HouseInfo& houseInfo)
{
	Elite::Vector2 closestPos{ houseInfo.Center.x + houseInfo.Size.x, houseInfo.Center.y + houseInfo.Size.y };
	float currentClosestPosDistance{ Elite::Distance(closestPos,worldInfo.Center) };

	Elite::Vector2 currentPos{ houseInfo.Center.x + houseInfo.Size.x, houseInfo.Center.y - houseInfo.Size.y };
	float currentPosDistance{ Elite::Distance(currentPos, worldInfo.Center) };
	if (currentPosDistance < currentClosestPosDistance)
	{
		closestPos = currentPos;
		currentClosestPosDistance = currentPosDistance;
	}

	currentPos = Elite::Vector2 { houseInfo.Center.x - houseInfo.Size.x, houseInfo.Center.y - houseInfo.Size.y };
	currentPosDistance = Elite::Distance(currentPos, worldInfo.Center);
	if (currentPosDistance < currentClosestPosDistance)
	{
		closestPos = currentPos;
		currentClosestPosDistance = currentPosDistance;
	}
	
	currentPos = Elite::Vector2{ houseInfo.Center.x - houseInfo.Size.x, houseInfo.Center.y + houseInfo.Size.y };
	currentPosDistance = Elite::Distance(currentPos, worldInfo.Center);
	if (currentPosDistance < currentClosestPosDistance)
	{
		closestPos = currentPos;
		currentClosestPosDistance = currentPosDistance;
	}

	return closestPos;
}

inline bool SeeAndCanGoOutHouse(Elite::Blackboard* pBlackBoard)
{
	//if house in sight
	std::vector<HouseInfo> housesInSight;
	pBlackBoard->GetData("housesInSight", housesInSight);
	if (!housesInSight.empty())
	{
		//get agentInfo
		AgentInfo agentInfo;
		pBlackBoard->GetData("agentInfo", agentInfo);

		//check if in house
		if (IsInHouse(housesInSight[0], agentInfo))
		{
			WorldInfo worldInfo;
			pBlackBoard->GetData("worldInfo", worldInfo);

			//set goal to pos outside of house
			pBlackBoard->AddData("currentGoal", GetClosestPosToWorldCenter(worldInfo, housesInSight[0]));
			return true;
		}
	}
	return false;
}

inline bool SeeAndCanGoInHouseF(Elite::Blackboard* pBlackBoard)
{
	//if house in sight
	std::vector<HouseInfo> housesInSight;
	pBlackBoard->GetData("housesInSight", housesInSight);
	if (!housesInSight.empty())
	{
		//get agentInfo
		AgentInfo agentInfo;
		pBlackBoard->GetData("agentInfo", agentInfo);
		//check if not in house
		if (!IsInHouse(housesInSight[0], agentInfo))
		{
			//set center of house as goal
			pBlackBoard->AddData("currentGoal", housesInSight[0].Center);
			return true;
		}
	}
	return false;
}