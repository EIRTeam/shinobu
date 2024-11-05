/**************************************************************************/
/*  stencil_write_rd.cpp                                                  */
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

#include "stencil_write_rd.h"
#include "core/string/print_string.h"
#include "servers/rendering/rendering_device_commons.h"
#include "storage_rd/material_storage.h"

void StencilWriteRD::initialize() {
	RD *rd = RD::get_singleton();

	Vector<String> write_modes;
	write_modes.push_back("\n");
	write_shader.shader_rd.initialize(write_modes);

	write_shader.version = write_shader.shader_rd.version_create();

	write_shader.write_shader = write_shader.shader_rd.version_get_shader(write_shader.version, 0);

	Vector<RD::Uniform> uniforms;

	stencil_write_uniform = rd->uniform_buffer_create(sizeof(StencilWriteBuffer));

	RD::Uniform buffer_uniform;
	buffer_uniform.uniform_type = RenderingDeviceCommons::UNIFORM_TYPE_UNIFORM_BUFFER,
	buffer_uniform.binding = 0,
	buffer_uniform.append_id(stencil_write_uniform);
	uniforms.push_back(buffer_uniform);

	stencil_write_uniform_set = rd->uniform_set_create(uniforms, write_shader.write_shader, 0);

	Vector<RD::VertexAttribute> vf;
	RD::VertexAttribute vertex_attrib;
	vertex_attrib.format = RD::DATA_FORMAT_R32G32_SFLOAT;
	vertex_attrib.stride = sizeof(float) * 2;
	vertex_attrib.location = 0;
	vertex_attrib.offset = 0;
	vf.push_back(vertex_attrib);
	vertex_format = rd->vertex_format_create(vf);
	vertex_buffer = rd->vertex_buffer_create(sizeof(float) * 6 * 2);

	Vector<RID> vertex_buffers;
	vertex_buffers.push_back(vertex_buffer);
	vertex_array = rd->vertex_array_create(6, vertex_format, vertex_buffers);
}

void StencilWriteRD::setup_stencil_write(RenderingDevice::FramebufferFormatID p_format_id, const StencilWriteTransforms &p_write_transforms) {
	RendererRD::MaterialStorage::store_transform(p_write_transforms.canvas, buffer.canvas_transform);
	RendererRD::MaterialStorage::store_transform(p_write_transforms.screen, buffer.screen_transform);
	RendererRD::MaterialStorage::store_transform(p_write_transforms.view, buffer.view);
	RendererRD::MaterialStorage::store_camera(p_write_transforms.projection, buffer.projection);

	RD::get_singleton()->buffer_update(stencil_write_uniform, 0, sizeof(buffer), &buffer);

	HashMap<RenderingDevice::FramebufferFormatID, RID>::Iterator it = pipelines.find(p_format_id);
	if (it != pipelines.end()) {
		return;
	}

	RD::PipelineColorBlendState cb_state = RD::PipelineColorBlendState::create_disabled();
	cb_state.attachments.ptrw()[0].write_r = false;
	cb_state.attachments.ptrw()[0].write_g = false;
	cb_state.attachments.ptrw()[0].write_b = false;
	cb_state.attachments.ptrw()[0].write_a = false;
	RD::PipelineDepthStencilState ds_state;
	ds_state.enable_stencil = true;
	ds_state.front_op.compare_mask = 0x00;
	ds_state.front_op.write_mask = 0xFF;
	ds_state.front_op.fail = RenderingDeviceCommons::STENCIL_OP_REPLACE;
	ds_state.front_op.pass = RenderingDeviceCommons::STENCIL_OP_REPLACE;
	ds_state.back_op = ds_state.front_op;

	RD::PipelineMultisampleState multisample_state;
	multisample_state.sample_count = RD::get_singleton()->framebuffer_format_get_texture_samples(p_format_id, 0);

	RID pipeline = RD::get_singleton()->render_pipeline_create(write_shader.write_shader, p_format_id, vertex_format, RD::RENDER_PRIMITIVE_TRIANGLES, RD::PipelineRasterizationState(), multisample_state, ds_state, cb_state, RD::DYNAMIC_STATE_STENCIL_REFERENCE);
	print_line(pipeline.is_valid(), p_format_id);
	RD::get_singleton()->set_resource_name(pipeline, "Stencil Write Pipeline");
	pipelines.insert(p_format_id, pipeline);
}

void StencilWriteRD::do_stencil_write(RD::DrawListID p_draw_list, RD::FramebufferFormatID p_format, Rect2 p_rect, int p_layer) {
	RD *rd = RD::get_singleton();
	push_constant.rect_pos[0] = p_rect.position.x;
	push_constant.rect_pos[1] = p_rect.position.y;
	push_constant.rect_size[0] = p_rect.size.x;
	push_constant.rect_size[1] = p_rect.size.y;
	rd->draw_list_bind_render_pipeline(p_draw_list, pipelines[p_format]);
	rd->draw_list_set_stencil_ref(p_draw_list, p_layer);
	rd->draw_list_bind_uniform_set(p_draw_list, stencil_write_uniform_set, 0);
	rd->draw_list_bind_vertex_array(p_draw_list, vertex_array);
	rd->draw_list_set_push_constant(p_draw_list, &push_constant, sizeof(push_constant));
	rd->draw_list_draw(p_draw_list, false);
}

StencilWriteRD::~StencilWriteRD() {
	RD *rd = RD::get_singleton();

	rd->free(stencil_write_uniform_set);
	rd->free(stencil_write_uniform);
	rd->free(vertex_array);
	rd->free(vertex_buffer);

	for (KeyValue<RD::FramebufferFormatID, RID> kv : pipelines) {
		rd->free(kv.value);
	}

	pipelines.clear();
	write_shader.shader_rd.version_free(write_shader.version);
}
