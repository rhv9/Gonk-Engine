#pragma once

#include "entt.hpp"
#include "Gonk/Core/Timestep.h"

namespace Gonk {

	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		// temp
		entt::registry& Reg() { return m_Registry; }

		Entity CreateEntity(const std::string& name = std::string("Entity"));

		void OnUpdate(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);
	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;
	};

}