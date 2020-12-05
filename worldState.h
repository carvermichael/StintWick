#if !defined(WORLDSTATE)

#include "constants.h"
#include "camera.h"
#include "model.h"
#include "entities.h"
#include "global_manip.h"
#include "playerActions.h"
#include "levels.h"
#include "utils.h"



struct InputRecord {
	GLFWgamepadstate gamepadState;
	float deltaTime;
};

struct WorldState {
	unsigned int seed;
	unsigned int mode;

	my_vec3 globalOffset = my_vec3(0.0f);

	Textbox *eventTextBox;

	Level *level;

	InputRecord recordedInput[MAX_INPUTS];
	int currentInputIndex = 0;
	int numInputs = 0;

	// level v1 stuff
	unsigned int gridSizeX;
	unsigned int gridSizeY;
    AABB wallBounds;

	// level v2 stuff
	unsigned int numWalls;
	my_ivec2 wallLocations[MAX_WALLS];
	
	// level v3 stuff -- not being stupid this time
	unsigned short grid[MAX_GRID_ONE_DIM][MAX_GRID_ONE_DIM];

	Light lights[MAX_LIGHTS];
	
	Player player;
	int numEnemies = 0;
    Enemy enemies[MAX_ENEMIES];
    
    Bullet playerBullets[MAX_BULLETS];
	Bullet enemyBullets[MAX_BULLETS];

	ParticleEmitter particleEmitters[MAX_PARTICLE_EMITTERS];

	void resetToLevel(Level *inLevel, Textbox *inEventTextbox) {
		this->level = inLevel;
		this->eventTextBox = inEventTextbox;
		
		currentInputIndex = 0;

		srand(250);

		lights[0].current = true;
		lights[0].pos = my_vec3(-2.0f, -5.0f, 4.0f);
		lights[0].ambient = my_vec3(1.0f, 1.0f, 1.0f);
		lights[0].diffuse = my_vec3(1.0f, 1.0f, 1.0f);
		lights[0].specular = my_vec3(1.0f, 1.0f, 1.0f);

		player.init(gridCoordsToWorldOffset(my_ivec3(level->playerStartX, level->playerStartY, 1)), &models.player);

		for (int i = 0; i < MAX_ENEMIES; i++) {
			enemies[i].current = false;
		}

		numEnemies = 0;

		unsigned int numOfEnemies = level->numEnemies;
		for (unsigned int i = 0; i < numOfEnemies; i++) {

			unsigned int enemyType = level->enemies[i].enemyType;
			unsigned int gridX = level->enemies[i].gridX;
			unsigned int gridY = level->enemies[i].gridY;

			addEnemyToWorld(enemyType, my_ivec2(gridX, gridY));
		}

		// clear particles
		for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
			particleEmitters[i].current = false;
		}

		for (int i = 0; i < MAX_BULLETS; i++) {
			playerBullets[i].current = false;
			enemyBullets[i].current = false;
		}

		// hit the lights
		// i starts at one to keep global light active
		for (int i = 1; i < MAX_LIGHTS; i++) {
			lights[i].current = false;
		}

		// TODO: need to reset grid to zeroes
		// v3 wall setting
		for (unsigned int i = 0; i < level->numWalls; i++) {
			my_ivec2 currWallLoc = level->wallLocations[i];
			grid[currWallLoc.x][currWallLoc.y] = WALL;
		}
	}

  //  void init(unsigned int newGridSizeX, unsigned int newGridSizeY) {
		//mode = MODE_PAUSED;
		//
		//gridSizeX = newGridSizeX;
		//gridSizeY = newGridSizeY;

  //      // @HARDCODE: this is set relative to wall cube sizes
  //      wallBounds.AX =  1.0f;
  //      wallBounds.AY = -1.0f;

  //      wallBounds.BX =  (float)(gridSizeX - 1);
  //      wallBounds.BY = -(float)(gridSizeY - 1);

		//lights[0].current = true;
		//lights[0].pos = my_vec3(-2.0f, -5.0f, 4.0f);
		//lights[0].ambient = my_vec3(1.0f, 1.0f, 1.0f);
		//lights[0].diffuse = my_vec3(1.0f, 1.0f, 1.0f);
		//lights[0].specular = my_vec3(1.0f, 1.0f, 1.0f);

		//numEnemies = 0;
  //  }

	// UPDATE
	void update(GLFWgamepadstate *gamepadState, float deltaTime) {

		// INPUT STUFF
		GLFWgamepadstate gamepadStateToUse = *gamepadState;

		if (mode == MODE_REPLAY) {
			gamepadStateToUse = recordedInput[currentInputIndex].gamepadState;
			deltaTime = recordedInput[currentInputIndex].deltaTime;

			currentInputIndex++;
		}
		else if (mode == MODE_PLAY) {
			recordedInput[currentInputIndex].gamepadState = gamepadStateToUse;
			recordedInput[currentInputIndex].deltaTime = deltaTime;
			currentInputIndex++;
		}
		else if (mode == MODE_PAUSED) {
			deltaTime = 0.0f;
		}

		processGamepadState(gamepadState, deltaTime);

		// TODO: this fucks things up. probably cause loadCurrentLevel zeroes out this whole struct...I think...
		/*if (numEnemies <= 0 && (mode == MODE_PLAY || mode == MODE_REPLAY)) {
			loadCurrentLevel();
			currentInputIndex = 0;
			mode = MODE_REPLAY;
		}*/

		// STATE UPDATE STUFF
		updateBullets(deltaTime);
		checkBulletsForEnemyCollisions();
		checkPlayerForEnemyCollisions();
		checkPlayerForEnemyBulletCollisions();
		updateEnemies(deltaTime);
		updateParticleEmitters(deltaTime);
	}

	void processGamepadState(GLFWgamepadstate *state, float deltaTime) {
		static GLFWgamepadstate prevState;

		if (state->buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS
			&& prevState.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_RELEASE) {
			if (mode == MODE_PLAY) mode = MODE_PAUSED;
			else if (mode == MODE_PAUSED) mode = MODE_PLAY;
			else if (mode == MODE_REPLAY) {
				goForwardOneLevel();
				loadCurrentLevel();
				mode = MODE_PAUSED;
			}
		}

		// movement
		float leftX = state->axes[GLFW_GAMEPAD_AXIS_LEFT_X];
		float leftY = state->axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

		if (glm::abs(leftX) < 0.1f) leftX = 0;
		if (glm::abs(leftY) < 0.1f) leftY = 0;

		leftX *= deltaTime;
		leftY *= deltaTime;

		// Up on Y joystick is negative. Flipping here to make it easier to work with in relation to world space.
		movePlayer(leftX, leftY * -1.0f);

		// movement
		float rightX = state->axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
		float rightY = state->axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];

		if (glm::abs(rightX) < 0.1f) rightX = 0;
		if (glm::abs(rightY) < 0.1f) rightY = 0;

		// Up on Y joystick is negative. Flipping here to make it easier to work with in relation to world space.
		playerShoot(rightX, rightY * -1.0f, deltaTime);

		prevState = *state;
	}

	void updateBullets(float deltaTime) {
		for (int i = 0; i < MAX_BULLETS; i++) {
			if (playerBullets[i].current) {
				playerBullets[i].update(deltaTime);
			}

			if (enemyBullets[i].current) {
				enemyBullets[i].update(deltaTime);
			}
		}
	}

	void updateEnemies(float deltaTime) {
		for (int i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i].current) enemies[i].update(&player, deltaTime);
		}
	}	

	void checkBulletsForEnemyCollisions() {

		for (int i = 0; i < MAX_BULLETS; i++) {

			if (!playerBullets[i].current) continue;
			Bullet *bullet = &playerBullets[i];

			for (int j = 0; j < MAX_ENEMIES; j++) {

				if (!enemies[j].current) continue;
				Enemy *enemy = &enemies[j];

				if (bullet->bounds.left > enemy->bounds.right)  continue;
				if (bullet->bounds.right < enemy->bounds.left)   continue;
				if (bullet->bounds.top < enemy->bounds.bottom) continue;
				if (bullet->bounds.bottom > enemy->bounds.top)    continue;

				createParticleEmitter(my_vec3(bullet->worldOffset.x,
					bullet->worldOffset.y,
					1.5f));

				enemy->current = false;
				bullet->current = false;
				numEnemies--;
				//camera.shakeScreen(0.075f);

				break;
			}
		}
	}

	void checkPlayerForEnemyCollisions() {
		for (int j = 0; j < MAX_ENEMIES; j++) {

			if (!enemies[j].current) continue;
			Enemy *enemy = &enemies[j];

			if (player.bounds.left > enemy->bounds.right)		continue;
			if (player.bounds.right < enemy->bounds.left)		continue;
			if (player.bounds.top < enemy->bounds.bottom)		continue;
			if (player.bounds.bottom > enemy->bounds.top)		continue;

			mode = MODE_PAUSED;
			eventTextBox->addTextToBox("You Died. Try Again.");
			loadCurrentLevel();

			break;
		}
	}

	void checkPlayerForEnemyBulletCollisions() {

		for (int i = 0; i < MAX_BULLETS; i++) {

			if (!enemyBullets[i].current) continue;
			Bullet *bullet = &enemyBullets[i];

			if (player.bounds.left > bullet->bounds.right)		continue;
			if (player.bounds.right < bullet->bounds.left)		continue;
			if (player.bounds.top < bullet->bounds.bottom)	continue;
			if (player.bounds.bottom > bullet->bounds.top)		continue;

			mode = MODE_PAUSED;
			eventTextBox->addTextToBox("You Died. Try Again.");
			loadCurrentLevel();

			break;
		}
	}

	void updateParticleEmitters(float deltaTime) {
		for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
			if (particleEmitters[i].current) {
				particleEmitters[i].update(deltaTime);
			}
		}
	}

	// Player-Controlled Updates
	void movePlayer(float x, float y) {
		x *= player.speed;
		y *= player.speed;

		AABB playerBounds = AABB(my_vec2(player.worldOffset.x, player.worldOffset.y));

		bool collided;
		my_vec2 finalOffset = adjustForWallCollisions(playerBounds, my_vec2(x, y), &collided, true);

		player.updateWorldOffset(finalOffset);
	}

	void playerShoot(float x, float y, float deltaTime) {
		if (x == 0.0f && y == 0.0f) return;

		player.timeSinceLastShot += deltaTime;
		if (player.timeSinceLastShot < player.timeBetweenShots) return;

		bool foundBullet = false;
		for (int i = 0; i < MAX_BULLETS; i++) {
			if (!playerBullets[i].current) {
				playerBullets[i].init(player.worldOffset,
					normalize(my_vec2(x, y)),
					&models.bullet, player.shotSpeed);
				foundBullet = true;
				break;
			}
		}

		if (!foundBullet) {
			//printf("Bullet array full! Ah!\n");
		}
		else player.timeSinceLastShot = 0.0f;
	}

	// DRAW
	void draw() {
		drawGrid();
		player.draw(globalOffset);
		drawEnemies();
		drawBullets();
		drawParticleEmitters();
	}

	void drawGrid() {
		for (unsigned int i = 0; i < MAX_GRID_ONE_DIM; i++) {
			for (unsigned int j = 0; j < MAX_GRID_ONE_DIM; j++) {
				if (grid[i][j] == WALL) {
					my_vec3 worldOffset = gridCoordsToWorldOffset(my_ivec3(i, j, 1));
					models.wall.draw(worldOffset + globalOffset);
				}
			}
		}
	}

	void drawBullets() {
		for (int i = 0; i < MAX_BULLETS; i++) {
			if (playerBullets[i].current) playerBullets[i].draw(globalOffset);
		}

		for (int i = 0; i < MAX_BULLETS; i++) {
			if (enemyBullets[i].current) enemyBullets[i].draw(globalOffset);
		}
	}

	void drawEnemies() {
		for (int i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i].current) enemies[i].draw(globalOffset);
		}
	}

	void drawParticleEmitters() {
		for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
			if (particleEmitters[i].current) {
				particleEmitters[i].draw(globalOffset);
			}
		}
	}
};

#define MAX_ENTITIES 100
#define NUM_TURN_ENTITIES 2

#define WORLDSTATE
#endif
