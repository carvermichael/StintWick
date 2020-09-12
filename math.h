#pragma once

#pragma warning (push, 0)
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma warning (pop)

#define PI 3.14159f

struct my_vec2 {
	float x;
	float y;

	my_vec2() {}

	my_vec2(float x, float y) {
		this->x = x;
		this->y = y;
	}

	my_vec2(float all) {
		this->x = all;
		this->y = all;
	}
};
struct my_ivec2 {
	int x;
	int y;

	my_ivec2() {}

	my_ivec2(int newX, int newY) {
		x = newX;
		y = newY;
	}

	my_ivec2(int all) {
		x = all;
		y = all;
	}
};

struct my_vec3 {
	float x;
	float y;
	float z;

	my_vec3() {}

	my_vec3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	my_vec3(float all) {
		this->x = all;
		this->y = all;
		this->z = all;
	}

	void operator+=(my_vec3 v) {
		this->x = this->x + v.x;
		this->y = this->y + v.y;
		this->z = this->z + v.z;
	}

	void operator-=(my_vec3 v) {
		this->x = this->x - v.x;
		this->y = this->y - v.y;
		this->z = this->z - v.z;
	}

	bool operator==(my_vec3 v) {
		return this->x == v.x &&
			   this->y == v.y &&
			   this->z == v.z;
	}
};

struct my_ivec3 {
	int x;
	int y;
	int z;

	my_ivec3() {}

	my_ivec3(int x, int y, int z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	my_ivec3(int all) {
		this->x = all;
		this->y = all;
		this->z = all;
	}
};

struct my_vec4 {
	float x;
	float y;
	float z;
	float w;

	my_vec4() {}

	my_vec4(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	my_vec4(float all) {
		this->x = all;
		this->y = all;
		this->z = all;
		this->w = all;
	}
};

// TODO:
// not sure how to refer to this
// probably want to overload the [] operators
// need to find more usage code...

// This has to be aligned in a very specific way, cause
// openGL, when setting mat4 uniforms, takes a pointer
// to the first entry, and thus assumes the entire structure.
struct mat4 {

	

};

struct AABB {

	union {
		struct {
			float AX;
			float AY;
			float BX;
			float BY;
		};

		struct {
			float left;
			float top;
			float right;
			float bottom;
		};
	};

	void set(my_vec2 topLeft, float height, float width) {
		AX = topLeft.x;
		AY = topLeft.y;
		BX = topLeft.x + width;
		BY = topLeft.y - height;
	}

	AABB() {};
	
	AABB(float left, float right, float top, float bottom) {
		this->top		= top;
		this->bottom	= bottom; 
		this->left		= left;
		this->right		= right;
	}
};

// ----- operators -----
my_vec3 operator-(my_vec3 one, my_vec3 two) {
	return my_vec3(one.x - two.x, one.y - two.y, one.z - two.z);
}

my_vec3 operator+(my_vec3 one, my_vec3 two) {
	return my_vec3(one.x + two.x, one.y + two.y, one.z + two.z);
}

my_vec3 operator*(my_vec3 v, float f) {
	return my_vec3(v.x * f, v.y * f, v.z * f);
}

my_vec3 operator*(float f, my_vec3 v) {
	return my_vec3(v.x * f, v.y * f, v.z * f);
}

my_vec2 operator*(my_vec2 v, float f) {
	return my_vec2(v.x * f, v.y * f);
}

my_vec2 operator*(float f, my_vec2 v) {
	return my_vec2(v.x * f, v.y * f);
}

float mapToNewRange(float x, float minBefore, float maxBefore, float minAfter, float maxAfter) {

	float beforeRange = maxBefore - minBefore;
	float afterRange = maxAfter - minAfter;
	return x / (beforeRange / afterRange) + minAfter;
}

// TODO
//float abs(float x) {
//	assert(0);
//}

my_vec2 normalize(my_vec2 in) {
	float magnitude = sqrt(in.x*in.x + in.y*in.y);

	return in * (1 / magnitude);	
}

my_vec3 normalize(my_vec3 v) {
	float magnitude = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);

	return v * (1 / magnitude);
}

my_vec3 crossproduct(my_vec3 a, my_vec3 b) {
	return my_vec3(a.y*b.z - a.z*b.y,
				a.z*b.x - a.x*b.z,
				a.x*b.y - a.y*b.x);
}

glm::vec3 toGLM(my_vec3 v) {
	return glm::vec3(v.x, v.y, v.z);
}

// TODO
mat4 perspective(float fovy, float aspect, float zNear, float zFar) {
	assert(0);

	return mat4();
}

// TODO
mat4 ortho(float left, float right, float bottom, float top) {
	assert(0);

	return mat4();
}

// TODO
mat4 translate(mat4 inMat4, my_vec3 inVec3) {
	assert(0);

	return mat4();
}

// TODO
mat4 lookAt(my_vec3 pos, my_vec3 posFront, my_vec3 up) {
	assert(0);

	return mat4();
}

// ----- trig -----
float radians(float degrees) {
	return degrees * PI / 180;
}

//// punting on these two, using cmath for now
//// might implement this later: http://www.mathonweb.com/help_ebook/html/algorithms.htm
//float sin(float radians) {
//	return sinf(radians);
//}
//
//float cos(float radians) {
//	return cosf(radians);
//}