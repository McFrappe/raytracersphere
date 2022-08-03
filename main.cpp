#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"

const int fov = 90;

struct Sphere {
	Vec3f center;
	float radius;

	Sphere(const Vec3f &c, const float &r) : center(c), radius(r) {}

	bool ray_intersect(const Vec3f &origin, const Vec3f &dir, float &t0) const {
		Vec3f L = center - origin;
		float tca = L * dir;
		float d2 = L * L - tca * tca;
		if (d2 > radius * radius) return false;

		float thc = sqrtf(radius * radius - d2);
		t0 = tca - thc;
		float t1 = tca + thc;
		if (t0 < 0) {
			t0 = 1;
			return false;
		}

		return true;
	}
};

Vec3f cast_ray(const Vec3f &origin, const Vec3f &dir, const Sphere &sphere) {
	float sphere_dist = std::numeric_limits<float>::max();
	if (!sphere.ray_intersect(origin, dir, sphere_dist)) {
		return Vec3f(0.2, 0.7, 0.8); // The background color
	}

	return Vec3f(0.4, 0.4, 0.3);
}

void render(const Sphere &sphere) {
	const int width    = 1024;
	const int height   = 768;
	const int fov = M_PI/2.;
	std::vector<Vec3f> framebuffer(width*height);

	#pragma omp parallel for
	for (size_t i = 0; i < height; i++) {
		for (size_t j = 0; j < width; j++) {
			float x = (2 * (j + 0.5) / (float)width - 1) * tan(fov/2.) * width / (float)height;
			float y = (2 * (i + 0.5) / (float)height - 1) * tan(fov/2.);
			Vec3f dir = Vec3f(x, y, -1).normalize();
			framebuffer[j+i*width] = cast_ray(Vec3f(0, 0, 0), dir, sphere);
		}
	}

	// save the framebuffer to file
	std::ofstream ofs;
	ofs.open("./out.ppm");
	ofs << "P6\n" << width << " " << height << "\n255\n";
	for (size_t i = 0; i < height * width; ++i) {
		for (size_t j = 0; j<3; j++) {
			ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
		}
	}
	ofs.close();
}

int main() {
	Sphere sphere(Vec3f(-3, 0, -16), 2);
	render(sphere);

	return 0;
}
