#include "gkpch.h"
#include "Scene.h"

#include "Component.h"
#include "Gonk/Renderer/Renderer2D.h"

#include "Entity.h"

namespace Gonk {

    static void DoMaths(const glm::mat4& transform)
    {

    }

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity e = { m_Registry.create() , this };
        e.AddComponent<TransformComponent>();
        e.AddComponent<TagComponent>(name);
        return e;
    }

    void Scene::OnUpdate(Timestep ts)
    {
        // Update Scripts
        {
            m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
                {
                    if (!nsc.Instance)
                    {
                        nsc.InstantiateFunction();
                        nsc.Instance->m_Entity = Entity{ entity, this };
                        nsc.OnCreateFunction(nsc.Instance);
                    }
                    nsc.OnUpdateFunction(nsc.Instance, ts);
                });
        }


        // Render 2D
        Camera* mainCamera = nullptr;
        glm::mat4* cameraTransform = nullptr;
        
        auto group2 = m_Registry.view<TransformComponent, CameraComponent>();
        for (auto entity : group2)
        {
            // TODO: Why do I have to put && instead of &
            auto&&[transform, camera] = group2.get<TransformComponent, CameraComponent>(entity);
            
            if (camera.Primary)
            {
                mainCamera = &camera.Camera;
                cameraTransform = &transform.Transform;
                break;
            }
        }
        

        if (mainCamera)
        {
            Renderer2D::BeginScene(*mainCamera, *cameraTransform);

            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group)
            {
                auto&& [transform, spriteRenderer] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                
                Renderer2D::DrawQuad(transform, spriteRenderer.Colour);
            }

            Renderer2D::EndScene();
        }
        else
        {
            GK_CORE_WARN("No main camera set!");
        }

    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        // Resize our non-Fixed Aspect Ratio cameras.
        auto view = m_Registry.view<CameraComponent>();

        for (auto entity : view)
        {
            auto& cameraComponent = view.get<CameraComponent>(entity);

            if (!cameraComponent.FixedAspectRatio)
            {
                cameraComponent.Camera.SetViewportSize(width, height);
            }
        }
    }

}