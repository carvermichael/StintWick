#if !defined(RANDOMUTILS)

#include "math.h"
#include <vector>

my_vec3 gridCoordsToWorldOffset(my_ivec3 gridCoords);
my_ivec3 worldOffsetToGridCoords(my_vec3 worldOffset);

std::vector<std::string> splitString(std::string str, char delimiter);

#define RANDOMUTILS
#endif