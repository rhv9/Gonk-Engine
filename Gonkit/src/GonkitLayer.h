#pragma once

#include <Gonk.h>

namespace Gonk {

	class GonkitLayer : public Layer
	{
	public:
		GonkitLayer() : Layer("Sandbox2D") {}

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

		virtual void OnImGuiRender() override;
	private:
		glm::vec4 m_Col = { 0.8f, 0.2f, 0.2f, 1.0f };
		Ref<Texture2D> m_CheckerboardTexture, m_LogoTexture;

		OrthographicCameraController m_CameraController{1280.0f / 720.0f};

		Ref<Texture2D> m_Spritesheet;
		Ref<SubTexture2D> m_Barrel, m_Tree;
		Ref<Framebuffer> m_Framebuffer;

		glm::vec2 m_ViewportSize;
		bool m_ViewportFocused = true, m_ViewportHovered = true;

		std::unordered_map<char, Ref<SubTexture2D>> s_TextureMap;
	};

}