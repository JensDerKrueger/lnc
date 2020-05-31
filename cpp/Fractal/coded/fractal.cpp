#include <cstdint>
#include <cmath>
#include <vector>
#include <iostream>

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

#include "bmp.h"

using namespace std;

template <typename T>
class complex {
	public:
		complex(T r, T i) :
			r(r),
			i(i)
		{}
		
		complex operator+( const complex& val ) {
			return complex(r+val.r,i+val.i);
		}
		
		complex operator*( const complex& val ) {
			return complex(r*val.r - i*val.i, r*val.i +i*val.r);
		}
		
		T real() const {
			return r;
		}

		T img() const {
			return i;
		}
		
		T abs() const {
			return sqrt(i*i  + r*r);
		}

		T sq_abs() const {
			return i*i  + r*r;
		}		
				
	private:
		T r,i;
};
	
uint8_t mandelbrot(complex<double> c) {
	complex<double> z(0,0);
	uint16_t i = 0;
	while (i <= 255 && z.sq_abs() < 4) {
		z = z*z+c;
		i += 1;
	}
	return uint8_t(i);
}


void indexToColor(std::vector<uint8_t>& picture, uint32_t index, uint8_t i) {
	picture[index+0] = (i*1)%256;
	picture[index+1] = (i*3)%256;
	picture[index+2] = (i*25)%256;
	picture[index+3] = 255;
}
	
	
int main(int argc, char** argv) {	
	uint32_t width = 4096;
	uint32_t height = 4096;

	<uint8_t> picture(width * height * 4);

	double zoom = 1.0;
	double xs = -2.1;
	double ys = -1.3;

	double dx = (2.6/zoom) / width;
	double dy = (2.6/zoom) / height;

	auto t1 = Clock::now();

	#pragma omp parallel for
	for (int y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			complex<double> c(xs+x*dx,ys+y*dy);
			uint8_t colorIndex = mandelbrot(c);
			indexToColor(picture, (x+y*width)*4, colorIndex);
		}
	}
	
	auto t2 = Clock::now();
	
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << " ms passed" << std::endl;
	
	save("fractal.bmp", width, height, picture);
	
	return EXIT_SUCCESS;
}