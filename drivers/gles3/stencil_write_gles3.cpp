/**************************************************************************/
/*  stencil_write_gles3.cpp                                               */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "stencil_write_gles3.h"
#include "platform_gl.h"
#include "shaders/canvas_stencil_write.glsl.gen.h"
#include "storage/material_storage.h"

void StencilWriteGLES3::initialize() {
	shader.shader.initialize();
	shader.shader_version = shader.shader.version_create();
	shader.shader.version_bind_shader(shader.shader_version, CanvasStencilWriteShaderGLES3::MODE_DEFAULT);

	{
		glGenBuffers(1, &quad_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertices);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, nullptr, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenVertexArrays(1, &quad_array);
		glBindVertexArray(quad_array);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertices);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind
	}

	glGenBuffers(1, &canvas_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, canvas_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(StencilWriteBuffer), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0); //unbind
}

void StencilWriteGLES3::setup_stencil_write(const StencilWriteTransforms &p_write_transforms) {
	glBindBuffer(GL_UNIFORM_BUFFER, canvas_ubo);
	GLES3::MaterialStorage::store_transform(p_write_transforms.view, buffer.view);
	GLES3::MaterialStorage::store_transform(p_write_transforms.canvas, buffer.canvas_transform);
	GLES3::MaterialStorage::store_transform(p_write_transforms.screen, buffer.screen_transform);
	GLES3::MaterialStorage::store_camera(p_write_transforms.projection, buffer.projection);
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(StencilWriteBuffer, canvas_transform), sizeof(float) * 16 * 4, &buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void StencilWriteGLES3::do_stencil_write(Rect2 p_rect, int p_stencil_ref) {
	glBindBuffer(GL_UNIFORM_BUFFER, canvas_ubo);

	float rect_data[4] = {
		p_rect.position.x,
		p_rect.position.y,
		p_rect.size.x,
		p_rect.size.y,
	};
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(StencilWriteBuffer, rect), sizeof(rect_data), &rect_data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	shader.shader.version_bind_shader(shader.shader_version, CanvasStencilWriteShaderGLES3::MODE_DEFAULT);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, canvas_ubo);
	glBindVertexArray(quad_array);

	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, p_stencil_ref, 0xFF);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glStencilFunc(GL_EQUAL, p_stencil_ref, 0xFF);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilMask(0x00);
}

StencilWriteGLES3::~StencilWriteGLES3() {
	if (shader.shader_version.is_valid()) {
		shader.shader.version_free(shader.shader_version);
	}

	if (canvas_ubo) {
		glDeleteBuffers(1, &canvas_ubo);
	}

	if (quad_vertices) {
		glDeleteBuffers(1, &quad_array);
	}

	if (quad_array) {
		glDeleteVertexArrays(1, &quad_array);
	}
}
