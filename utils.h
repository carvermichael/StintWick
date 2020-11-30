#pragma once

#include "constants.h"
#include "math.h"

my_vec3 gridCoordsToWorldOffset(my_ivec3 gridCoords) {
	my_vec3 worldOffset;

	worldOffset.x = (float)gridCoords.x;
	worldOffset.y = (float)-gridCoords.y;
	worldOffset.z = (float)gridCoords.z;

	return worldOffset;
}

my_ivec3 worldOffsetToGridCoords(my_vec3 worldOffset) {

	my_ivec3 gridCoords;

	gridCoords.x = (int)(worldOffset.x);
	gridCoords.y = (int)(-worldOffset.y);
	gridCoords.z = (int)(worldOffset.z);

	return gridCoords;
}
