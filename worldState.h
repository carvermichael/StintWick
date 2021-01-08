#pragma once

#include "constants.h"
#include "camera.h"
#include "model.h"
#include "entities.h"
#include "levels.h"
#include <glfw3.h>

struct InputRecord {
	GLFWgamepadstate gamepadState;
	float deltaTime;
};

struct WorldState {

	Light lights[MAX_LIGHTS];
	
	void init(Models *models, Textbox *inEventTextBox, EnemyStrats *inEnemyStrats, Materials *inMaterials);
	void resetToLevel(Level *level);
	void update(InputRecord inputRecord);
	void draw();
	
	void pause();
	void resume();
	void replay();
	unsigned int getCurrentMode();

	my_vec3 getPlayerWorldOffset();
	
	// USED BY EDITOR
	void addEnemyToWorld(int type, my_ivec2 gridCoords);
	void addWallToWorld(my_ivec2 gridCoords);

	void removeEntityAtOffset(my_vec3 worldOffset);

	unsigned int numWalls;
	my_ivec2 wallLocations[MAX_WALLS];

	unsigned int mode;
	unsigned int seed;

	Textbox *eventTextBox;

	Models *models;
	EnemyStrats *enemyStrats;
	Materials *materials;

	GLFWgamepadstate prevGamepadState;
	InputRecord recordedInput[INPUT_MAX];
	int currentInputIndex = 0;
	
	Player player;

	int numEnemies = 0;
    Enemy enemies[MAX_ENEMIES];
    
    Bullet playerBullets[MAX_BULLETS];
	Bullet enemyBullets[MAX_BULLETS];

	ParticleEmitter particleEmitters[MAX_PARTICLE_EMITTERS];

	//my_vec2 adjustForWallCollisions(AABB entityBounds, float moveX, float moveY, bool *collided);
	void movePlayer(float x, float y);
	void playerShoot(float x, float y, float deltaTime);
	void createParticleEmitter(my_vec3 newPos);
	void createBullet(my_vec3 worldOffset, my_vec3 dirVec, float speed);
	void updateBullets(float deltaTime);
	void updateEnemies(float deltaTime);
	void checkBulletsForEnemyCollisions();
	void checkPlayerForEnemyCollisions();
	void updateParticleEmitters(float deltaTime);
	void moveWithController(GLFWgamepadstate state, float deltaTime);
	void drawGrid();
	void drawBullets();
	void drawEnemies();
	void drawParticleEmitters();
	void checkPlayerForEnemyBulletCollisions();
};

//#define WORLDSTATE 0
//#endif
