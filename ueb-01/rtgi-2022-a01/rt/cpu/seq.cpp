#include "seq.h"

#include "libgi/scene.h"

#include <iostream>

using namespace std;

triangle_intersection seq_tri_is::closest_hit(const ray &ray) {
  // NOTE: Assignment 4
  triangle_intersection closest, intersection;

  int num_triangles = scene->triangles.size();
  triangle closest_tri;

  for (int i = 0; i < num_triangles; i++) {
    triangle tri = scene->triangles.at(i);

    bool hit = intersect(tri, scene->vertices.data(), ray, closest);

    if (hit && closest.t < intersection.t) {
      intersection.t = closest.t;
      intersection.beta = closest.beta;
      intersection.gamma = closest.gamma;
      closest_tri = tri;
    }
  }

  return closest;
}

bool seq_tri_is::any_hit(const ray &ray) {
  triangle_intersection intersection;
  return false;
}
