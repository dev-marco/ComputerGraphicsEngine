#ifndef SRC_ENGINE_LIGHTING_VERTEX_H_
#define SRC_ENGINE_LIGHTING_VERTEX_H_

#include <string>

namespace Engine {
	namespace Shader {

		const std::string lighting_vertex = R"_shader(
			#version 130

			varying vec4 position;  // position of the vertex (and fragment) in world space
			varying vec3 varyingNormalDirection;  // surface normal vector in world space

			void main (void) {
				position = gl_ModelViewMatrix * gl_Vertex;
				varyingNormalDirection = normalize(gl_NormalMatrix * gl_Normal);

				gl_Position = ftransform();
			}
		)_shader";

		const std::string lighting_fragment = R"_shader(
			#version 130

			#define MAX_LIGHTS 1000

			varying vec4 position;  // position of the vertex (and fragment) in world space
			varying vec3 varyingNormalDirection;  // surface normal vector in world space

			struct lightSource {
				vec4 position;
				vec4 ambient;
				vec4 diffuse;
				vec4 specular;
				float constantAttenuation, linearAttenuation, quadraticAttenuation;
				float spotCutoff, spotExponent;
				vec3 spotDirection;
			};

			uint total_lights = 1U;

			lightSource light[MAX_LIGHTS] = (
				lightSource(
					vec4(0.0, 0.0, 1.0, 1.0),
					vec4(0.0, 0.0, 0.0, 1.0),
					vec4(1.0, 1.0, 1.0, 1.0),
					vec4(1.0, 1.0, 1.0, 1.0),
					1.5, 0.0, 0.0,
					45.0, 10.0,
					vec3(0.0, 0.0, -1.0)
				)
			);

			out vec4 fragColor;

			void main (void) {
				vec3
					lightDirection,
					normalDirection = normalize(varyingNormalDirection),
					viewDirection = normalize(-position.xyz);
				vec4 result = vec4(0.0, 0.0, 0.0, 0.0);
				float attenuation;

				for (uint i = 0U; i < total_lights; ++i) {

					vec4 ambientLighting, diffuseReflection, specularReflection;

					if (0.0 == light[i].position.w) { // directional light?

						attenuation = 1.0; // no attenuation
						lightDirection = normalize(light[i].position.xyz);

				    } else { // point light or spotlight (or other kind of light)

						vec3 positionToLightSource = light[i].position.xyz - position.xyz;
						float distance = length(positionToLightSource);

						lightDirection = normalize(positionToLightSource);

					    if (light[i].spotCutoff <= 90.0) { // spotlight?
							float clampedCosine = max(0.0, dot(-lightDirection, light[i].spotDirection));
							if (clampedCosine < cos(radians(light[i].spotCutoff))) { // outside of spotlight cone?
								attenuation = 0.0;
							} else {
								attenuation = pow(clampedCosine, light[i].spotExponent) / (
									light[i].constantAttenuation +
				                    light[i].linearAttenuation * distance +
				                    light[i].quadraticAttenuation * distance * distance
								);
							}
						} else {
							attenuation = 1.0 / (
								light[i].constantAttenuation +
			                    light[i].linearAttenuation * distance +
			                    light[i].quadraticAttenuation * distance * distance
							);
						}
				    }

					ambientLighting = light[i].ambient * gl_FrontMaterial.ambient;

					diffuseReflection = attenuation *
					    light[i].diffuse * gl_FrontMaterial.diffuse *
					    max(0.0, dot(normalDirection, lightDirection));

					if (dot(normalDirection, lightDirection) < 0.0) { // light source on the wrong side?
				    	specularReflection = vec3(0.0, 0.0, 0.0); // no specular reflection
				    } else { // light source on the right side
						specularReflection = attenuation * light[i].specular * gl_FrontMaterial.specular *
							pow(max(0.0, dot(reflect(-lightDirection, normalDirection), viewDirection)), gl_FrontMaterial.shininess);
				    }

					result += ambientLighting + diffuseReflection + specularReflection;
				}

				fragColor = result + gl_FrontLightModelProduct.sceneColor;
			}
		)_shader";

	};
};

#endif
