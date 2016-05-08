#ifndef SRC_ENGINE_SHADERS_WAVEROTATE_H_
#define SRC_ENGINE_SHADERS_WAVEROTATE_H_

#include <string>

namespace Engine {
	namespace Shader {

		const std::string wave_rotate_fragment = R"_wave_rot_frag(
			#version 120

			void main (void) {
				gl_FragColor = gl_Color;
			}
		)_wave_rot_frag";

		const std::string wave_rotate_vertex = R"_wave_rot_vert(
			#version 120

			uniform float time, parameter_wave, parameter_rotate;

			void main (void) {

				gl_FrontColor = gl_Color;
				vec4 v = vec4(gl_Vertex);

				if (parameter_wave != 0.0) {
					float increment = 0.25f * sqrt(parameter_wave) / 2.0f;
					v.z += (sin(5.0 * v.x + time) * increment) - increment;
				}

				if (parameter_rotate != 0.0) {
					v = mat4(
						 cos(parameter_rotate), sin(parameter_rotate), 0, 0,
						-sin(parameter_rotate), cos(parameter_rotate), 0, 0,
						                     0,                     0, 1, 0,
						                     0,                     0, 0, 1
					) * v;
				}

				gl_Position = gl_ModelViewProjectionMatrix * v;
			}
		)_wave_rot_vert";

	};
};

#endif
