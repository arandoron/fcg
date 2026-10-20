#include "bvh.h"

#include "libgi/timer.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#define error(x) { std::cerr << "command " << " (" << command << "): " << x << std::endl; return true;}
#define check_in_complete(x) { if (in.bad() || in.fail() || !in.eof()) error(x); }
using namespace glm;


// 
//    a more realistic binary bvh
//

	
binary_bvh_tracer::binary_bvh_tracer(binary_split_type type) : split_type(type) {
}

vec3 binary_bvh_tracer::center(const triangle &t) {
	return (scene->vertices[t.a].pos + scene->vertices[t.b].pos + scene->vertices[t.c].pos) / 3.0f;
}

void binary_bvh_tracer::build(::scene *scene) {
	time_this_block(build_bvh);
	this->scene = scene;
	std::cout << "Building BVH..." << std::endl;
	auto t1 = std::chrono::high_resolution_clock::now();
	
	aabb box;
	for (int i = 0; i < scene->triangles.size(); ++i) {
		box.grow(scene->vertices[scene->triangles[i].a].pos);
		box.grow(scene->vertices[scene->triangles[i].b].pos);
		box.grow(scene->vertices[scene->triangles[i].c].pos);
	}

	if (split_type == om)      root = subdivide_om(0,  scene->triangles.size(), box);
	else if (split_type == sm) root = subdivide_sm(0,  scene->triangles.size(), box);

	std::cout << nodes[0].box_l.min << " x " << nodes[0].box_l.max << std::endl;
	std::cout << nodes[0].box_r.min << " x " << nodes[0].box_r.max << std::endl;

	auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	std::cout << "Done after " << duration << "ms" << std::endl;
}
	
int32_t binary_bvh_tracer::subdivide_om(int start, int end, aabb box) {
	// todo
	std::logic_error("Not implemented, yet");
	// Achtung, wenn die aktuelle Node den Index id hat, dann den rekursiven Aufruf nicht so machen:
	//   node[id].link_l = subdivide(...)
	// Warum: zuerst wird "node[id].link_l" ausgewertet und referenziert speicher,
	//        danach wird subdivide ausgeführt und re-allokiert evtl den vektor,
	//        damit ist die zuvor schon referenzierte speicherstelle nicht mehr gültig!
	return 0;
}

int32_t binary_bvh_tracer::subdivide_sm(int start, int end, aabb box) {
	// todo
	std::logic_error("Not implemented, yet");
	return 0;
}

triangle_intersection binary_bvh_tracer::closest_hit(const ray &ray) {
	// todo
	std::logic_error("Not implemented, yet");
	return triangle_intersection();
}

bool binary_bvh_tracer::any_hit(const ray &ray) {
	// todo
	std::logic_error("Not implemented, yet");
	return false;
}

bool binary_bvh_tracer::interprete(const std::string &command, std::istringstream &in) {
	return false;
}

