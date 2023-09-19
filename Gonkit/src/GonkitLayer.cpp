#include "gkpch.h"
#include "GonkitLayer.h"
#include <Gonk.h>

#include <imgui/imgui.h>
#include "Platform/OpenGL/OpenGLShader.h"

#include <chrono>

const char* s_MapTiles =
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWDDDDDDDDWWWWWWWWW"
"WWWWWDDDDDDDDDDDDDWWWWWW"
"WWWWWDDDDWDDDWDDDDDWWWWW"
"WWWWWDDDDWDWDWDDDDDWWWWW"
"WWWWWDDDDDWDWDDDDDDWWWWW"
"WWWDDDDDDDDDDDDDDDDWWWWW"
"WWWDDDDDDDDDDDDDDDDWWWWW"
"WWWDDDDDDDDDDDDDDDDWWWWW"
"WWWDDDDDDDDDDDDDDDDWWWWW"
"WWWDDDDDDDDDDDDDDDDWWWWW"
"WWWDDDDDDDDDDDDDDDDWWWWW"
"WWWWDDDDDDDDDDDDDDDWWWWW"
"WWWWDDDDDDDDDDDDDDWWWWWW"
"WWWWWDDDDDDDDDDDDDWWWWWW"
"WWWWWWWDDDDDDDDWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW"
;
const uint32_t s_MapWidth = 24;
const uint32_t s_MapHeight = strlen(s_MapTiles) / s_MapWidth;

Gonk::Entity squareEntity;

namespace Gonk {

	void GonkitLayer::OnAttach()
	{
		GK_PROFILE_FUNCTION();

		m_ActiveScene = CreateRef<Scene>();
		squareEntity = m_ActiveScene->CreateEntity("Square Entity");
		squareEntity.AddComponent<SpriteRendererComponent>(glm::vec4{0.0f, 1.0f, 0.0f, 1.0f });
	 
		// framebuffer
		FramebufferSpec fbSpec;
		fbSpec.Width = 1280.0f;
		fbSpec.Height = 720.0f;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		// textures
		m_CheckerboardTexture = Texture2D::Create("assets/textures/Checkerboard.png");
		m_LogoTexture = Texture2D::Create("assets/textures/ChernoLogo.png");
		m_Spritesheet = Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

		// subtextures
		m_Barrel = SubTexture2D::CreateFromCoords(m_Spritesheet, { 9, 2 }, { 128, 128 });
		m_Tree = SubTexture2D::CreateFromCoords(m_Spritesheet, { 0, 1 }, { 128, 128 }, { 1, 2 });
		s_TextureMap['D'] = SubTexture2D::CreateFromCoords(m_Spritesheet, { 6, 11 }, { 128, 128 });
		s_TextureMap['W'] = SubTexture2D::CreateFromCoords(m_Spritesheet, { 11, 11 }, { 128, 128 });

		// camera
		m_CameraController.SetZoomLevel(5.0f);
		m_CameraController.SetPosition({ s_MapWidth / 2.0f, s_MapHeight / 2.0f });

		Shader::Create("assets/shaders/FlatColour.glsl");

		//Application::Get().GetWindow().SetVSync(false);
	}

	void GonkitLayer::OnDetach()
	{
		GK_PROFILE_FUNCTION();

	}

	float timestep = 0.0f;


	void GonkitLayer::OnUpdate(Timestep ts)
	{
		GK_PROFILE_FUNCTION();

		timestep = (float)ts.GetMilliSeconds();
		// update
		if (m_ViewportFocused)
			m_CameraController.OnUpdate(ts);




		Renderer2D::ResetStats();
		{
			GK_PROFILE_SCOPE("Renderer Prep");
			m_Framebuffer->Bind();
			RendererCommand::SetColour(glm::vec4{0.2f});
			RendererCommand::Clear();
		}

		Renderer2D::BeginScene(m_CameraController.GetCamera());

		for (int y = 0; y < s_MapHeight; y++)
		{
			for (int x = 0; x < s_MapWidth; x++)
			{
				char c = s_MapTiles[x + y * s_MapWidth];
				Ref<SubTexture2D> texture;
				if (s_TextureMap.find(c) != s_TextureMap.end())
					texture = s_TextureMap[c];
				else
					texture = m_Barrel;
				Renderer2D::DrawQuad({ x, y, 0.0f }, { 1.0f, 1.0f }, texture);
			}
		}

		Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.9f }, { 1.0f, 1.0f }, m_Barrel);
		Renderer2D::DrawQuad({ 1.0f, 0.0f, 0.9f }, { 1.0f, 2.0f }, m_Tree);

		// update scene
		m_ActiveScene->OnUpdate(ts);


		Renderer2D::EndScene();

		m_Framebuffer->UnBind();
	}

	void GonkitLayer::OnEvent(Event& e)
	{
		m_CameraController.OnEvent(e);
	}

	void GonkitLayer::OnImGuiRender()
	{
		GK_PROFILE_FUNCTION();

		static bool p_open = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &p_open, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
					Application::Get().Close();

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Begin("Settings");

		auto stats = Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("	FrameTime: %0.2fms (%f fps)", timestep, 1000.0f / timestep);
		ImGui::Text("	Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("	Quads: %d", stats.QuadCount);
		ImGui::Text("	Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("	Indices: %d", stats.GetTotalIndexCount());

		ImGui::NewLine();
		ImGui::InputFloat3("Camera Info", (float*)&m_CameraController);
		ImGui::NewLine();

		
		ImGui::ColorEdit4("Col", &squareEntity.GetComponent<SpriteRendererComponent>().Colour[0]);

		ImGui::End();
		
		// Gonk Viewport

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		ImGui::Begin("Gonk Viewport");

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);
		

		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		if (m_ViewportSize != *((glm::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0)
		{	
			// Checking due to weird bug when double clicking imgui window causes height to be -16. Like why?
			m_ViewportSize = { viewportSize.x, viewportSize.y };
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

			m_CameraController.OnResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		ImGui::Image((void*)m_Framebuffer->GetColorAttachmentRendererID(), { m_ViewportSize.x, m_ViewportSize.y }, { 0, 1 }, { 1, 0 });
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();

	}

}