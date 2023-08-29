#pragma once

namespace Gonk {

	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case Gonk::ShaderDataType::Float:  return 4;
		case Gonk::ShaderDataType::Float2: return 4 * 2;
		case Gonk::ShaderDataType::Float3: return 4 * 3;
		case Gonk::ShaderDataType::Float4: return 4 * 4;
		case Gonk::ShaderDataType::Mat3:   return 4 * 3 * 3;
		case Gonk::ShaderDataType::Mat4:   return 4 * 4 * 4;
		case Gonk::ShaderDataType::Int:    return 4;
		case Gonk::ShaderDataType::Int2:   return 4 * 2;
		case Gonk::ShaderDataType::Int3:   return 4 * 3;
		case Gonk::ShaderDataType::Int4:   return 4 * 4;
		case Gonk::ShaderDataType::Bool:   return 1;
		}
		GK_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}


	struct BufferElement 
	{
	public:
		BufferElement() {}

		ShaderDataType Type;
		std::string Name;
		uint32_t Offset;
		uint32_t Size;
		bool Normalized;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: Type(type), Name(name), Size(ShaderDataTypeSize(Type)), Offset(0), Normalized(normalized)
		{
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case Gonk::ShaderDataType::Float:  return 1;
			case Gonk::ShaderDataType::Float2: return 2;
			case Gonk::ShaderDataType::Float3: return 3;
			case Gonk::ShaderDataType::Float4: return 4;
			case Gonk::ShaderDataType::Mat3:   return 3 * 3;
			case Gonk::ShaderDataType::Mat4:   return 4 * 4;
			case Gonk::ShaderDataType::Int:    return 1;
			case Gonk::ShaderDataType::Int2:   return 2;
			case Gonk::ShaderDataType::Int3:   return 3;
			case Gonk::ShaderDataType::Int4:   return 4;
			case Gonk::ShaderDataType::Bool:   return 1;
			}
			GK_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() {}

		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateStrideAndOffset();
		}

		inline const std::vector<BufferElement>& GetElements() { return m_Elements; }

		uint32_t GetStride() const { return m_Stride; }
		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateStrideAndOffset()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		uint32_t m_Stride;
		std::vector<BufferElement> m_Elements;
	};

	class VertexBuffer 
	{
	public:
		virtual ~VertexBuffer() {};

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		static VertexBuffer* Create(float* vertices, uint32_t size);

		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& GetLayout() = 0;

	private:
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {};

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static IndexBuffer* Create(uint32_t* indices, uint32_t count);
	};

}