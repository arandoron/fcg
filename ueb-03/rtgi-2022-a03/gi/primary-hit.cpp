#include "primary-hit.h"

#include "config.h"

#ifdef HAVE_GL
#include "driver/preview.h"
#endif

#include "libgi/rt.h"
#include "libgi/context.h"
#include "libgi/intersect.h"
#include "libgi/util.h"
#include "libgi/color.h"

#include "libgi/global-context.h"

using namespace glm;
using namespace std;

vec3 primary_hit_display::sample_pixel(uint32_t x, uint32_t y) {
	vec3 radiance(0);
	ray view_ray = cam_ray(rc->scene.camera, x, y, glm::vec2(rc->rng.uniform_float()-0.5f, rc->rng.uniform_float()-0.5f));
	triangle_intersection closest = rc->scene.rt->closest_hit(view_ray);
	if (closest.valid()) {
		diff_geom dg(closest, rc->scene);
		radiance = dg.albedo();
	}
	return radiance;
}

vec3 local_illumination::sample_pixel(uint32_t x, uint32_t y) {
	vec3 radiance(0);
	ray view_ray = cam_ray(rc->scene.camera, x, y, glm::vec2(rc->rng.uniform_float()-0.5f, rc->rng.uniform_float()-0.5f));
	triangle_intersection closest = rc->scene.rt->closest_hit(view_ray);
	if (closest.valid()) {
		diff_geom dg(closest, rc->scene);
		brdf *brdf = dg.mat->brdf;
		assert(!rc->scene.lights.empty());
		pointlight *pl = dynamic_cast<pointlight*>(rc->scene.lights[0]);
		assert(pl);
		// todo: implement local illumination via the BRDF
		radiance = dg.albedo();
	}
	return radiance;
}

