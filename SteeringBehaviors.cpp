//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"


//SEEK
//****
inline SteeringPlugin_Output Seek::CalculateSteeringOutput(float deltaT, AgentInfo agentInfo)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = m_Target - agentInfo.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed
	
	/*if (Distance(m_Target, agentInfo.Position) < 0.5f)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
	}*/

	if (!m_StamineRegenerating)
	{
		steering.RunMode = true;
		if (agentInfo.Stamina < 0.1f)
		{
			m_StamineRegenerating = true;
		}
	}
	else if (agentInfo.Stamina > 9.5 || (agentInfo.Health < 2.f && agentInfo.Stamina > 2.f))
	{
		m_StamineRegenerating = false;
		steering.RunMode = true;
	}

	return steering;
}

//WANDER (base> SEEK)
//******

inline SteeringPlugin_Output Wander::CalculateSteeringOutput(float deltaT, AgentInfo agentInfo)
{
	SteeringPlugin_Output steering = {};
	//SetTarget
	m_AngleChangeTimer += deltaT;
	if (m_AngleChangeTimer >= (0.1f * m_NrWanderers))
	{
		m_WanderAngle += (m_AngleChange * (randomFloat(2.f) - 1.f));
		if (m_WanderAngle > 2 * float(M_PI))m_WanderAngle -= 2 * float(M_PI);
		if (m_WanderAngle < 0.f)m_WanderAngle += 2 * float(M_PI);
		m_AngleChangeTimer = 0.f;
	}

	Elite::Vector2 randomCirclePoint{};
	randomCirclePoint = agentInfo.Position + m_Offset * agentInfo.LinearVelocity.GetNormalized();
	randomCirclePoint.x += m_Radius * cos(m_WanderAngle);
	randomCirclePoint.y += m_Radius * sin(m_WanderAngle);
	m_Target = randomCirclePoint;

	steering.LinearVelocity = m_Target - agentInfo.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	return steering;
}

//Flee
//******
inline SteeringPlugin_Output Flee::CalculateSteeringOutput(float deltaT, AgentInfo agentInfo)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = m_Target - agentInfo.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;
	steering.LinearVelocity *= -1; 

	if (!m_StamineRegenerating)
	{
		steering.RunMode = true;
		if (agentInfo.Stamina < 0.1f)
		{
			m_StamineRegenerating = true;
		}
	}
	else if (agentInfo.Stamina > 9.5 || (agentInfo.Health < 2.f && agentInfo.Stamina > 2.f))
	{
		m_StamineRegenerating = false;
		steering.RunMode = true;
	}

	return steering;
}

inline void Face::SetRotation(int rot)
{
	m_Rotation = rot;
}

inline int Face::GetRotation() const
{
	return m_Rotation;
}

inline SteeringPlugin_Output Face::CalculateSteeringOutput(float deltaT, AgentInfo agentInfo)
{
	SteeringPlugin_Output steering = {};
	float angleDiffRange{ 2.f };

	Elite::Vector2 velocity = m_Target - agentInfo.Position;
	float desiredOrientation = Elite::GetOrientationFromVelocity(velocity) * 180.f / float(M_PI);
	float agentOrientation = agentInfo.Orientation * 180.f / float(M_PI);
	while (agentOrientation > 360.f)agentOrientation -= 360.f;
	while (agentOrientation < 0.f)agentOrientation += 360.f;

	//fix 3th and 4th quadrant
	if (desiredOrientation < 0)desiredOrientation += 360.f;

	//std::cout << "desiredOrientation: " << desiredOrientation << std::endl;
	//std::cout << "agentOrientation: " << agentOrientation << std::endl;

	//if rotation = 0 caculate rotation with target
	if (m_Rotation == 0)
	{
		float diff{ desiredOrientation - agentOrientation };
		if (fabs(diff) > angleDiffRange)
		{
			if (diff > 0.f)
			{
				if (diff < 180.f)m_Rotation = 1;
				else { m_Rotation = -1; }
			}
			else
			{
				if (diff > -180.f)m_Rotation = -1;
				else { m_Rotation = 1; }
			}
			//steering.LinearVelocity = velocity * -1;
		}
	}
	steering.AngularVelocity = agentInfo.MaxAngularSpeed * m_Rotation;
	steering.AutoOrient = false;
	return steering;
}
