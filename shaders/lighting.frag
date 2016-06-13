#version 130

#define MAX_LIGHTS 1000

in vec4 position;  // position of the vertex (and fragment) in world space
in vec3 varyingNormalDirection;  // surface normal vector in world space

out vec4 finalColor;

uniform float scriptTime;

float rand (vec2 co){
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

struct lightSource {
    vec4 position;
    vec4 diffuse;
    vec4 specular;
    float constantAttenuation, linearAttenuation, quadraticAttenuation;
    float spotCutoff, spotExponent;
    vec3 spotDirection;
};

struct material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

const int total_lights = 1;

lightSource light[] = lightSource[](
    lightSource(
        vec4(0.0, 0.0, 1.0, 1.0),
        vec4(0.0, 1.0, 0.0, 0.5),
        vec4(0.0, 1.0, 0.0, 0.5),
        1.5, rand(vec2(scriptTime, scriptTime)), 0.0,
        15.0, 128.0,
        vec3(0.0, 0.0, -1.0)
    ),
    lightSource(
        vec4(3.0, 3.0, 1.0, 1.0),
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(1.0, 0.0, 0.0, 1.0),
        1.5, 0.0, 0.0,
        15.0, 128.0,
        vec3(0.0, 0.0, -1.0)
    )
);

material mate = material(
    vec4(0.2, 0.2, 0.2, 1.0),
    vec4(0.8, 0.8, 0.8, 0.8),
    vec4(0.0, 0.0, 0.0, 1.0),
    0.0
);

vec4 ambient_light = vec4(1.0, 1.0, 1.0, 1.0);

void main (void) {
    vec3
        lightDirection,
        normalDirection = normalize(varyingNormalDirection),
        viewDirection = normalize(-position.xyz);
    vec4 result = ambient_light * mate.ambient;
    float attenuation;

    for (int i = 0; i < total_lights; ++i) {

        vec4 diffuseReflection, specularReflection;

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

        diffuseReflection = attenuation *
            light[i].diffuse * mate.diffuse *
            max(0.0, dot(normalDirection, lightDirection));

        if (dot(normalDirection, lightDirection) < 0.0) { // light source on the wrong side?
            specularReflection = vec4(0.0, 0.0, 0.0, 0.0); // no specular reflection
        } else { // light source on the right side
            specularReflection = attenuation * light[i].specular * mate.specular *
                pow(max(0.0, dot(reflect(-lightDirection, normalDirection), viewDirection)), mate.shininess);
        }

        result += diffuseReflection + specularReflection;
    }

    finalColor = result + gl_FrontLightModelProduct.sceneColor;
}
