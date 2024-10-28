/**************************************************************************/
/*  stencil_write_gles3.h                                                 */
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

#ifndef STENCIL_WRITE_GLES3_H
#define STENCIL_WRITE_GLES3_H

#include "shaders/canvas_stencil_write.glsl.gen.h"

class StencilWriteGLES3 {
	struct StencilWriteBuffer {
		float canvas_transform[16];
		float screen_transform[16];
		float view[16];
		float projection[16];
		float rect[4];
	} buffer;
	static_assert(sizeof(StencilWriteBuffer) % 16 == 0, "Stencil Write Buffer UBO size must be a multiple of 16 bytes");

	struct {
		CanvasStencilWriteShaderGLES3 shader;
		RID shader_version;
	} shader;

	GLuint quad_vertices = 0;
	GLuint quad_array = 0;
	GLuint canvas_ubo = 0;

public:
	struct StencilWriteTransforms {
		Transform3D canvas;
		Transform3D screen;
		Transform3D view;
		Projection projection;
	};

	void initialize();
	void setup_stencil_write(const StencilWriteTransforms &p_write_transforms);
	void do_stencil_write(Rect2 p_rect, int p_stencil_ref);

	~StencilWriteGLES3();
};

#endif // STENCIL_WRITE_GLES3_H
