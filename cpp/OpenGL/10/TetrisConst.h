#pragma once 

#include <vector>
#include <array>
#include <Vec3.h>
#include <Vec2.h>


std::array<Vec2,4> test{Vec2{-1,0},Vec2{0,0},Vec2{1,0},Vec2{2,0}};

std::vector<std::vector<std::vector<Vec2>>> tetrominos{	
		{{Vec2{-1,0},Vec2{0,0},Vec2{1,0},Vec2{2,0}},
		 {Vec2{1,-1},Vec2{1,0},Vec2{1,1},Vec2{1,2}},
		 {Vec2{-1,1},Vec2{0,1},Vec2{1,1},Vec2{2,1}},
		 {Vec2{0,-1},Vec2{0,0},Vec2{0,1},Vec2{0,2}}},		// I, 4 orientations
		{{Vec2{-1,0},Vec2{0,0},Vec2{-1,1},Vec2{0,1}}},		// O, 1 orientation
		{{Vec2{-1,0},Vec2{0,0},Vec2{1,0},Vec2{0,1}},
		 {Vec2{-1,0},Vec2{0,0},Vec2{0,-1},Vec2{0,1}},
		 {Vec2{-1,0},Vec2{0,0},Vec2{1,0},Vec2{0,-1}},
		 {Vec2{1,0},Vec2{0,0},Vec2{0,-1},Vec2{0,1}}},		// T, 4 orientations
		{{Vec2{-1,0},Vec2{0,0},Vec2{1,0},Vec2{1,1}},
		 {Vec2{-1,1},Vec2{0,0},Vec2{0,-1},Vec2{0,1}},
		 {Vec2{-1,0},Vec2{0,0},Vec2{1,0},Vec2{-1,-1}},
		 {Vec2{1,-1},Vec2{0,0},Vec2{0,-1},Vec2{0,1}}},		// J, 4 orientations
		{{Vec2{-1,0},Vec2{0,0},Vec2{1,0},Vec2{-1,1}},
		 {Vec2{0,-1},Vec2{0,0},Vec2{0,1},Vec2{-1,-1}},
		 {Vec2{-1,0},Vec2{0,0},Vec2{1,0},Vec2{1,-1}},
		 {Vec2{0,-1},Vec2{0,0},Vec2{0,1},Vec2{1,1}}},		// L, 4 orientations
		{{Vec2{-1,1},Vec2{0,1},Vec2{0,0},Vec2{1,0}},
		 {Vec2{-1,-1},Vec2{-1,0},Vec2{0,0},Vec2{0,1}},
		 {Vec2{-1,0},Vec2{0,0},Vec2{0,-1},Vec2{1,-1}},
		 {Vec2{0,-1},Vec2{0,0},Vec2{1,0},Vec2{1,1}}},		// S, 4 orientations
		{{Vec2{-1,0},Vec2{0,0},Vec2{0,1},Vec2{1,1}},
		 {Vec2{0,-1},Vec2{0,0},Vec2{-1,0},Vec2{-1,1}},
		 {Vec2{-1,-1},Vec2{0,-1},Vec2{0,0},Vec2{1,0}},
		 {Vec2{1,-1},Vec2{1,0},Vec2{0,0},Vec2{0,1}}}  		 // Z, 4 orientations
};

std::vector<Vec3> colors{
		Vec3{0,1,1},		// cyan
		Vec3{1,1,0},		// yellow
		Vec3{1,0,1},		// magenta
		Vec3{0,0,1},		// blue
		Vec3{0.6,0.15,0},	// brown
		Vec3{0,1,0},		// green
		Vec3{1,0,0},  	   	// red
		Vec3{0,0,0}			// black
};
