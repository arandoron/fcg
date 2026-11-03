#include "primary-hit.h"

#include "config.h"
#include "libgi/material.h"
#include <glm/geometric.hpp>

#ifdef HAVE_GL
#include "driver/preview.h"
#endif

#include "libgi/color.h"
#include "libgi/context.h"
#include "libgi/intersect.h"
#include "libgi/rt.h"
#include "libgi/util.h"

#include "libgi/global-context.h"

using namespace glm;
using namespace std;

vec3 primary_hit_display::sample_pixel(uint32_t x, uint32_t y) {
  vec3 radiance(0);
  ray view_ray = cam_ray(rc->scene.camera, x, y,
                         glm::vec2(rc->rng.uniform_float() - 0.5f,
                                   rc->rng.uniform_float() - 0.5f));
  triangle_intersection closest = rc->scene.rt->closest_hit(view_ray);
  if (closest.valid()) {
    diff_geom dg(closest, rc->scene);
    radiance = dg.albedo();
  }
  return radiance;
}

// TODO: figure out when this function gets called
vec3 local_illumination::sample_pixel(uint32_t x, uint32_t y) {
  vec3 radiance(0);
  ray view_ray = cam_ray(rc->scene.camera, x, y,
                         glm::vec2(rc->rng.uniform_float() - 0.5f,
                                   rc->rng.uniform_float() - 0.5f));
  triangle_intersection closest = rc->scene.rt->closest_hit(view_ray);
  if (closest.valid()) {
    diff_geom dg(closest, rc->scene);
    brdf *brdf = dg.mat->brdf;
    assert(!rc->scene.lights.empty());
    pointlight *pl = dynamic_cast<pointlight *>(rc->scene.lights[0]);
    assert(pl);
    // NOTE: Assignment 1
    // implement local illumination via the BRDF

    // direction to the camera
    // vec3 to_camera = rc->scene.camera.pos - dg.x;
    vec3 w_o = -view_ray.d;
    // direction to the light source
    vec3 to_light = pl->pos - dg.x;
    vec3 w_i = normalize(to_light);
    // distance between light source and x
    // to_light is the non-normalized vector between light source and x
    // square root of the dot product of itself yields the length of that vector
    float d = sqrt(dot(to_light, to_light));

    radiance = pl->power() * brdf->f(dg, w_o, w_i) / (d * d);
    // TODO: nachfragen ob das hier nötig ist oder nicht, da es in der Lösung
    // nicht zu finden ist (soweit ich sehen konnte)

    // lambert's cosine law
    // => multiply with cos(theta) to normalize a larger surface area being
    // "hit" if the light source hits the surface at an angle
    // => less intensity at those spots
    radiance = radiance * dot(dg.ns, w_i);
  }
  return radiance;
}
