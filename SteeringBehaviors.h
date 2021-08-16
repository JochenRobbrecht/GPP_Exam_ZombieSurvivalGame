/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
//#include "SteeringHelpers.h"
#include "IExamInterface.h"

using namespace Elite;


class ISteeringBehavior 
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteeringOutput(float deltaT, AgentInfo agentInfo) = 0;

	//Seek Functions
	virtual void SetTarget(Vector2 target) { m_Target = target; }

	/*template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }*/

protected:
	Vector2 m_Target;
};


///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	virtual SteeringPlugin_Output CalculateSteeringOutput(float deltaT, AgentInfo agentInfo) override;
private:
	bool m_StamineRegenerating = false;
};

//////////////////////////
//WANDER
//******
class Wander final : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Wander Behavior
	virtual SteeringPlugin_Output CalculateSteeringOutput(float deltaT, AgentInfo agentInfo) override;
protected:
	float m_Offset = 6.f;
	float m_Radius = 4.f;
	float m_AngleChange = ToRadians(45);
	float m_WanderAngle = float(rand() % int(2 * M_PI * 100.f)) / 100.f;
	float m_AngleChangeTimer= 0.f;
	int m_NrWanderers = 0;
};

//////////////////////////
//FLEE
//******

class Flee final : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	// Flee Behavior
	virtual SteeringPlugin_Output CalculateSteeringOutput(float deltaT, AgentInfo agentInfo) override;
private:
	bool m_StamineRegenerating = false;
};

//////////////////////////
//FACE
//******

class Face final : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	void SetRotation(int rot);
	int GetRotation()const;
	// Flee Behavior
	virtual SteeringPlugin_Output CalculateSteeringOutput(float deltaT, AgentInfo agentInfo) override;
private:
	int m_Rotation{0};//either -1,0 or 1
};