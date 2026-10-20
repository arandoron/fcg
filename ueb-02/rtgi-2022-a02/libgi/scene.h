#pragma once

#include "camera.h"
#include "intersect.h"
#include "material.h"

#include <vector>
#include <map>
#include <string>
#include <filesystem>

namespace wf {
	class batch_ray_tracer;
}

template <typename T>
struct texture2d {
	std::string name;
	std::filesystem::path path;
	unsigned w, h;
	T *texel = nullptr;
	~texture2d() {
		delete [] texel;
	}
	const T& sample(float u, float v) const {
		u = u - floor(u);
		v = v - floor(v);
		int x = (int)(u*w+0.5f);
		int y = (int)(v*h+0.5f);
		if (x == w) x = 0;
		if (y == h) y = 0;
		return texel[y*w+x];
	}
	const T& sample(vec2 uv) const {
		return sample(uv.x, uv.y);
	}
	const T& operator()(float u, float v) const {
		return sample(u, v);
	}
	const T& operator()(vec2 uv) const {
		return sample(uv.x, uv.y);
	}
	const T& value(int x, int y) const {
		return texel[y*w+x];
	}
	const T& operator[](glm::uvec2 xy) const {
		return value(xy.x, xy.y);
	}
};

texture2d<vec3>* load_image3f(const std::filesystem::path &path, bool crash_on_error = true);
texture2d<vec4>* load_image4f(const std::filesystem::path &path, const std::filesystem::path *opacity_path = nullptr, bool crash_on_error = true);
texture2d<vec3>* load_hdr_image3f(const std::filesystem::path &path);






/*  \brief The scene culminates all the geometric information that we use.
 *
 *  This naturally includes the surface geometry to be displayed, but also light sources and cameras.
 *
 *  The scene also holds a reference to the ray tracer as the tracer runs on the scene's data.
 *
 */
struct scene {
	struct object {
		std::string name;
		unsigned start, end;
		unsigned material_id;
	};
	std::vector<::vertex>    vertices;
	std::vector<::triangle>  triangles;
	std::vector<::material>  materials;
	std::vector<::texture2d<vec4>*>  textures;
	std::vector<object>      objects;
	std::map<std::string, ::camera> cameras;
	::camera camera;
	vec3 up;
	aabb scene_bounds;
	const ::material material(uint32_t triangle_index) const {
		return materials[triangles[triangle_index].material_id];
	}
	scene() : camera(vec3(0,0,-1), vec3(0,0,0), vec3(0,1,0), 65, 1280, 720) {
	}
	~scene();
	void add(const std::filesystem::path &path, const std::string &name, const glm::mat4 &trafo = glm::mat4());

	vec3 normal(const triangle &tri) const;

	individual_ray_tracer *rt = nullptr;
	
	//! The scene takes ownership of the RT, deletes it upon destruction and when taking ownership of a new RT.
	void release_rt();
	void use(individual_ray_tracer *new_rt);
};

// std::vector<triangle> scene_triangles();
// scene load_scene(const std::string& filename);
