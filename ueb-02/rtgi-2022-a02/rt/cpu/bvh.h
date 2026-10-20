#pragma once

#include "libgi/scene.h"
#include "libgi/intersect.h"

#include <vector>
#include <float.h>
#include <glm/glm.hpp>

constexpr const float ALPHA_THRESHOLD = 0.5f;
// #define COUNT_HITS

struct naive_bvh : public individual_ray_tracer {
	struct node {
		aabb box;
		uint32_t left, right;
		uint32_t triangle = (uint32_t)-1;
		bool inner() const { return triangle == (uint32_t)-1; }
	};

	std::vector<node> nodes;
	uint32_t root;
	void build(::scene *scene);
private:
	uint32_t subdivide(std::vector<triangle> &triangles, std::vector<vertex> &vertices, uint32_t start, uint32_t end);
	triangle_intersection closest_hit(const ray &ray) override;
	bool any_hit(const ray &ray) override;
};
	
/* Innere und Blattknoten werden durch trickserei unterschieden.
 * Für Blattknoten gilt:
 * - link_l = -tri_offset
 * - link_r = -tri_count
 */
struct bbvh_node {
	aabb box_l, box_r;
	int32_t link_l, link_r;
	bool inner() const { return link_r >= 0; }
	int32_t tri_offset() const { return -link_l; }
	int32_t tri_count()  const { return -link_r; }
	void tri_offset(int32_t offset) { link_l = -offset; }
	void tri_count(int32_t count) { link_r = -count; }
};
static_assert(sizeof(bbvh_node) == 2*2*3*4+2*4);

struct binary_bvh_tracer : public individual_ray_tracer {
	typedef bbvh_node node;

	std::vector<node> nodes;
	int32_t root = 0;

	enum binary_split_type {sm, om };
	binary_split_type split_type = om;

	// config options
	int max_triangles_per_node = 1;
	int number_of_planes = 1;
	
	// helper
	vec3 center(const triangle &t);
	
	binary_bvh_tracer(binary_split_type type);
	void build(::scene *scene) override;
	triangle_intersection closest_hit(const ray &ray) override;
	bool any_hit(const ray &ray) override;
	bool interprete(const std::string &command, std::istringstream &in) override;

private:
	int32_t subdivide_om(int start, int end, aabb box); 
	int32_t subdivide_sm(int start, int end, aabb box); 
};

