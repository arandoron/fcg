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
	// todo: implement primary hitpoint algorithm
	return vec3(0);
}


