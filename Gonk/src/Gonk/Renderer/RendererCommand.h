#pragma once
#include <glm/glm.hpp>

#include "Gonk/Renderer/RendererAPI.h"

namespace Gonk {

	class RendererCommand
	{
	public:

		inline static void SetColour(const glm::vec4& colour)
		{
			s_RendererAPI->SetClearColour(colour);
		}

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};
}