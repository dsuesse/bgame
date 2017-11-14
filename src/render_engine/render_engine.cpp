#include "../stdafx.h"
#include "../bengine/gl_include.hpp"
#include "render_engine.hpp"
#include "chunks/chunks.hpp"
#include "../global_assets/game_camera.hpp"
#include "../global_assets/shader_storage.hpp"
#include "../global_assets/texture_storage.hpp"
#include "../bengine/main_window.hpp"
#include "fbo/buffertest.hpp"
#include "world_textures/world_textures.hpp"
#include "vox/voxreader.hpp"
#include "vox/voxel_model.hpp"
#include "../components/position.hpp"
#include "../components/building.hpp"
#include "../components/renderable_composite.hpp"
#include "../global_assets/game_calendar.hpp"
#include "vox/renderables.hpp"
#include "../systems/mouse.hpp"
#include "chunks/cursors.hpp"
#include "../systems/gui/particle_system.hpp"
#include "camera.hpp"
#include "renderbuffers.hpp"

namespace render {


    inline void chunk_maintenance() {
        if (!chunks::chunks_initialized) {
            chunks::initialize_chunks();
        }
        chunks::update_dirty_chunks();
        chunks::update_buffers();
    }

    inline void do_chunk_render() {
        for (const auto &idx : visible_chunks) {
            chunks::chunk_t * target = &chunks::chunks[idx];
            if (target->has_geometry) {
                size_t n_elements = 0;
                for (int z=0; z<chunks::CHUNK_SIZE; ++z) {
                    const int layer_z = z + target->base_z;
                    if (layer_z <= camera_position->region_z && layer_z > camera_position->region_z-10) {
                        n_elements += target->layers[z].n_elements;
                    }
                }

                if (n_elements > 0) {
                    glBindVertexArray(target->vao);
                    glDrawArrays(GL_TRIANGLES, 0, n_elements);
                }
            }
        }
    }

	inline void do_trans_chunk_render() {
		for (const auto &idx : visible_chunks) {
			chunks::chunk_t * target = &chunks::chunks[idx];
			if (target->has_transparency) {
				size_t n_elements = 0;
				for (int z = 0; z<chunks::CHUNK_SIZE; ++z) {
					const int layer_z = z + target->base_z;
					if (layer_z <= camera_position->region_z && layer_z > camera_position->region_z - 10) {
						n_elements += target->layers[z].n_trans;
					}
				}

				if (n_elements > 0) {
					glBindVertexArray(target->tvao);
					glDrawArrays(GL_TRIANGLES, 0, n_elements);
				}
			}
		}
	}	

    inline void render_chunks() {
        // Use the program
		assets::chunkshader->use();
        glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->fbo_id);
		glClearDepth(0.0f); // We're using an inverted Z-buffer
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // Assign the uniforms
        glUniformMatrix4fv(assets::chunkshader->projection_matrix, 1, GL_FALSE, glm::value_ptr(camera_projection_matrix));
        glUniformMatrix4fv(assets::chunkshader->view_matrix, 1, GL_FALSE, glm::value_ptr(camera_modelview_matrix));

        // Assign the texture array
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, assets::chunk_texture_array);

        glUniform1i(assets::chunkshader->textureArray, 0);

        do_chunk_render();
		do_trans_chunk_render();
    }

    void render_to_light_buffer() {
        glUseProgram(assets::lightstage_shader);
        glBindFramebuffer(GL_FRAMEBUFFER, light_stage_buffer->fbo_id);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glUniform1i(glGetUniformLocation(assets::lightstage_shader, "albedo_tex"), 0);
        glUniform1i(glGetUniformLocation(assets::lightstage_shader, "normal_tex"), 1);
        glUniform1i(glGetUniformLocation(assets::lightstage_shader, "position_tex"), 2);
        glUniform1i(glGetUniformLocation(assets::lightstage_shader, "ao_tex"), 3);
        glUniform1i(glGetUniformLocation(assets::lightstage_shader, "info_tex"), 4);
        glUniform1i(glGetUniformLocation(assets::lightstage_shader, "light_pos_tex"), 5);
        glUniform1i(glGetUniformLocation(assets::lightstage_shader, "light_col_tex"), 6);
        glUniform3f(glGetUniformLocation(assets::lightstage_shader, "camera_position"), (float)camera_position->region_x, (float)camera_position->region_z, (float)camera_position->region_y);
		glUniform3f(glGetUniformLocation(assets::lightstage_shader, "sun_direction"), calendar->sun_x, calendar->sun_y, calendar->sun_z);
		glUniform3f(glGetUniformLocation(assets::lightstage_shader, "moon_direction"), calendar->moon_x, calendar->moon_y, calendar->moon_z);
		glUniform3f(glGetUniformLocation(assets::lightstage_shader, "sun_color"), calendar->sun_r, calendar->sun_g, calendar->sun_b);
		glUniform3f(glGetUniformLocation(assets::lightstage_shader, "moon_color"), calendar->moon_r, calendar->moon_g, calendar->moon_b);
		glUniform1i(glGetUniformLocation(assets::lightstage_shader, "sun_depth_tex"), 7);
		glUniformMatrix4fv(glGetUniformLocation(assets::lightstage_shader, "sun_projection"), 1, GL_FALSE, glm::value_ptr(sun_projection_matrix));
		glUniformMatrix4fv(glGetUniformLocation(assets::lightstage_shader, "sun_modelview"), 1, GL_FALSE, glm::value_ptr(sun_modelview_matrix));
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gbuffer->albedo_tex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gbuffer->normal_tex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gbuffer->position_tex);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gbuffer->ao_tex);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_3D, outdoors_y_z_tex);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_3D, lightpos_tex);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_3D, lightcol_tex);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, sun_buffer->depth_tex);
        render_buffer_quad();
    }

    /*
    void bloom_blur() {
        glUseProgram(assets::bloom_shader);
        glActiveTexture(GL_TEXTURE0);
        bool horizontal = true, first_iteration = true;
        constexpr int amount = 10;
        for (unsigned int i=0; i<amount; ++i) {
            if (horizontal) {
                glBindFramebuffer(GL_FRAMEBUFFER, bloom_buffer->fbo1_id);
                glUniform1i(glGetUniformLocation(assets::bloom_shader, "horizontal"), 1);
                if (first_iteration) {
                    glBindTexture(GL_TEXTURE_2D, light_stage_buffer->bright_tex);
                } else {
                    glBindTexture(GL_TEXTURE_2D, bloom_buffer->blur_tex2);
                }
            } else {
                glBindFramebuffer(GL_FRAMEBUFFER, bloom_buffer->fbo2_id);
                glUniform1i(glGetUniformLocation(assets::bloom_shader, "horizontal"), 0);
                glBindTexture(GL_TEXTURE_2D, bloom_buffer->blur_tex1);
            }
            render_buffer_quad();
            horizontal = !horizontal;
            if (first_iteration) first_iteration = false;
        }
    }*/

    void tone_map_scene() {
        glUseProgram(assets::tonemap_shader);
        glBindFramebuffer(GL_FRAMEBUFFER, hdr_buffer->fbo_id);
        // Setup uniforms
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniform1i(glGetUniformLocation(assets::tonemap_shader, "hdr_tex"), 0);
        //glUniform1i(glGetUniformLocation(assets::tonemap_shader, "blur_tex"), 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, light_stage_buffer->color_tex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bloom_buffer->blur_tex1);
        render_buffer_quad();
    }

	inline void update_buffers() {
		chunk_maintenance();
		if (camera_moved) update_camera();
		if (sun_moved) update_sun_camera();
		update_world_textures();
		build_voxel_render_list(visible_chunks);
		build_cursor_geometry();
		systems::particles::build_buffers();
		glCheckError();
	}

    void render_gl(const double &duration_ms) {
        glCheckError();
        int screen_w, screen_h;
        glfwGetWindowSize(bengine::main_window, &screen_w, &screen_h);

        if (!gbuffer) {
			build_framebuffers(screen_w, screen_h);
        }

		// Handle building all of our GL buffers
		update_buffers();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GREATER);

		// Render the scene from the point-of-view of the sun
		// Bind the directional light shader
		glUseProgram(assets::dirlight_shader);
		// Bind the SUN FBO
		glBindFramebuffer(GL_FRAMEBUFFER, sun_buffer->fbo_id);
		// Set uniforms
		glUniformMatrix4fv(glGetUniformLocation(assets::dirlight_shader, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(sun_projection_matrix));
		glUniformMatrix4fv(glGetUniformLocation(assets::dirlight_shader, "view_matrix"), 1, GL_FALSE, glm::value_ptr(sun_modelview_matrix));
		// Render everything to it
		glClearDepth(0.0f); // Inverted
		glClear(GL_DEPTH_BUFFER_BIT);
		for (const auto target: chunks::chunks) {
			if (target.has_geometry) {
				size_t n_elements = 0;
				for (int z = 0; z<chunks::CHUNK_SIZE; ++z) {
					const int layer_z = z + target.base_z;
					if (layer_z > camera_position->region_z - 10) {
						n_elements += target.layers[z].n_elements;
					}
				}

				if (n_elements > 0) {
					glBindVertexArray(target.vao);
					glDrawArrays(GL_TRIANGLES, 0, n_elements);
				}
			}
		}
		glCheckError();

		// TODO: Render the scene from the POV of the moon
		// TODO: Render cube-maps from lights

        // Render a pre-pass to put color, normal, etc. into the gbuffer. Also puts sunlight in place.
        render_chunks();
        glCheckError();
        render_voxel_models(gbuffer.get(), camera_projection_matrix, camera_modelview_matrix);
		systems::particles::render_particles(camera_projection_matrix, camera_modelview_matrix);

        // TODO: Render windows and other transparency

        // Stop writing to the gbuffer and depth-testing
        glDisable(GL_DEPTH_TEST);

        // Render the combined light buffer
        render_to_light_buffer();

		// Add in translucent cursors
		glBindFramebuffer(GL_FRAMEBUFFER, light_stage_buffer->fbo_id);
		render_cursor(camera_projection_matrix, camera_modelview_matrix);

        // Tone mapping
        tone_map_scene();

        // Render some test results
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        render_test_quad(hdr_buffer->color_tex);
        //render_test_quad(sun_buffer->depth_tex);

        // TODO: Final combination and post-process

        glCheckError();
    }
}