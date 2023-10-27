#pragma once

namespace Gonk {

	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection)
			: m_Projection(projection) {}

		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_Projection; }

		// TODO: Add specific type of camera setters to make API glm agnostic

	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};

	class OrthographicCamera
	{

	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetProjectionMatrix(float left, float right, float bottom, float top);
		const glm::mat4& GetViewProjection() const { return m_ViewProjectionMatrix; }
		void SetPosition(const glm::vec3& pos) { m_Position = pos; RecalculateProjectionMatrix(); }
		void SetRotation(const float rotation) { m_Rotation = rotation; RecalculateProjectionMatrix(); }
		const glm::vec3& GetPosition() const { return m_Position; }

	private:
		void RecalculateProjectionMatrix();

	private:
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		float m_Rotation = 0.0f;
	};

}