#pragma once

namespace Gonk {

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetData(const void* data, uint32_t size) const = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
	};


	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::string& path);
		static Ref<Texture2D> Create(const uint32_t width, const uint32_t height);
	};
}