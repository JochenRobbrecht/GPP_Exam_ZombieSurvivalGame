#pragma once
#include "IExamPlugin.h"
#include "StatesAndTransitions.cpp"


class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;

	Elite::Vector2 m_Target = {};
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose

	//steering
	ISteeringBehavior* m_pCurrentSteering = nullptr;
	
	//Decision making 
	Elite::FiniteStateMachine* m_pFSM = nullptr;
	std::vector<Elite::FSMState*> m_pStates{};
	std::vector<Elite::FSMTransition*> m_pTransitions{};
	
	//extra
	int m_NrPistols = 0;
	float m_LastFrameHp = 0;
	Blackboard* pBlackBoard = nullptr;
	std::vector<bool> m_InvertoryBools{false,false,false,false,false};
	void ItemHandling(const AgentInfo& agentInfo);
	bool CheckUseItem(const AgentInfo& agentInfo, ItemInfo& itemInfo, UINT index);
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}