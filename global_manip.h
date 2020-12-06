#pragma once

#include <iomanip>
#include "constants.h"

//void loadCurrentLevel();
//void createBullet(my_vec3 worldOffset, my_vec3 dirVec, float speed);
//void addEnemyToWorld(int type, my_ivec2 gridCoords);
my_vec2 adjustForWallCollisions(unsigned short grid[MAX_GRID_ONE_DIM][MAX_GRID_ONE_DIM], AABB entityBounds, my_vec2 move, bool *collided);
my_vec2 adjustForWallCollisions(unsigned short grid[MAX_GRID_ONE_DIM][MAX_GRID_ONE_DIM], AABB entityBounds, my_vec2 move, bool *collided, bool isPlayer);

void processConsoleCommand(std::string command);
void goBackOneLevel();
void goForwardOneLevel();
void deleteCurrentLevel();
void goBackOneEnemyType();
void goForwardOneEnemyType();
unsigned int getCurrentLevel();
unsigned int getEnemySelection();
void setPauseCoords();
void hitTheLights();
void addEnemyToLevel(int type, my_ivec2 gridCoords);
my_ivec3 cameraCenterToGridCoords();
void toggleEditorMode();
int getEditorMode();

// TODO: These shouldn't be here --> These will go in the openGL file, when that's created (as part of pulling that out for easier/simpler porting)
void setUniformBool(unsigned int shaderProgramID, const char *uniformName, bool value);
void setUniform1f(unsigned int shaderProgramID, const char *uniformName, float value);
void setUniform3f(unsigned int shaderProgramID, const char *uniformName, my_vec3 my_vec3);
void setUniform4f(unsigned int shaderProgramID, const char *uniformName, my_vec4 my_vec4);
void setUniformMat4(unsigned int shaderProgramID, const char *uniformName, glm::mat4 mat4);
void setUniformMat4(unsigned int shaderProgramID, const char *uniformName, my_mat4 mat4);
