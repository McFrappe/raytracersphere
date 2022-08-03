#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"

struct Light {
    Light(const Vec3f &p, const float &i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};

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

bool scene_intersect(const Vec3f &origin, const Vec3f &dir, const Sphere &sphere, Vec3f &hit, Vec3f &N) {
    float sphere_dist = std::numeric_limits<float>::max();
		float dist_i;

		if (sphere.ray_intersect(origin, dir, dist_i) && dist_i < sphere_dist) {
				sphere_dist = dist_i;
				hit = origin + dir * dist_i;
				N = (hit - sphere.center).normalize();
    }

    return sphere_dist < 1000;
}

Vec3f cast_ray(const Vec3f &origin, const Vec3f &dir, const Sphere &sphere, const Light &light) {
	Vec3f point, N;

	if (!scene_intersect(origin, dir, sphere, point, N)) {
		return Vec3f(0.2, 0.2, 0.2); // The background color
	}

	float diffuse_light_intensity = 0;
	Vec3f light_dir = (light.position - point).normalize();
	diffuse_light_intensity += light.intensity * std::max(0.f, light_dir * N);

	return Vec3f(0.4, 0.4, 0.3) * diffuse_light_intensity;
}

void render(const Sphere &sphere, const Light &light) {
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
			framebuffer[j+i*width] = cast_ray(Vec3f(0, 0, 0), dir, sphere, light);
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
	Light light(Vec3f(-20, 20, 20), 1.5);

	render(sphere, light);

	return 0;
}
