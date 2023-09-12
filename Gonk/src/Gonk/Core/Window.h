#pragma once

#include "gkpch.h"

#include "Gonk/Core/Core.h"
#include "Gonk/Events/Event.h"

namespace Gonk {

	struct WindowProps
	{
		std::string Title;
		unsigned int Width, Height;

		WindowProps(const std::string& title = "Gonk Engine",
			unsigned int width = 1280,
			unsigned int height = 720)
			: Title(title), Width(width), Height(height)
		{
		}
	};

	class GONK_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;
		virtual inline void* GetNativeWindow() const = 0;

		virtual void Shutdown() = 0;

		static Window* Create(const WindowProps& props = WindowProps());

	};
}