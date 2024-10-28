/**************************************************************************/
/*  stencil_write_rd.h                                                    */
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

#ifndef STENCIL_WRITE_RD_H
#define STENCIL_WRITE_RD_H

#include "core/math/transform_3d.h"
#include "shaders/canvas_stencil_write.glsl.gen.h"

class StencilWriteRD {
	struct StencilWriteBuffer {
		float canvas_transform[16];
		float screen_transform[16];
		float view[16];
		float projection[16];
	} buffer;

	RID stencil_write_uniform;
	RID stencil_write_uniform_set;

	RD::VertexFormatID vertex_format;
	RID vertex_buffer;
	RID vertex_array;

	struct StencilWritePushConstant {
		float rect_pos[2];
		float rect_size[2];
	} push_constant;

	struct {
		CanvasStencilWriteShaderRD shader_rd;
		RID version;
		RID write_shader;
	} write_shader;

	RID quad_vertex_format;

	HashMap<RenderingDevice::FramebufferFormatID, RID> pipelines;

public:
	struct StencilWriteTransforms {
		Transform3D canvas;
		Transform3D screen;
		Transform3D view;
		Projection projection;
	};
	void initialize();
	void setup_stencil_write(RenderingDevice::FramebufferFormatID p_format_id, const StencilWriteTransforms &p_write_transforms);
	void do_stencil_write(RD::DrawListID p_draw_list, RD::FramebufferFormatID p_format, Rect2 p_rect, int p_layer);
	~StencilWriteRD();
};

#endif // STENCIL_WRITE_RD_H
