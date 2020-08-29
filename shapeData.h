#if !defined(SHAPE_DATA)

float cubeVertices[] = {
	// top
	// 1, 2, 3, 4
	0.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f,

	// bottom
	// 5, 6, 7, 8
	0.0f, 1.0f, 0.0f,	0.0f, 0.0f, -1.0f,
	1.0f, 1.0f, 0.0f,	0.0f, 0.0f, -1.0f,
	1.0f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,

	// left
	// 1, 4, 5, 8
	0.0f, 1.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,	-1.0f, 0.0f, 0.0f,

	// right
	// 2, 3, 6, 7
	1.0f, 1.0f, 1.0f,	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 1.0f,	1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,

	// front
	// 4, 3, 8, 7
	0.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,

	// back		
	// 1, 2, 5, 6
	0.0f, 1.0f, 1.0f,	0.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 1.0f,	0.0f, -1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
};
unsigned int cubeIndices[] = {
	// top
	0, 1, 3,
	1, 2, 3,

	// bottom
	4, 5, 7,
	5, 6, 7,

	// left
	8, 9, 10,
	9, 10, 11,

	// right
	12, 13, 14,
	13, 14, 15,

	// front
	16, 17, 18,
	17, 18, 19,

	// back
	20, 21, 22,
	21, 22, 23
};

unsigned int cubeOutlineIndices[] = {
	0, 4, 0, 1, 5, 1,
	2, 6, 2, 3, 7, 3,
	0, 4, 5, 6, 7, 4
};

#define SHAPE_DATA
#endif
