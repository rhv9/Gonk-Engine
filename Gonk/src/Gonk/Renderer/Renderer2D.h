#pragma once

#include "RendererCommand.h"
#include "Camera.h"
#include "Gonk/Renderer/Texture.h"
#include "Gonk/Renderer/SubTexture2D.h"

namespace Gonk {

	class Renderer2D
	{
	public:
		static void Init();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const OrthographicCamera& camera); // TODO: Remove
		static void EndScene();
		static void Flush();

		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colour);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, Ref<Texture> texture, const float tilingFactor = 1.0f, const glm::vec4& colour = { 1.0f, 1.0f, 1.0f, 1.0f });
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, Ref<SubTexture2D> subTexture, const float tilingFactor = 1.0f, const glm::vec4& colour = { 1.0f, 1.0f, 1.0f, 1.0f });

		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float rotation, const glm::vec4& colour);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float rotation, Ref<Texture> texture, const float tilingFactor = 1.0f, const glm::vec4& colour = { 1.0f, 1.0f, 1.0f, 1.0f });
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float rotation, Ref<SubTexture2D> subTexture, const float tilingFactor = 1.0f, const glm::vec4& colour = { 1.0f, 1.0f, 1.0f, 1.0f });

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& colour);
		static void DrawQuad(const glm::mat4& transform, Ref<Texture> texture, const float tilingFactor = 1.0f, const glm::vec4& colour = { 1.0f, 1.0f, 1.0f, 1.0f });



		static void Shutdown();

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};

		static void ResetStats();
		static Statistics GetStats();

	private:
		static void InitNewBatch();
		static void FlushAndReset();
	};

}