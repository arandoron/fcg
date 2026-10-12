#include "camera.h"

#include "random.h"

#include <fstream>

using namespace glm;
using namespace std;

//! Set up a camera ray using cam.up, cam.dir, cam.w, cam.h (see \ref camera::update_frustum)
ray cam_ray(const camera &cam, int x, int y, vec2 offset) {
	// todo: compute a camera ray a given each pixel position.
	// note: you can use the function `test_camrays' to export them as an obj model and look at them in blender.
	return ray(cam.pos, vec3(0,0,-1));
}

void test_camrays(const camera &camera, int stride) {
	ofstream out("test.obj");
	int i = 1;
	for (int y = 0; y < camera.h; y += stride)
		for (int x = 0; x < camera.w; x += stride) {
			ray ray = cam_ray(camera, x, y);
			out << "v " << ray.o.x << " " << ray.o.y << " " << ray.o.z << endl;
			out << "v " << ray.d.x << " " << ray.d.y << " " << ray.d.z << endl;
			out << "l " << i++ << " " << i++ << endl;
		}
}


