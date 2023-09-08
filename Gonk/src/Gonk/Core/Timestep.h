#pragma once

namespace Gonk {

	class Timestep 
	{
	public:
		Timestep(float time) : m_Time(time) {}

		operator float() { return m_Time; }

		float GetSeconds() const { return m_Time; }
		float GetMilliSeconds() const { return m_Time * 1000; }

	private:
		float m_Time;
	};

}
