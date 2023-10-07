#include <Gonk.h>

#include <Gonk/Core/EntryPoint.h>

#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

#include "Sandbox2D.h"

class ExampleLayer : public Gonk::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_CameraController( 1280.0f/720.0f )
	{

		m_TextureVertexArray = Gonk::VertexArray::Create();

		float vertices[6 * 4] = {
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
		};

		Gonk::Ref<Gonk::VertexBuffer> vertexBuffer;
		vertexBuffer = Gonk::VertexBuffer::Create(vertices, sizeof(vertices));
		vertexBuffer->SetLayout({
			{ Gonk::ShaderDataType::Float3, "a_Position" },
			{ Gonk::ShaderDataType::Float2, "a_TexCoord" },
			});

		m_TextureVertexArray->AddVertexBuffer(vertexBuffer);

		unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
		Gonk::Ref<Gonk::IndexBuffer> indexBuffer;
		indexBuffer = Gonk::IndexBuffer::Create(indices, sizeof(indices) / sizeof(unsigned int));
		m_TextureVertexArray->SetIndexBuffer(indexBuffer);

		m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		m_CheckerboardTexture = Gonk::Texture2D::Create("assets/textures/Checkerboard.png");
		m_ChernoTexture = Gonk::Texture2D::Create("assets/textures/ChernoLogo.png");
		m_ShaderLibrary.Get("Texture")->Bind();
		std::dynamic_pointer_cast<Gonk::OpenGLShader>(m_ShaderLibrary.Get("Texture"))->UniformInt("u_Texture", 0);
		



		/////////////////////////////////////////////////
		///// Blue square
		/////////////////////////////////////////////////
		m_BlueVertexArray = Gonk::VertexArray::Create();

		float bluevertices[3 * 4] = {
			-0.5f,  0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f,
		};

		Gonk::Ref<Gonk::VertexBuffer> bluevertexBuffer;
		bluevertexBuffer = Gonk::VertexBuffer::Create(bluevertices, sizeof(bluevertices));
		bluevertexBuffer->SetLayout({
			{ Gonk::ShaderDataType::Float3, "a_Position" },
			});

		m_BlueVertexArray->AddVertexBuffer(bluevertexBuffer);

		unsigned int blueindices[] = { 0, 1, 2, 2, 3, 0 };
		Gonk::Ref<Gonk::IndexBuffer> blueindexBuffer;
		blueindexBuffer = Gonk::IndexBuffer::Create(blueindices, 6);
		m_BlueVertexArray->SetIndexBuffer(blueindexBuffer);

		std::string bluevertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Pos;
			
			uniform mat4 u_ViewProjectionMatrix;	
			uniform mat4 u_Transform;	

			out vec4 v_Col;

			void main() {
				gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Pos, 1.0);
			}
		)";

		std::string bluefragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			uniform vec3 u_Col;
	
			void main() {
				color = vec4(u_Col, 1.0);
			}

		)";

		m_BlueShader = Gonk::Shader::Create("Blue", bluevertexSrc, bluefragmentSrc);

	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::End();
	}

	void OnUpdate(Gonk::Timestep ts) override
	{
		m_CameraController.OnUpdate(ts);
		

		if (Gonk::Input::IsKeyPressed(Gonk::Key::I))
			m_SquarePosition.y += 1.0f * ts;
		if (Gonk::Input::IsKeyPressed(Gonk::Key::K))
			m_SquarePosition.y -= 1.0f * ts;
		if (Gonk::Input::IsKeyPressed(Gonk::Key::L))
			m_SquarePosition.x += 1.0f * ts;
		if (Gonk::Input::IsKeyPressed(Gonk::Key::J))
			m_SquarePosition.x -= 1.0f * ts;


		Gonk::RendererCommand::SetColour({ 0.1f, 0.1f, 0.1f, 1.0f });
		Gonk::RendererCommand::Clear();

		Gonk::Renderer::BeginScene(m_CameraController.GetCamera());

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		glm::vec3 whiteCol{0.8f};
		glm::vec3 redCol{0.8f, 0.2f, 0.2f};

		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 offset{x * 0.11f, y * 0.11f, 0.0f};
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), offset + m_SquarePosition) * scale;
				
				if ((x + y) % 2 == 0)
					std::dynamic_pointer_cast<Gonk::OpenGLShader>(m_BlueShader)->UniformFloat3("u_Col", redCol);
				else
					std::dynamic_pointer_cast<Gonk::OpenGLShader>(m_BlueShader)->UniformFloat3("u_Col", whiteCol);
					
				m_BlueShader->Bind();
				Gonk::Renderer::Submit(m_BlueShader, m_BlueVertexArray, transform);

			}

		}
		m_ShaderLibrary.Get("Texture")->Bind();
		m_CheckerboardTexture->Bind();
		Gonk::Renderer::Submit(m_ShaderLibrary.Get("Texture"), m_TextureVertexArray);
		m_ChernoTexture->Bind();
		Gonk::Renderer::Submit(m_ShaderLibrary.Get("Texture"), m_TextureVertexArray);
	}

	void OnEvent(Gonk::Event& e)
	{
		m_CameraController.OnEvent(e);
	}

private:
	Gonk::ShaderLibrary m_ShaderLibrary;

	Gonk::Ref<Gonk::Texture2D> m_CheckerboardTexture, m_ChernoTexture;
	Gonk::Ref<Gonk::VertexArray> m_TextureVertexArray;

	Gonk::Ref<Gonk::Shader> m_BlueShader;
	Gonk::Ref<Gonk::VertexArray> m_BlueVertexArray;

	Gonk::OrthographicCameraController m_CameraController;

	glm::vec3 m_SquarePosition{0.0f};
};


class Sandbox : public Gonk::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2DLayer());
	}

	~Sandbox()
	{

	}
};

Gonk::Application* Gonk::CreateApplication() 
{
	return new Sandbox();
}

