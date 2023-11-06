#pragma once

// for use by Gonk applications

// core
#include "Gonk/Core/Application.h"
#include "Gonk/Core/Layer.h"
#include "Gonk/Core/Log.h"
#include "Gonk/ImGui/ImGuiLayer.h"
#include "Gonk/Core/Timestep.h"

// input
#include "Gonk/Input/Input.h"
#include "Gonk/Input/KeyCodes.h"
#include "Gonk/Input/MouseButtonCodes.h"

#include "Gonk/Scene/Scene.h"
#include "Gonk/Scene/Entity.h"
#include "Gonk/Scene/ScriptableEntity.h"
#include "Gonk/Scene/Component.h"

// Rendering
#include "Gonk/Renderer/Renderer.h"
#include "Gonk/Renderer/Renderer2D.h"
#include "Gonk/Renderer/RendererCommand.h"

#include "Gonk/Renderer/Camera.h"
#include "Gonk/Renderer/Shader.h"
#include "Gonk/Renderer/Framebuffer.h"
#include "Gonk/Renderer/VertexArray.h"
#include "Gonk/Renderer/Texture.h"
#include "Gonk/Renderer/SubTexture2D.h"

#include "Gonk/Renderer/OrthographicCameraController.h"

