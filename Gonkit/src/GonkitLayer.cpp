#include "gkpch.h"
#include "GonkitLayer.h"
#include <Gonk.h>

#include <imgui/imgui.h>
#include "Platform/OpenGL/OpenGLShader.h"

#include <chrono>


namespace Gonk {

	void GonkitLayer::OnAttach()
	{
		GK_PROFILE_FUNCTION();

		m_ActiveScene = CreateRef<Scene>();
		m_SquareEntity = m_ActiveScene->CreateEntity("Square Entity");
		m_SquareEntity.AddComponent<SpriteRendererComponent>(glm::vec4{0.0f, 1.0f, 0.0f, 1.0f });
	 
		// framebuffer
		FramebufferSpec fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		// camera
		m_CameraController.SetZoomLevel(5.0f);

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
		m_CameraEntity.AddComponent<CameraComponent>();

		m_SecondCameraEntity = m_ActiveScene->CreateEntity("Clip Space camera entity");
		m_SecondCameraEntity.AddComponent<CameraComponent>();

		class TestScript : public ScriptableEntity
		{
		public:
			void OnCreate()
			{
				GK_TRACE("OnCreate Successful for TestScript!");
			}

			void OnDestroy()
			{

			}

			void OnUpdate(Timestep ts)
			{
				auto& transform = GetComponent<TransformComponent>().Transform;

				if (Input::IsKeyPressed(Key::W))
					transform = glm::translate(transform, glm::vec3(0.0f, 1.0f, 0.0f));
				if (Input::IsKeyPressed(Key::S))
					transform = glm::translate(transform, glm::vec3(0.0f, -1.0f, 0.0f));
				if (Input::IsKeyPressed(Key::A))
					transform = glm::translate(transform, glm::vec3(-1.0f, 0.0f, 0.0f));
				if (Input::IsKeyPressed(Key::D))
					transform = glm::translate(transform, glm::vec3(1.0f, 0.0f, 0.0f));
			}
		};

		m_SecondCameraEntity.AddComponent<NativeScriptComponent>().Bind<TestScript>();

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

		if (FramebufferSpec spec = m_Framebuffer->GetSpec();
			m_ViewportSize.x > 0 && m_ViewportSize.y > 0 &&
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			// Checking due to weird bug when double clicking imgui window causes height to be -16. Like why?
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

			m_CameraController.OnResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}


		// update
		if (m_ViewportFocused)
			m_CameraController.OnUpdate(ts);


		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RendererCommand::SetColour(glm::vec4{0.2f});
		RendererCommand::Clear();
		
		// update scene
		m_ActiveScene->OnUpdate(ts);

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

		ImGui::ColorEdit4("Col", &m_SquareEntity.GetComponent<SpriteRendererComponent>().Colour[0]);

		ImGui::Checkbox("Primary Camera", &m_PrimaryCamera);
		m_SecondCameraEntity.GetComponent<CameraComponent>().Primary = !m_PrimaryCamera;

		ImGui::DragFloat("Secondary camera transform", (float*)&m_SecondCameraEntity.GetComponent<TransformComponent>().Transform[3]);

		if (!m_PrimaryCamera)
		{
			SceneCamera& camera = m_SecondCameraEntity.GetComponent<CameraComponent>().Camera;
			float orthoSize = camera.GetOrthographicSize();

			if (ImGui::DragFloat("Secondary Camera Orthographic Size", &orthoSize))
			{
				camera.SetOrthographicSize(orthoSize);
			}
		}


		ImGui::End();
		
		// Gonk Viewport

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		ImGui::Begin("Gonk Viewport"); 

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);
		
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportSize.x, viewportSize.y };

		ImGui::Image((void*)m_Framebuffer->GetColorAttachmentRendererID(), { m_ViewportSize.x, m_ViewportSize.y }, { 0, 1 }, { 1, 0 });

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();

	}

}