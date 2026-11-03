#include "material.h"
#include "scene.h"
#include "util.h"

using namespace glm;

vec3 layered_brdf::f(const diff_geom &geom, const vec3 &w_o, const vec3 &w_i) {
  // todo proper fresnel reflection for layered material
  vec3 diff = base->f(geom, w_o, w_i);
  vec3 spec = coat->f(geom, w_o, w_i);
  return diff + spec;
}

// lambertian_reflection

vec3 lambertian_reflection::f(const diff_geom &geom, const vec3 &w_o,
                              const vec3 &w_i) {
  if (!same_hemisphere(w_i, geom.ns))
    return vec3(0);
  // NOTE: Assignment 1
  // albedo = rho = surface color = ratio of reflected outgoing light to
  // incident light at a point
  return one_over_pi * geom.albedo();
}

// specular_reflection

vec3 phong_specular_reflection::f(const diff_geom &geom, const vec3 &w_o,
                                  const vec3 &w_i) {
  if (!same_hemisphere(w_i, geom.ng))
    return vec3(0);
  // NOTE: Assignment 2

  // TODO: better solution for this?
  vec3 double_ns = geom.ns;
  double_ns.x = double_ns.x * 2;
  double_ns.y = double_ns.y * 2;
  double_ns.z = double_ns.z * 2;

  // direction of reflection of the light (mirror of w_i)
  vec3 w_r = double_ns * dot(geom.ns, w_i) - w_i;

  // TODO: check this again what I have to return, also look into note in the
  // assignment description
  float shininess_value = exponent_from_roughness(geom.mat->roughness);

  float reflection = pow(dot(w_r, w_o), shininess_value);
  // normalize
  reflection = ((shininess_value + 2) / 2 * pi) * reflection;

  return reflection * geom.albedo();
}

// In the following, make sure that corner cases are taken care of (positive
// dot products, >0 denominators, correctly aligned directions, etc)

inline float ggx_d(const float NdotH, float roughness) {
  // todo TR NDF
  return 0.0f;
}

inline float ggx_g1(const float NdotV, float roughness) {
  // todo TR Smith G_1
  return 0.0f;
}

vec3 gtr2_reflection::f(const diff_geom &geom, const vec3 &w_o,
                        const vec3 &w_i) {
  // todo full TR Microfacet BRDF, use ggx_d and ggx_g1
  return vec3(0);
}

brdf *new_brdf(const std::string name, scene &scene) {
  if (scene.brdfs.count(name) == 0) {
    brdf *f = nullptr;
    if (name == "lambert")
      f = new lambertian_reflection;
    else if (name == "phong")
      f = new phong_specular_reflection;
    else if (name == "layered-phong") {
      brdf *base = new_brdf("lambert", scene);
      specular_brdf *coat =
          dynamic_cast<specular_brdf *>(new_brdf("phong", scene));
      assert(coat);
      f = new layered_brdf(coat, base);
    } else if (name == "gtr2")
      f = new gtr2_reflection;
    else if (name == "layered-gtr2") {
      throw std::runtime_error(std::string("Not implemented yet: ") + name);
    } else
      throw std::runtime_error(std::string("No such brdf defined: ") + name);
    return f;
  }
  return scene.brdfs[name];
}
