#pragma once

// TODO: set four different entrances/exits &
//		 also 4 different player spawn points

unsigned int level_1[] = {
	// size x, y
	30, 30,

	// player
	// gridX, gridY
	15, 26,
	
	// num enemies
	9,
	// enemies
	// type, gridX, gridY
	1,  8, 4,
	1, 12, 4,
	1, 16, 4,
	1, 20, 4,
	1, 24, 4,
	2, 12, 6,
	2, 16, 6,
	2, 20, 6,
	2, 24, 6
};

unsigned int level_2[] = {
	// size x, y
	10, 30,

	// player
	// gridX, gridY
	5, 25,

	// num enemies
	8,
	// enemies
	// type, gridX, gridY
	1,  2,  4,
	1,  4,  4,
	1,  6,  4,
	1,  8,  4,
	2,  2, 12,
	2,  4, 12,
	2,  6, 12,
	2,  8, 12
};

unsigned int level_3[] = {
	// size x, y
	50, 50,

	// player
	// gridX, gridY
	5, 45,

	// num enemies
	20,
	// enemies
	// type, gridX, gridY
	1,  2,  4,
	1,  4,  4,
	1,  6,  4,
	1,  8,  4,
	2,  2, 12,
	2,  4, 12,
	2,  6, 12,
	2,  8, 12,

	1,  1,  2,
	1,  1,  4,
	1,  1,  6,
	1,  1,  8,
	2,  12, 12,
	2,  14, 12,
	2,  18, 12,
	2,  26, 12,

	1,  35,  16,
	1,  38,  23,
	1,  42,  47,
	1,  45,  49
};