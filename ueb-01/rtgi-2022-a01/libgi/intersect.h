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
inline bool intersect(const triangle &tri, const vertex *vertices,
                      const ray &ray, triangle_intersection &info) {
  // NOTE: Assignment 3

  vec3 vec_a = vertices[tri.a].pos;
  vec3 vec_b = vertices[tri.b].pos;
  vec3 vec_c = vertices[tri.c].pos;
  vec3 vec_d = ray.d;
  vec3 vec_o = ray.o;

  // values for the matrix A:
  //
  // [
  //  a, b, c
  //  d, e, f
  //  g, h, i
  // ]
  //
  // values for the vector b:
  //
  // [
  //  j
  //  k
  //  l
  // ]

  float a = vec_a.x - vec_b.x;
  float b = vec_a.x - vec_c.x;
  float c = vec_d.x;

  float d = vec_a.y - vec_b.y;
  float e = vec_a.y - vec_c.y;
  float f = vec_d.y;

  float g = vec_a.z - vec_b.z;
  float h = vec_a.z - vec_c.z;
  float i = vec_d.z;

  float ei_minus_fh = e * i - f * h;
  float di_minus_gf = d * i - g * f;
  float dh_minus_eg = d * h - e * g;

  float j = vec_a.x - vec_o.x;
  float k = vec_a.y - vec_o.y;
  float l = vec_a.z - vec_o.z;

  float det_A = a * ei_minus_fh - b * di_minus_gf + c * dh_minus_eg;

  if (det_A == 0) {
    return false;
  }

  info.t =
      (a * (e * l - k * h) - b * (d * l - g * k) + j * dh_minus_eg) / det_A;

  if (info.t < 0) {
    return false;
  }

  info.beta =
      (j * ei_minus_fh - b * (k * i - l * f) + c * (k * h - e * l)) / det_A;

  if (info.beta < 0 || info.beta > 1) {
    return false;
  }

  info.gamma =
      (a * (k * i - f * l) - j * di_minus_gf + c * (d * l - k * g)) / det_A;

  if (info.gamma < 0 || info.gamma > 1 || info.beta + info.gamma > 1) {
    return false;
  }

  return true;
}
