#pragma once

#include "rt.h"

struct aabb {
	vec3 min, max;
	aabb() : min(FLT_MAX), max(-FLT_MAX) {}
	void grow(vec3 v) {
		min = glm::min(v, min);
		max = glm::max(v, max);
	}
	void grow(const aabb &other) {
		min = glm::min(other.min, min);
		max = glm::max(other.max, max);
	}
};

// See Shirley (2nd Ed.), pp. 206. (library or excerpt online)
inline bool intersect(const triangle &t, const vertex *vertices, const ray &ray, triangle_intersection &info) {
	// todo
	return false;
}	

