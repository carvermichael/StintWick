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

	my_vec4() {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 0.0f;
	}

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

	float operator[](int i) {
		assert(i >= 0 && i < 4);

		if (i == 0) return x;
		if (i == 1) return y;
		if (i == 2) return z;
		if (i == 3) return w;

		return x;
	}
};

struct my_mat4 {

	my_vec4 col0;
	my_vec4 col1;
	my_vec4 col2;
	my_vec4 col3;	

	my_mat4() {

	}

	my_mat4(float diag) {
		col0.x = diag;
		col1.y = diag;
		col2.z = diag;
		col3.w = diag;
	}

	my_mat4(glm::mat4 glmMat4) {
		this->col0.x = glmMat4[0][0];
		this->col0.y = glmMat4[0][1];
		this->col0.z = glmMat4[0][2];
		this->col0.w = glmMat4[0][3];

		this->col1.x = glmMat4[1][0];
		this->col1.y = glmMat4[1][1];
		this->col1.z = glmMat4[1][2];
		this->col1.w = glmMat4[1][3];

		this->col2.x = glmMat4[2][0];
		this->col2.y = glmMat4[2][1];
		this->col2.z = glmMat4[2][2];
		this->col2.w = glmMat4[2][3];

		this->col3.x = glmMat4[3][0];
		this->col3.y = glmMat4[3][1];
		this->col3.z = glmMat4[3][2];
		this->col3.w = glmMat4[3][3];
	}

	my_vec4 operator[](int i) {
		assert(i >= 0 && i < 4);

		if (i == 0) return col0;
		if (i == 1) return col1;
		if (i == 2) return col2;
		if (i == 3) return col3;

		return col1;
	}
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

float dot(my_vec3 a, my_vec3 b) {
	return a.x * b.x +
		   a.y * b.y +
		   a.z * b.z;
}

glm::vec3 toGLM(my_vec3 v) {
	return glm::vec3(v.x, v.y, v.z);
}

// Used to create the cut-off pyramid frustum in going from view space
// to clip space. Current use: for standard 3D camera perspective (depth
// testing needs to be on, of course).
my_mat4 perspective(float fovy, float aspect, float zNear, float zFar) {
	// TODO

	assert(0);

	return my_mat4();
}

// Used to create a rectangular frustrum in going from view space
// to clip space. Current use: for UI elements (note: depth testing is 
// off here, too).
my_mat4 ortho(float left, float right, float bottom, float top) {
	// TODO

	assert(0);

	return my_mat4();
}


// Puts the vec3 into the last column of the given matrix.
// The matrix then adds the effect of moving a vec4 by the amount given in the
// last column, assuming that the w of that vec4 is 1.

// Typical use: adding a worldOffset (the vec3) to the identity matrix, then 
//				using the result as the model matrix when going from
//				local space to world space.
my_mat4 translate(my_mat4 inMat4, my_vec3 inVec3) {
	inMat4.col3.x = inVec3.x;
	inMat4.col3.y = inVec3.y;
	inMat4.col3.z = inVec3.z;

	return inMat4;
}

// Used for generating the view matrix, which then is used
// to translate from world space to view space.
my_mat4 lookAt(my_vec3 pos, my_vec3 posFront, my_vec3 up) {
	// TODO

	my_mat4 mat4 = my_mat4(1.0f);

	/*
	 Need:
		position --> pos
		right --> 
		up --> up
		front --> posFront - pos
		    -- still not sure why common implementations of lookAt take (pos + front) 
			   as a single param, then pull out front immediately (diff invocation
			   circumstances?)

	*/

	// Still do not fully grok how this works, specifically the negations.
	// More work is needed to build intuitions here.

	my_vec3 front = normalize(posFront - pos);
	my_vec3 right = crossproduct(front, up);
	up = crossproduct(right, front);

	mat4.col0.x = right.x;
	mat4.col0.y = up.x;
	mat4.col0.z = -front.x;
	mat4.col0.w = 0.0f;

	mat4.col1.x = right.y;
	mat4.col1.y = up.y;
	mat4.col1.z = -front.y;
	mat4.col1.w = 0.0f;

	mat4.col2.x = right.z;
	mat4.col2.y = up.z;
	mat4.col2.z = -front.z;
	mat4.col2.w = 0.0f;

	mat4.col3.x = -dot(pos, right); // also might be reversed
	mat4.col3.y = -dot(pos, up);
	mat4.col3.z = dot(pos, front);
	mat4.col3.w = 1.0f;

	return mat4;
}

void printGLMMat4(glm::mat4 mat4) {
	printf("---------------------------------\n");
	printf("%.2f, %.2f, %.2f, %.2f\n", mat4[0][0], mat4[1][0], mat4[2][0], mat4[3][0]);
	printf("%.2f, %.2f, %.2f, %.2f\n", mat4[0][1], mat4[1][1], mat4[2][1], mat4[3][1]);
	printf("%.2f, %.2f, %.2f, %.2f\n", mat4[0][2], mat4[1][2], mat4[2][2], mat4[3][2]);
	printf("%.2f, %.2f, %.2f, %.2f\n", mat4[0][3], mat4[1][3], mat4[2][3], mat4[3][3]);
	printf("---------------------------------\n");
}

void printMat4(my_mat4 mat4) {
	printf("---------------------------------\n");
	printf("%.2f, %.2f, %.2f, %.2f\n", mat4[0][0], mat4[1][0], mat4[2][0], mat4[3][0]);
	printf("%.2f, %.2f, %.2f, %.2f\n", mat4[0][1], mat4[1][1], mat4[2][1], mat4[3][1]);
	printf("%.2f, %.2f, %.2f, %.2f\n", mat4[0][2], mat4[1][2], mat4[2][2], mat4[3][2]);
	printf("%.2f, %.2f, %.2f, %.2f\n", mat4[0][3], mat4[1][3], mat4[2][3], mat4[3][3]);
	printf("---------------------------------\n");
}

my_vec2 randomVec2() {
	return normalize(my_vec2((float)(rand() - (RAND_MAX / 2)), (float)(rand() - (RAND_MAX / 2))));
}

// ----- trig -----
float radians(float degrees) {
	return degrees * PI / 180;
}

//// punting on these two, using glm functions for now
//// might implement this later: http://www.mathonweb.com/help_ebook/html/algorithms.htm
//float sin(float radians) {
//	return sinf(radians);
//}
//
//float cos(float radians) {
//	return cosf(radians);
//}