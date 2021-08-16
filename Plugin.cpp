#include "stdafx.h"
#include "Plugin.h"


//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);
	
	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "BotNameTEST";
	info.Student_FirstName = "Jochen";
	info.Student_LastName = "Robbrecht";
	info.Student_Class = "2DAE02";
	
	m_LastFrameHp = m_pInterface->Agent_GetInfo().Health;
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
	std::srand(time(nullptr));

	Blackboard* pBlackBoard =  new Blackboard();
	
	std::deque<Elite::Vector2> lastLootedHousePositions{};
	pBlackBoard->AddData("lastLootedHousePositions", lastLootedHousePositions);
	//states
	WanderState* pWanderState{ new WanderState() };
	m_pStates.push_back(pWanderState);
	SeekItemState* pSeekItemState{ new SeekItemState() };
	m_pStates.push_back(pSeekItemState);
	SeekGoalState* pSeekGoalState{ new SeekGoalState() };
	m_pStates.push_back(pSeekGoalState);
	FaceState* pFaceState{ new FaceState() };
	m_pStates.push_back(pFaceState);
	FleeState* pFleeState{ new FleeState() };
	m_pStates.push_back(pFleeState);

	//fsm
	m_pFSM = new FiniteStateMachine(pWanderState, pBlackBoard);

	//transitions
	ItemInSight* pItemInSight{ new ItemInSight() };
	NoItemInSight* pNoItemInSight{ new NoItemInSight() };
	NothingInSight* pNothingInSight{ new NothingInSight() };
	//SeeAndCanGoInHouseForFlee* pSeeAndCanGoInHouseForFlee{ new SeeAndCanGoInHouseForFlee() };
	//SeeAndCanGoInHouseForLoot* pSeeAndCanGoInHouseForLoot{ new SeeAndCanGoInHouseForLoot() };
	SeeAndCanGoOutHouseIfLooted* pSeeAndCanGoOutHouseIfLooted{ new SeeAndCanGoOutHouseIfLooted() };
	GoalReached* pGoalReachted{ new GoalReached() };
	EnemiesNearAndHasPistol* pEnemiesNearAndHasPistol{ new EnemiesNearAndHasPistol() };
	NoEnemiesNear* pNoEnemiesNear{ new NoEnemiesNear() };
	NoPistolsLeft* pNoPistolsLeft{ new NoPistolsLeft() };
	//EnemiesNearNoPistolNoHouseVis* pEnemiesNearNoPistolNoHouseVis{ new EnemiesNearNoPistolNoHouseVis() };
	//FleeFinished* pFleeFinished{ new FleeFinished() };
	//PurgeZoneDetected* pPurgeZoneDetected{ new PurgeZoneDetected() };
	//InPurgeZone* pInPurgeZone{ new InPurgeZone() };

	m_pTransitions.push_back(pItemInSight);
	m_pTransitions.push_back(pNoItemInSight);
	m_pTransitions.push_back(pNothingInSight);
	//m_pTransitions.push_back(pSeeAndCanGoInHouseForFlee); 
	//m_pTransitions.push_back(pSeeAndCanGoInHouseForLoot);
	m_pTransitions.push_back(pSeeAndCanGoOutHouseIfLooted);
	m_pTransitions.push_back(pGoalReachted);
	m_pTransitions.push_back(pEnemiesNearAndHasPistol);
	m_pTransitions.push_back(pNoEnemiesNear);
	m_pTransitions.push_back(pNoPistolsLeft);
	//m_pTransitions.push_back(pEnemiesNearNoPistolNoHouseVis);
	//m_pTransitions.push_back(pFleeFinished);
	//m_pTransitions.push_back(pPurgeZoneDetected);
	//m_pTransitions.push_back(pInPurgeZone);

	//wander to seek and reverse
	m_pFSM->AddTransition(pWanderState, pSeekItemState, pItemInSight);
	m_pFSM->AddTransition(pSeekItemState, pWanderState, pNoItemInSight);

	//from wander/flee to seekToGoal if houseInSight&& notInHouse (goal center of house)
	//m_pFSM->AddTransition(pWanderState, pSeekGoalState, pSeeAndCanGoInHouseForLoot);
	//m_pFSM->AddTransition(pFleeState, pSeekGoalState, pSeeAndCanGoInHouseForFlee);

	//from seekgoal to seekitem if iteminsight and agent is in house
	//m_pFSM->AddTransition(pSeekGoalState, pSeekItemState, pItemInSight);
	
	//wander to seekToGoal if inHouse && elapsedTimeWander > minWanderTime (center + size of house +...)
	m_pFSM->AddTransition(pWanderState, pSeekGoalState, pSeeAndCanGoOutHouseIfLooted);

	//from seekToGoal to wander if dist(goal,pos) < 2 
	m_pFSM->AddTransition(pSeekGoalState, pWanderState, pGoalReachted);

	//from all states to face(attack mode)
	m_pFSM->AddTransition(pWanderState, pFaceState, pEnemiesNearAndHasPistol);
	m_pFSM->AddTransition(pSeekGoalState, pFaceState, pEnemiesNearAndHasPistol);
	m_pFSM->AddTransition(pSeekItemState, pFaceState, pEnemiesNearAndHasPistol);
	m_pFSM->AddTransition(pFleeState, pFaceState, pEnemiesNearAndHasPistol);

	//from face to wander
	m_pFSM->AddTransition(pFaceState, pWanderState, pNoEnemiesNear);
	m_pFSM->AddTransition(pFaceState, pWanderState, pNoPistolsLeft);

	//to flee if not house vis and no pistol and enemies near
	//m_pFSM->AddTransition(pWanderState, pFleeState, pEnemiesNearNoPistolNoHouseVis);
	//m_pFSM->AddTransition(pSeekGoalState, pFleeState, pEnemiesNearNoPistolNoHouseVis);


	//if purgezone detected and we are outside go to face
	//m_pFSM->AddTransition(pWanderState, pFaceState, pPurgeZoneDetected);
	//m_pFSM->AddTransition(pFleeState, pFaceState, pPurgeZoneDetected);
	//m_pFSM->AddTransition(pSeekItemState, pFaceState, pPurgeZoneDetected);
	//m_pFSM->AddTransition(pSeekGoalState, pFaceState, pPurgeZoneDetected);

	//if purgezone detected and we are in it seekToGoal outside;
	//m_pFSM->AddTransition(pWanderState, pSeekGoalState, pInPurgeZone);
	//m_pFSM->AddTransition(pFleeState, pSeekGoalState, pInPurgeZone);
	//m_pFSM->AddTransition(pSeekItemState, pSeekGoalState, pInPurgeZone);
	//m_pFSM->AddTransition(pFaceState, pSeekGoalState, pInPurgeZone);
	//m_pFSM->AddTransition(pSeekGoalState, pSeekGoalState, pInPurgeZone);

	//flee to wander after .. seconds not being attacked
	//m_pFSM->AddTransition(pFleeState, pWanderState, pFleeFinished);

}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded

	SAFE_DELETE(m_pFSM);
	for (auto& s : m_pStates)
	{
		SAFE_DELETE(s);
	}
	for (auto& t : m_pTransitions)
	{
		SAFE_DELETE(t);
	}
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		//m_CanRun = true;
		std::cout << "inventorybools: " << std::endl;
		for (bool i : m_InvertoryBools)
			std::cout << std::boolalpha << i << ", ";
		std::cout << std::endl;
		std::cout << "inventory: " << std::endl;
		for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
		{
			ItemInfo itemInfo{};
			std::cout << std::boolalpha << m_pInterface->Inventory_GetItem(i, itemInfo);
		}
		std::cout << std::endl;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}	
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	auto steeringOutput = SteeringPlugin_Output();

	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	auto agentInfo = m_pInterface->Agent_GetInfo();
	auto worldInfo = m_pInterface->World_GetInfo();

	/*for (auto& e : vEntitiesInFOV)
	{
		if (e.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo zoneInfo;
			m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
			std::cout << "Purge Zone in FOV:" << e.Location.x << ", "<< e.Location.y <<  " ---EntityHash: " << e.EntityHash << "---Radius: "<< zoneInfo.Radius << std::endl;
		}
	}*/
	
	//update Blackboard
	m_pFSM->GetBlackboard()->AddData("agentInfo", agentInfo);
	m_pFSM->GetBlackboard()->AddData("worldInfo", worldInfo);
	m_pFSM->GetBlackboard()->AddData("housesInSight", GetHousesInFOV());
	std::vector<EnemyInfo> enemies{};
	std::vector<PurgeZoneInfo> purgeZones{};

	for (const EntityInfo& entityInfo : GetEntitiesInFOV())
	{
		if (entityInfo.Type == eEntityType::ENEMY)
		{
			EnemyInfo enemyInfo;
			m_pInterface->Enemy_GetInfo(entityInfo, enemyInfo);
			enemies.push_back(enemyInfo);
		}
		if (entityInfo.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo purgeInfo;
			m_pInterface->PurgeZone_GetInfo(entityInfo, purgeInfo);
			purgeZones.push_back(purgeInfo);
		}
	}
	m_pFSM->GetBlackboard()->AddData("enemiesInSight", enemies);
	m_pFSM->GetBlackboard()->AddData("purgeZonesInSight", purgeZones);

	Elite::Vector2 goal{};
	bool dataAvailable = m_pFSM->GetBlackboard()->GetData("currentGoal", goal,false);
	if (dataAvailable)
	{
		m_pFSM->GetBlackboard()->AddData("nextPathPos", m_pInterface->NavMesh_GetClosestPathPoint(goal));
	}

	ItemHandling(agentInfo);
	m_pFSM->GetBlackboard()->AddData("nrPistols", m_NrPistols);

	//if lose hp since last frame, not because of energy(diff in hp > 0.2f)
	if (m_LastFrameHp != agentInfo.Health && (m_LastFrameHp - agentInfo.Health) > 0.2f)
	{
		m_LastFrameHp = agentInfo.Health;
		m_pFSM->GetBlackboard()->AddData("isAttacked", static_cast<bool>(true));
	}
	else
	{
		m_LastFrameHp = agentInfo.Health;//in case of hp loss due too energy
		m_pFSM->GetBlackboard()->AddData("isAttacked", static_cast<bool>(false));
	}

	m_pFSM->Update(dt);
	
	m_pFSM->GetBlackboard()->GetData("currentSteering", m_pCurrentSteering);
	steeringOutput = m_pCurrentSteering->CalculateSteeringOutput(dt, agentInfo);
	return steeringOutput;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

void Plugin::ItemHandling(const AgentInfo& agentInfo)
{
	std::vector<EntityInfo> entitiesInSight{ GetEntitiesInFOV() };

	std::vector<ItemInfo> itemsInSight{};

	bool usedItemArleady{};

	//count pistols
	m_NrPistols = 0;
	for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
	{
		if (m_InvertoryBools[i] == false)continue;
		ItemInfo currentSlotItemInfo{};
		m_pInterface->Inventory_GetItem(i, currentSlotItemInfo);

		if (currentSlotItemInfo.Type == eItemType::PISTOL)m_NrPistols++;
	}

	for (const EntityInfo& entity : entitiesInSight)
	{
		ItemInfo itemInfo{};
		if (entity.Type == eEntityType::ITEM)
		{
			m_pInterface->Item_GetInfo(entity, itemInfo);
			if (itemInfo.Type != eItemType::GARBAGE)
			{
				//decide if we need item
				bool skipItem{ false };
				for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
				{
					if (m_InvertoryBools[i] == false)continue;
					ItemInfo currentSlotItemInfo{};
					m_pInterface->Inventory_GetItem(i, currentSlotItemInfo);

					if (currentSlotItemInfo.Type == eItemType::FOOD && itemInfo.Type == eItemType::FOOD)
					{
						if (m_pInterface->Food_GetEnergy(currentSlotItemInfo) <= m_pInterface->Food_GetEnergy(itemInfo)
							&& agentInfo.Energy < 9.f)
						{
							m_pInterface->Inventory_UseItem(i);
							m_pInterface->Inventory_RemoveItem(i);
							m_InvertoryBools[i] = false;
							usedItemArleady = true;
						}
						else
						{//store pos?
							skipItem = true;
							break;
						}
					}
					else if (currentSlotItemInfo.Type == eItemType::MEDKIT && itemInfo.Type == eItemType::MEDKIT)
					{
						if (agentInfo.Health < 9.5f)
						{
							m_pInterface->Inventory_UseItem(i);
							m_pInterface->Inventory_RemoveItem(i);
							m_InvertoryBools[i] = false;
							usedItemArleady = true;
						}
						else
						{//store pos?
							skipItem = true;
							break;
						}
					}
				}
				if (m_NrPistols == 3 && itemInfo.Type == eItemType::PISTOL)skipItem = true;
				if (skipItem)continue;
				if (Elite::Distance(itemInfo.Location, agentInfo.Position) <= agentInfo.GrabRange)
				{
					bool itemGrabbed = m_pInterface->Item_Grab(entity, itemInfo);

					if (itemGrabbed)
					{
						bool itemAdded{};
						for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
						{
							if (m_InvertoryBools[i] == false)
							{
								itemAdded = m_pInterface->Inventory_AddItem(i, itemInfo);
								m_InvertoryBools[i] = true;
								if (!itemAdded)std::cout << "failed to add item, even thought there is inventory slot open\n";
								break;
							}
						}
						if (!itemAdded)
						{
							std::cout << "no space found in inventory: " << std::endl;
							for (bool i : m_InvertoryBools)
								std::cout << i << ", ";
							std::cout << std::endl;
						}
					}
				}
				else
				{
					itemsInSight.push_back(itemInfo);
				}
			}
		}
	}

	if (!usedItemArleady)
	{
		ItemInfo itemInfo{};
		for (UINT i = 0; i < m_pInterface->Inventory_GetCapacity(); i++)
		{
			if (m_InvertoryBools[i] == false)continue;
			m_pInterface->Inventory_GetItem(i, itemInfo);
			if (CheckUseItem(agentInfo, itemInfo, i) == true)break;
		}
	}

	m_pFSM->GetBlackboard()->AddData("itemsInSight", itemsInSight);
	m_pFSM->GetBlackboard()->AddData("nrItemsInSight", itemsInSight.size());
}

bool Plugin::CheckUseItem(const AgentInfo& agentInfo, ItemInfo& itemInfo, UINT index)
{
	bool usedItem{ false };
	if (itemInfo.Type == eItemType::FOOD)
	{
		if (10.f - agentInfo.Energy >= m_pInterface->Food_GetEnergy(itemInfo))
		{
			usedItem = m_pInterface->Inventory_UseItem(index);
			m_pInterface->Inventory_RemoveItem(index);
			m_InvertoryBools[index] = false;
		}
	}
	else if (itemInfo.Type == eItemType::MEDKIT)
	{
		if (10.f - agentInfo.Health >= m_pInterface->Medkit_GetHealth(itemInfo))
		{
			usedItem = m_pInterface->Inventory_UseItem(index);
			m_pInterface->Inventory_RemoveItem(index);
			m_InvertoryBools[index] = false;
		}
	}
	else if (itemInfo.Type == eItemType::PISTOL)
	{
		
		std::vector<EnemyInfo> enemies;
		m_pFSM->GetBlackboard()->GetData("enemiesInSight", enemies);
		if (enemies.empty())return usedItem;

		bool shootableEnemy{false};
		float angleDiffRange{ 2.f };
		for (const EnemyInfo& enemyInfo : enemies)
		{
			Elite::Vector2 velocity = enemyInfo.Location - agentInfo.Position;
			float desiredOrientation = Elite::GetOrientationFromVelocity(velocity) * 180.f / float(M_PI);
			float agentOrientation = agentInfo.Orientation * 180.f / float(M_PI);
			while (agentOrientation > 360.f)agentOrientation -= 360.f;
			while (agentOrientation < 0.f)agentOrientation += 360.f;

			//fix 3th and 4th quadrant
			if (desiredOrientation < 0)desiredOrientation += 360.f;

			if (fabs(desiredOrientation - agentOrientation) < angleDiffRange)
			{
				shootableEnemy = true;
			}
		}
		if (shootableEnemy)
		{
			usedItem = m_pInterface->Inventory_UseItem(index);
			if (m_pInterface->Weapon_GetAmmo(itemInfo) == 0)
			{
				m_pInterface->Inventory_RemoveItem(index);
				m_InvertoryBools[index] = false;
				m_NrPistols--;
			}
		}
	}
	return usedItem;
}
