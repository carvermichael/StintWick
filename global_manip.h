#pragma once

#include <iomanip>
#include "levels.h"
#include "textBox.h"
#include "model.h"


my_vec2 adjustForWallCollisions(AABB entityBounds, float moveX, float moveY, bool *collided);

void processConsoleCommand(std::string command);
void loadCurrentLevel();
void goBackOneLevel();
void goForwardOneLevel();

unsigned int getCurrentLevel();

my_ivec3 cameraCenterToGridCoords();

// Editor-related
void saveAllLevelsV2(Level levels[], unsigned int levelCount);
unsigned int addLevel(Level levels[], unsigned int levelCount);
unsigned int getEnemyTypeSelection();
unsigned int getEditorMode();
void deleteCurrentLevel();
void addEnemyToWorld(unsigned int enemyType, my_ivec2 gridCoords);
void addWallToWorld(my_ivec2 gridCoords);
void addEnemyToCurrentLevel(int type, my_ivec2 gridCoords);
void addWallToCurrentLevel(my_ivec2 location);
void removeEntityFromCurrentLevel(my_ivec2 gridCoords);
void removeEntityFromWorld(my_vec3 worldOffset);
void toggleEditorMode();
void goBackOneEnemyType();
void goForwardOneEnemyType();

void drawText(Font *font, std::string text, float x, float y, float scale, my_vec3 color);

// TODO: These shouldn't be here --> These will go in the openGL file, when that's created (as part of pulling that out for easier/simpler porting)
void setUniformBool(unsigned int shaderProgramID, const char *uniformName, bool value);
void setUniform1f(unsigned int shaderProgramID, const char *uniformName, float value);
void setUniform3f(unsigned int shaderProgramID, const char *uniformName, my_vec3 my_vec3);
void setUniform4f(unsigned int shaderProgramID, const char *uniformName, my_vec4 my_vec4);
void setUniformMat4(unsigned int shaderProgramID, const char *uniformName, glm::mat4 mat4);
void setUniformMat4(unsigned int shaderProgramID, const char *uniformName, my_mat4 mat4);
