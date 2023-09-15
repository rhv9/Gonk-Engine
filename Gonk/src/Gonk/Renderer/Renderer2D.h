#pragma once

#include "RendererCommand.h"
#include "Camera.h"
#include "Gonk/Renderer/Texture.h"

namespace Gonk {

	class Renderer2D
	{
	public:
		static void Init();

		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();
		static void Flush();

		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colour);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, Ref<Texture> texture, const glm::vec4& colour = { 1.0f, 1.0f, 1.0f, 1.0f });

		static void Shutdown();
	};

}