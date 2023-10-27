#include "gkpch.h"
#include "Renderer2D.h"

#include "Gonk/Renderer/Shader.h"

namespace Gonk {

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Colour;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
	};

	struct Renderer2DData
	{
		static const uint32_t MaxQuads = 100000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32; // TODO: RenderCaps
		const glm::vec4 QuadVertexPositions[4] = {
			{ -0.5f, -0.5f,  0.0f,  1.0f },
			{  0.5f, -0.5f,  0.0f,  1.0f },
			{  0.5f,  0.5f,  0.0f,  1.0f },
			{ -0.5f,  0.5f,  0.0f,  1.0f }
		};
		const glm::vec2 TexCoords[4] = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f },
		};
		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		Ref<Texture> WhiteTexture;
		std::array<Ref<Texture>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = WhiteTexture

		bool SceneStarted = false;

		Renderer2D::Statistics Stats;

	};

	static Renderer2DData s_Data;

	// helper function
	static void inline setQuadVertexBuffer(const QuadVertex* quadVertexBufferPtr, const glm::vec4& position, const glm::vec4& colour, const glm::vec2& texCoords, const float textureIndex, const float tilingFactor)
	{
		s_Data.QuadVertexBufferPtr->Position     = position;
		s_Data.QuadVertexBufferPtr->Colour       = colour;
		s_Data.QuadVertexBufferPtr->TexCoord     = texCoords;
		s_Data.QuadVertexBufferPtr->TexIndex     = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
	}

	void Gonk::Renderer2D::Init()
	{
		GK_PROFILE_FUNCTION();

		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;
			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;
			offset += 4;
		}
		s_Data.QuadVertexBuffer = Gonk::VertexBuffer::Create(s_Data.MaxQuads * sizeof(QuadVertex) * 4);
		s_Data.QuadVertexBuffer->SetLayout({
			{ Gonk::ShaderDataType::Float3, "a_Position" },
			{ Gonk::ShaderDataType::Float4, "a_Colour" },
			{ Gonk::ShaderDataType::Float2, "a_TexCoord" },
			{ Gonk::ShaderDataType::Float, "a_TexIndex" },
			{ Gonk::ShaderDataType::Float, "a_TilingFactor" },
			});
		s_Data.QuadVertexArray = Gonk::VertexArray::Create();
		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);
		Gonk::Ref<Gonk::IndexBuffer> quadIndexBuffer = Gonk::IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		delete[] quadIndices;
		s_Data.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);

		// Textures
		int32_t samplers[s_Data.MaxTextureSlots];
		for (int32_t i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;

		s_Data.TextureShader = Gonk::Shader::Create("assets/shaders/TextColShader.glsl");
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

		// init rendererdata vars
		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];
		// create white texture
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint8_t pixel[] = { 255, 255, 255, 255 };
		s_Data.WhiteTexture->SetData(&pixel, 1);

		s_Data.TextureSlots[0] = s_Data.WhiteTexture;
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		GK_PROFILE_FUNCTION();

		if (s_Data.SceneStarted)
		{
			GK_CORE_CRITICAL("Beginning a Renderer2D Scene without ending prior scene!");
		}

		s_Data.SceneStarted = true;

		glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjectionMatrix", viewProj);

		InitNewBatch();

	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		GK_PROFILE_FUNCTION();

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjectionMatrix", camera.GetViewProjection());

		InitNewBatch();
	}

	void Renderer2D::EndScene()
	{
		GK_PROFILE_FUNCTION();

		s_Data.SceneStarted = false;

		uint32_t dataSize = static_cast<uint32_t>((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::Flush()
	{
		GK_PROFILE_FUNCTION();

		// nothing to draw
		if (s_Data.QuadIndexCount == 0)
			return;

		// Bind textures
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);
		RendererCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);

		s_Data.Stats.DrawCalls++;
	}

	void Renderer2D::InitNewBatch()
	{
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::FlushAndReset()
	{
		EndScene();
		InitNewBatch();
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colour)
	{
		GK_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		constexpr float texIndex = 0.0f;
		constexpr float tilingFactor = 1.0f;
		glm::mat4 transform = glm::scale(glm::translate(glm::mat4(1.0f), position), { size, 1.0f });
		for (int i = 0; i < 4; i++)
		{
			setQuadVertexBuffer(s_Data.QuadVertexBufferPtr, transform * s_Data.QuadVertexPositions[i], colour, s_Data.TexCoords[i], texIndex, tilingFactor);
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, Ref<Texture> texture, const float tilingFactor, const glm::vec4& colour)
	{
		GK_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		float textureIndex = 0;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*texture.get() == *s_Data.TextureSlots[i].get())
			{
				textureIndex = (float)i;
				break;
			}
		}
		if (textureIndex == 0)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex++] = texture;
		}
		glm::mat4 transform = glm::scale(glm::translate(glm::mat4(1.0f), position),{ size, 1.0f });
		for (int i = 0; i < 4; i++)
		{
			setQuadVertexBuffer(s_Data.QuadVertexBufferPtr, transform * s_Data.QuadVertexPositions[i], colour, s_Data.TexCoords[i], textureIndex, tilingFactor);
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, Ref<SubTexture2D> subTexture, const float tilingFactor, const glm::vec4& colour)
	{
		GK_PROFILE_FUNCTION();

		auto texCoords = subTexture->GetTexCoords();
		auto texture = subTexture->GetTexture();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		float textureIndex = 0;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*texture.get() == *s_Data.TextureSlots[i].get())
			{
				textureIndex = (float)i;
				break;
			}
		}
		if (textureIndex == 0)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex++] = texture;
		}
		glm::mat4 transform = glm::scale(glm::translate(glm::mat4(1.0f), position), { size, 1.0f });
		for (int i = 0; i < 4; i++)
		{
			setQuadVertexBuffer(s_Data.QuadVertexBufferPtr, transform * s_Data.QuadVertexPositions[i], colour, texCoords[i], textureIndex, tilingFactor);
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float rotation, const glm::vec4& colour)
	{
		GK_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		constexpr float textureIndex = 0.0f;
		constexpr float tilingFactor = 1.0f;
		glm::mat4 transform =glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), position), rotation,{ 0.0f, 0.0f, 1.0f }),{ size, 1.0f });
		for (int i = 0; i < 4; i++)
		{
			setQuadVertexBuffer(s_Data.QuadVertexBufferPtr, transform * s_Data.QuadVertexPositions[i], colour, s_Data.TexCoords[i], textureIndex, tilingFactor);
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float rotation, Ref<Texture> texture, const float tilingFactor, const glm::vec4& colour)
	{
		GK_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		float textureIndex = 0;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*texture.get() == *s_Data.TextureSlots[i].get())
			{
				textureIndex = (float)i;
				break;
			}
		}
		if (textureIndex == 0)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex++] = texture;
		}
		glm::mat4 transform = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), position), rotation, { 0.0f, 0.0f, 1.0f }), { size, 1.0f });
		for (int i = 0; i < 4; i++)
		{
			setQuadVertexBuffer(s_Data.QuadVertexBufferPtr, transform * s_Data.QuadVertexPositions[i], colour, s_Data.TexCoords[i], textureIndex, tilingFactor);
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float rotation, Ref<SubTexture2D> subTexture, const float tilingFactor, const glm::vec4& colour)
	{
		GK_PROFILE_FUNCTION();

		auto texCoords = subTexture->GetTexCoords();
		auto texture = subTexture->GetTexture();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		float textureIndex = 0;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*texture.get() == *s_Data.TextureSlots[i].get())
			{
				textureIndex = (float)i;
				break;
			}
		}
		if (textureIndex == 0)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex++] = texture;
		}
		glm::mat4 transform = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), position), rotation, { 0.0f, 0.0f, 1.0f }), { size, 1.0f });
		for (int i = 0; i < 4; i++)
		{
			setQuadVertexBuffer(s_Data.QuadVertexBufferPtr, transform * s_Data.QuadVertexPositions[i], colour, texCoords[i], textureIndex, tilingFactor);
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& colour)
	{
		GK_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		constexpr float texIndex = 0.0f;
		constexpr float tilingFactor = 1.0f;
		for (int i = 0; i < 4; i++)
		{
			setQuadVertexBuffer(s_Data.QuadVertexBufferPtr, transform * s_Data.QuadVertexPositions[i], colour, s_Data.TexCoords[i], texIndex, tilingFactor);
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, Ref<Texture> texture, const float tilingFactor, const glm::vec4& colour)
	{
		GK_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		float textureIndex = 0;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*texture.get() == *s_Data.TextureSlots[i].get())
			{
				textureIndex = (float)i;
				break;
			}
		}
		if (textureIndex == 0)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex++] = texture;
		}
		for (int i = 0; i < 4; i++)
		{
			setQuadVertexBuffer(s_Data.QuadVertexBufferPtr, transform * s_Data.QuadVertexPositions[i], colour, s_Data.TexCoords[i], textureIndex, tilingFactor);
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::Shutdown()
	{
		GK_PROFILE_FUNCTION();

		delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data.Stats;
	}

}