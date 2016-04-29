#include <string>

namespace Shader {

	const std::string cgwg_CRT_fragment = R"_cgwg_CRT_frag(
		#version 120

		void main (void) {
			gl_FragColor = gl_Color;
		}
	)_cgwg_CRT_frag";

	const std::string cgwg_CRT_vertex = R"_cgwg_CRT_vert(
		#version 120

		uniform float time;

		void main (void) {

			gl_FrontColor = gl_Color;
			vec4 v = vec4(gl_Vertex);
			v.z += (sin(5.0 * v.x + time) * 0.25) - 0.25f;

			gl_Position = gl_ModelViewProjectionMatrix * v;
		}
	)_cgwg_CRT_vert";

}
