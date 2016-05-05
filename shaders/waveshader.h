#ifndef SRC_ENGINE_SHADERS_WAVESHADER_H_
#define SRC_ENGINE_SHADERS_WAVESHADER_H_

#include <string>

namespace Engine {
	namespace Shader {

		const std::string wave_fragment = R"_wave_frag(
			#version 120

			void main (void) {
				gl_FragColor = gl_Color;
			}
		)_wave_frag";

		const std::string wave_vertex = R"_wave_vert(
			#version 120

			uniform float time;
			uniform float parameter;

			void main (void) {

				gl_FrontColor = gl_Color;
				vec4 v = vec4(gl_Vertex);
				float increment = 0.25f * min(sqrt(parameter) / 2.0f, 1.5f);
				v.z += (sin(5.0 * v.x + time) * increment) - increment;

				gl_Position = gl_ModelViewProjectionMatrix * v;
			}
		)_wave_vert";

	};
};

#endif
