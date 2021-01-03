#include "randomUtils.h"

my_vec3 gridCoordsToWorldOffset(my_ivec3 gridCoords) {
	my_vec3 worldOffset;

	worldOffset.x = (float)gridCoords.x;
	worldOffset.y = (float)gridCoords.y;
	worldOffset.z = (float)gridCoords.z;

	return worldOffset;
}

my_ivec3 worldOffsetToGridCoords(my_vec3 worldOffset) {

	my_ivec3 gridCoords;

	gridCoords.x = (int)(worldOffset.x);
	gridCoords.y = (int)(worldOffset.y);
	gridCoords.z = (int)(worldOffset.z);

	return gridCoords;
}

std::vector<std::string> splitString(std::string str, char delimiter) {
	std::vector<std::string> returnStrings;

	int count = 0;
	for (int i = 0; i < str.size(); i++) {
		if (str[i] == delimiter) {
			returnStrings.push_back(std::string(str, i - count, count));
			count = 0;
		}
		else {
			count++;
		}
	}

	returnStrings.push_back(std::string(str, str.size() - count, count));

	return returnStrings;
}