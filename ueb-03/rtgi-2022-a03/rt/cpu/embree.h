#pragma once


#include "libgi/intersect.h"
#include "embree3/rtcore.h"
#include "rt/cpu/bvh.h"

#include <iostream>



template <bool alpha_aware = false>
struct embree_tracer : public individual_ray_tracer {
public:
	embree_tracer();
	~embree_tracer();
	void build(::scene *scene);
	bool any_hit(const ray &ray);
	triangle_intersection closest_hit(const ray &ray);
	RTCDevice em_device;

private:
	RTCScene em_scene;
};

//BVH Generation Callbacks

struct bvh_callback_data {
	std::vector<bvh::node> bvh_nodes;
	pthread_rwlock_t lock;
};

void* embvh_create_node(RTCThreadLocalAllocator alloc,
						unsigned int num_children,
						void *user_ptr);

void embvh_set_node_children(void *node_ptr,
							 void **children,
							 unsigned int child_count,
							 void* user_ptr);

void embvh_set_node_bounds(void *node_ptr,
						   const struct RTCBounds **bounds,
						   unsigned int child_count, void *user_ptr);

void* embvh_create_leaf(RTCThreadLocalAllocator allocator,
						const struct RTCBuildPrimitive *primitives,
						size_t primitive_count, void *user_ptr);

void embvh_split_primitive(const struct RTCBuildPrimitive *primitive,
						   unsigned int dimension,
						   float position,
						   struct RTCBounds *left_bounds,
						   struct RTCBounds *right_bounds,
						   void* user_ptr);
