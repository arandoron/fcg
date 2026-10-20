#pragma once

#include "rt.h"

#include <glm/glm.hpp>
#include <string>
#include <tuple>

template <typename T>
struct texture2d;
    
inline float roughness_from_exponent(float exponent) {
	return sqrtf(2.f / (exponent + 2.f));
}
inline float exponent_from_roughness(float roughness) {
	return 2 / (roughness * roughness) - 2;
}


struct material {
	std::string name;
	vec3 albedo = vec4(0);
	vec3 emissive = vec3(0);
	texture2d<vec4> *albedo_tex = nullptr;
	float ior = 1.3f, roughness = 0.1f;
	struct brdf *brdf = nullptr;
};


