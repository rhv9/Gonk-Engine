#pragma once

#include "Buffer.h"

namespace Gonk {

	class VertexArray 
	{
	public:
		virtual ~VertexArray() {}

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual void AddVertexBuffer(Ref<VertexBuffer> vertexBuffer) = 0;
		virtual void SetIndexBuffer(Ref<IndexBuffer> indexBuffer) = 0;
		
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

		static Ref<VertexArray> Create();
	};

}