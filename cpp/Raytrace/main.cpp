#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <memory>
#include <limits>
#include <cmath>

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

#include "HitableList.h"
#include "Vec3.h"
#include "Mat4.h"
#include "Ray.h"
#include "Camera.h"
#include "Sphere.h"
#include "Rand.h"
#include "bmp.h"

const Vec3 rayColor(const Ray& r, const HitableList& world, const float depth) {
	// if we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return {0,0,0};

	auto rec {world.hit(r, 0.001f, std::numeric_limits<float>::infinity() )};
	if (rec) {
		auto result {rec->material->scatter(r, rec.value())};
		if (result) {
			return rayColor(result->second, world, depth-1)*result->first;
		}
		return {0,0,0};
	}

	// background
	const Vec3 unitDirection{Vec3::normalize(r.direction())};
	const float t = (unitDirection.y() + 1.0f)*0.5f;
	return Vec3{1.0f, 1.0f, 1.0f}*(1.0f-t) + Vec3{0.5f, 0.7f, 1.0f}*t;
}

float clamp(const float n, const float smallest, const float largest) {
	return fmax(smallest, fmin(n, largest));
}
	
const std::array<uint8_t,3> finalizeColor(const Vec3& pixelColor, const uint32_t samplesPerPixel) {
	float r{pixelColor.r()};
	float g{pixelColor.g()};
	float b{pixelColor.b()};

	// Replace NaN components with zero. See explanation in Ray Tracing: The Rest of Your Life.
	if (!std::isnormal(r)) r = 0.0f;
	if (!std::isnormal(g)) g = 0.0f;
	if (!std::isnormal(b)) b = 0.0f;

	// Divide the color by the number of samples and gamma-correct for gamma=2.0.
	const float scale {1.0f / samplesPerPixel};
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);
		
	// return the translated [0,255] value of each color component.
	return {uint8_t(256 * clamp(r, 0.0f, 0.999f)),
			uint8_t(256 * clamp(g, 0.0f, 0.999f)),
			uint8_t(256 * clamp(b, 0.0f, 0.999f))};

}

HitableList randomSphereScene() {
	HitableList world{};

	// Bottom "plane"
	world.add(std::make_shared<Sphere>(Vec3{0.0f,-1000.0f,0.0f}, 1000.0f, std::make_shared<Lambertian>(Vec3{0.5f, 0.5f, 0.5f})));

	// three large spheres
	world.add(std::make_shared<Sphere>(Vec3{0.0f, 1.0f, 0.0f}, 1.0f, std::make_shared<Dielectric>(1.5f)));
	world.add(std::make_shared<Sphere>(Vec3{-4.0f, 1.0f, 0.0f}, 1.0f, std::make_shared<Lambertian>(Vec3{0.4f, 0.2f, 0.1f})));
	world.add(std::make_shared<Sphere>(Vec3{4.0f, 1.0f, 0.0f}, 1.0f, std::make_shared<Metal>(Vec3{0.7f, 0.6f, 0.5f}, 0.0f)));
	
	// numerous small spheres
	for (int32_t a{-11}; a < 11;++a) {
		for (int32_t b{-11}; b < 11;++b) {
			const float chooseMat{Rand::rand01()};
			const Vec3 center{a + 0.9f*Rand::rand01(), 0.2f, b + 0.9f*Rand::rand01()};
			
			if ((center - Vec3{4.0f, 0.2f, 0.0f}).length() > 0.9f) {
				if (chooseMat < 0.8f) {
					// diffuse
					const Vec3 color{Vec3::random() * Vec3::random()};
					world.add(std::make_shared<Sphere>(center, 0.2f, std::make_shared<Lambertian>(color)));
				} else {
					if (chooseMat < 0.9f) {
						// metal
						const float c{Rand::rand005()};
						const Vec3 color{c,c,c};
						const float fuzz{Rand::rand051()};
						world.add(std::make_shared<Sphere>(center, 0.2f, std::make_shared<Metal>(color, fuzz)));
					} else {
						// glass
						world.add(std::make_shared<Sphere>(center, 0.2f, std::make_shared<Dielectric>(1.5f)));
					}
				}
			}
		}
	}
	return world;
}

int main(int argc, char *argv[]) {
	const float aspectRatio{16.0f/9.0f};
	const uint32_t imageWidth{1920};
	const uint32_t imageHeight{uint32_t(imageWidth / aspectRatio)};
	const uint32_t samplesPerPixel{50};
	const uint32_t maxDepth{50};

	const HitableList world{randomSphereScene()};

	Vec3 lookFrom{13,2,3};
	const Vec3 lookAt{0,0,0};
	const Vec3 vUp{0,1,0};
	const float distToFocus{10.0f};
	const float aperture{0.1f};	
		

	std::vector<uint8_t> image(imageHeight*imageWidth*4);

	const uint32_t stepping{5};
	const Mat4 m{Mat4::rotationY(stepping)};

	for (uint32_t k{0};k<360;k+=stepping) {
		auto t1 = Clock::now();
			
		lookFrom = m*lookFrom;
		const Camera cam{lookFrom, lookAt, vUp, 20, aspectRatio, aperture, distToFocus};
			
		#pragma omp parallel for
		for (int j=0; j < int(imageHeight); ++j) {
			std::cout << "." << std::flush;
			for (uint32_t i{0}; i < imageWidth; ++i) {
				Vec3 pixelColor{0,0,0};
				for (uint32_t s{0}; s < samplesPerPixel; ++s) {
					const float u{(i + Rand::rand01()) / (imageWidth-1)};
					const float v{(j + Rand::rand01()) / (imageHeight-1)};
					const Ray r{cam.getRay(u, v)};
					pixelColor = pixelColor + rayColor(r, world, maxDepth);
				}
				auto c = finalizeColor(pixelColor, samplesPerPixel);
				uint32_t index{(j * imageWidth + i)*4};
				image[index++] = c[0];
				image[index++] = c[1];
				image[index++] = c[2];
				image[index++] = 255;
			}
		}

			
		std::stringstream s;
		s << "result" << std::setfill('0') << std::setw(3) << k << ".bmp";
		
		auto t2 = Clock::now();
		std::cout << "Done with " << s.str() << " " << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << " ms passed" << std::endl;

		save(s.str(), imageWidth, imageHeight, image);

	}
	
}
