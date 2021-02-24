#pragma once

#include "gl-header.h"

namespace res
{
	using Id = GLuint;

	constexpr const Id null = 0u; 
	constexpr const Id default_framebuffer = null;


	struct Shader
	{		
		Shader() = default;

		Shader(Shader&& another) noexcept;
		Shader(const Shader&) = delete;

		Shader& operator = (Shader&& another) noexcept;
		Shader& operator = (const Shader&) = delete;

		~Shader();


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct ShaderProgram
	{
		ShaderProgram() = default;

		ShaderProgram(ShaderProgram&& another) noexcept;
		ShaderProgram(const ShaderProgram&) = delete;

		ShaderProgram& operator = (ShaderProgram&& another) noexcept;
		ShaderProgram& operator = (const ShaderProgram&) = delete;

		~ShaderProgram();


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct Buffer
	{
		Buffer() = default;

		Buffer(Buffer&& another) noexcept;
		Buffer(const Buffer&) = delete;

		Buffer& operator = (Buffer&& another) noexcept;
		Buffer& operator = (const Buffer&) = delete;

		~Buffer();


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct VertexArray
	{
		VertexArray() = default;

		VertexArray(VertexArray&& another) noexcept;
		VertexArray(const VertexArray&) = delete;

		VertexArray& operator = (VertexArray&& another) noexcept;
		VertexArray& operator = (const VertexArray&) = delete;

		~VertexArray();


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct Texture
	{
		Texture() = default;

		Texture(Texture&& another) noexcept;
		Texture(const Texture&) = delete;

		Texture& operator = (Texture&& another) noexcept;
		Texture& operator = (const Texture&) = delete;

		~Texture();


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct Query
	{
		Query() = default;

		Query(Query&& another) noexcept;
		Query(const Query&) = delete;

		Query& operator = (Query&& another) noexcept;
		Query& operator = (const Query&) = delete;

		~Query();


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct Sync
	{
		// TODO
	};

	struct MapPointer
	{
		// TODO
	};
}
