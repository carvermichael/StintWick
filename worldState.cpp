#include "worldState.h"
#include <glfw3.h>
#include "randomUtils.h"
#include "levels.h"

void WorldState::init(Models *inModels, Textbox *inEventTextBox, EnemyStrats *inEnemyStrats, Materials *inMaterials) {
	this->models = inModels;
	this->eventTextBox = inEventTextBox;
	this->enemyStrats = inEnemyStrats;
	this->materials = inMaterials;
}

void WorldState::resetToLevel(Level *level) {
	srand(250);

	currentInputIndex = 0;

	// LIGHTS
	for (int i = 1; i < MAX_LIGHTS; i++) {
		lights[i].current = false;
	}

	lights[0].current = true;
	lights[0].pos = my_vec3(-2.0f, -5.0f, 4.0f);
	lights[0].ambient = my_vec3(1.0f, 1.0f, 1.0f);
	lights[0].diffuse = my_vec3(1.0f, 1.0f, 1.0f);
	lights[0].specular = my_vec3(1.0f, 1.0f, 1.0f);

	// PLAYER
	player.init(gridCoordsToWorldOffset(my_ivec3(level->playerStartX, level->playerStartY, 0)), &models->player);

	// ENEMIES
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

	// "PARTICLES"
	for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
		particleEmitters[i].current = false;
	}

	// BULLETS
	for (int i = 0; i < MAX_BULLETS; i++) {
		playerBullets[i].current = false;
		enemyBullets[i].current = false;
	}

	// WALLS
	numWalls = level->numWalls;

	for (unsigned int i = 0; i < numWalls; i++) {
		wallLocations[i] = level->wallLocations[i];
	}

	// TEXTBOX
	eventTextBox->clearTextBox();
}

void WorldState::pause() {
	mode = MODE_PAUSED;
}

void WorldState::resume() {
	mode = MODE_PLAY;
}

void WorldState::replay() {
	mode = MODE_REPLAY;
}

unsigned int WorldState::getCurrentMode() {
	return mode;
}

my_vec3 WorldState::getPlayerWorldOffset() {
	return player.worldOffset;
}

void WorldState::movePlayer(float x, float y) {
	x *= player.speed;
	y *= player.speed;

	AABB playerBounds = AABB(my_vec2(player.worldOffset.x, player.worldOffset.y));

	bool collided;
	my_vec2 finalOffset = adjustForWallCollisions(playerBounds, x, y, &collided);

	player.updateWorldOffset(finalOffset);
}

void WorldState::playerShoot(float x, float y, float deltaTime) {
	if (x == 0.0f && y == 0.0f) return;

	player.timeSinceLastShot += deltaTime;
	if (player.timeSinceLastShot < player.timeBetweenShots) return;

	bool foundBullet = false;
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!playerBullets[i].current) {
			playerBullets[i].init(player.worldOffset,
				normalize(my_vec2(x, y)),
				&models->bullet, player.shotSpeed);
			foundBullet = true;
			break;
		}
	}

	if (!foundBullet) printf("Bullet array full! Ah!\n");
	else player.timeSinceLastShot = 0.0f;
}

void WorldState::createParticleEmitter(my_vec3 newPos) {

	for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
		if (!particleEmitters[i].current) {
			particleEmitters[i].init(newPos, &models->bulletPart, lights);
			return;
		}
	}

	printf("ParticleEmitter array full! Ah!\n");
}

void WorldState::createBullet(my_vec3 worldOffset, my_vec3 dirVec, float speed) {
	bool foundBullet = false;
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!enemyBullets[i].current) {
			enemyBullets[i].init(worldOffset,
				my_vec2(dirVec.x, dirVec.y),
				&models->enemyBullet, speed);
			foundBullet = true;
			break;
		}
	}

	if (!foundBullet) printf("Bullet array full! Ah!\n");
}

void WorldState::updateBullets(float deltaTime) {
	bool collision;
	my_vec2 collisionWorldOffset;

	for (int i = 0; i < MAX_BULLETS; i++) {
		if (playerBullets[i].current) {
			playerBullets[i].update(deltaTime, &collision, &collisionWorldOffset);

			if (collision) {
				createParticleEmitter(my_vec3(collisionWorldOffset.x, collisionWorldOffset.y, 1.5f));
				playerBullets[i].current = false;
			}
		}

		if (enemyBullets[i].current) {
			enemyBullets[i].update(deltaTime, &collision, &collisionWorldOffset);

			if (collision) {
				createParticleEmitter(my_vec3(collisionWorldOffset.x, collisionWorldOffset.y, 1.5f));
				enemyBullets[i].current = false;
			}
		}
	}
}

void WorldState::addEnemyToWorld(int type, my_ivec2 gridCoords) {
	if (numEnemies >= MAX_ENEMIES) {
		printf("ERROR: Max enemies reached.\n");
		eventTextBox->addTextToBox("ERROR: Max enemies reached.");
		return;
	}

	EnemyStrat *strat = &enemyStrats->follow;
	if (type == 0) {
		strat = &enemyStrats->shoot;
	}
	else if (type == 1) {
		strat = &enemyStrats->follow;
	}

	Material *mat = &materials->mats[type];

	enemies[numEnemies].init(gridCoordsToWorldOffset(my_ivec3(gridCoords.x, gridCoords.y, 0)), &models->enemy, mat, strat);
	numEnemies++;
}

void WorldState::addWallToWorld(my_ivec2 gridCoords) {
	wallLocations[numWalls++] = gridCoords;
}

void WorldState::removeEntityAtOffset(my_vec3 worldOffset) {

	// remove target enemy from world
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i].current) {
			if (worldOffset.x == enemies[i].worldOffset.x &&
				worldOffset.y == enemies[i].worldOffset.y) {
				enemies[i].current = false;
			}
		}
	}
}

void WorldState::updateEnemies(float deltaTime) {
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i].current) enemies[i].update(&player, deltaTime);
	}
}

void WorldState::checkBulletsForEnemyCollisions() {

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
			
			break;
		}
	}
}

void WorldState::checkPlayerForEnemyCollisions() {
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

void WorldState::updateParticleEmitters(float deltaTime) {
	for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
		if (particleEmitters[i].current) {
			particleEmitters[i].update(deltaTime);
		}
	}
}

void WorldState::update(InputRecord inputRecord) {

	GLFWgamepadstate gamepadStateToUse = inputRecord.gamepadState;
	float deltaTime = inputRecord.deltaTime;
	float deltaTimeForUpdate;

	if (gamepadStateToUse.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS
		&& prevGamepadState.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_RELEASE) {
		if (mode == MODE_PLAY) mode = MODE_PAUSED;
		else if (mode == MODE_PAUSED) mode = MODE_PLAY;
		else if (mode == MODE_REPLAY) {
			goForwardOneLevel();
			loadCurrentLevel();
			mode = MODE_PAUSED;
		}
	}

	prevGamepadState = inputRecord.gamepadState;

	if (mode == MODE_REPLAY) {
		gamepadStateToUse = recordedInput[currentInputIndex].gamepadState;
		deltaTimeForUpdate = recordedInput[currentInputIndex].deltaTime;

		currentInputIndex++;
	}
	else if (mode == MODE_PLAY) {
		recordedInput[currentInputIndex].gamepadState = gamepadStateToUse;
		recordedInput[currentInputIndex].deltaTime = deltaTime;
		currentInputIndex++;
		deltaTimeForUpdate = deltaTime;
	}
	else if (mode == MODE_PAUSED) {
		deltaTimeForUpdate = 0.0f;
	}

	if (numEnemies <= 0 && (mode == MODE_PLAY || mode == MODE_REPLAY)) {
		loadCurrentLevel();
		currentInputIndex = 0;
		mode = MODE_REPLAY;
	}

	if (mode != MODE_PAUSED) {
		moveWithController(gamepadStateToUse, deltaTime);

		updateBullets(deltaTimeForUpdate);
		checkBulletsForEnemyCollisions();
		checkPlayerForEnemyCollisions();
		checkPlayerForEnemyBulletCollisions();
		updateEnemies(deltaTimeForUpdate);
		updateParticleEmitters(deltaTimeForUpdate);
	}
}

// NOTE: The check for GLFW_RELEASE relies on Windows repeat logic,
//		 probably don't want to rely on that long-term.		
//									-carver (8-10-20)
void WorldState::moveWithController(GLFWgamepadstate state, float deltaTime) {
	static GLFWgamepadstate prevState;

	// movement
	float leftX = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
	float leftY = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

	if (glm::abs(leftX) < 0.1f) leftX = 0;
	if (glm::abs(leftY) < 0.1f) leftY = 0;

	leftX *= deltaTime;
	leftY *= deltaTime;

	// Up on Y joystick is negative. Flipping here to make it easier to work with in relation to world space.
	movePlayer(leftX, leftY * -1.0f);

	// movement
	float rightX = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
	float rightY = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];

	if (glm::abs(rightX) < 0.1f) rightX = 0;
	if (glm::abs(rightY) < 0.1f) rightY = 0;

	// Up on Y joystick is negative. Flipping here to make it easier to work with in relation to world space.
	playerShoot(rightX, rightY * -1.0f, deltaTime);

	prevState = state;
}

void WorldState::draw() {
	player.draw();

	drawGrid();
	drawBullets();
	drawEnemies();
	drawParticleEmitters();
}

void WorldState::drawGrid() {
	for (unsigned int i = 0; i < numWalls; i++) {
		models->wall.draw(my_vec3((float)wallLocations[i].x, (float)wallLocations[i].y, 0.0f), 1.0f, 0.0f);
	}
}

void WorldState::drawBullets() {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (playerBullets[i].current) playerBullets[i].draw();
	}

	for (int i = 0; i < MAX_BULLETS; i++) {
		if (enemyBullets[i].current) enemyBullets[i].draw();
	}
}

void WorldState::drawEnemies() {
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i].current) enemies[i].draw();
	}
}

void WorldState::drawParticleEmitters() {
	for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
		if (particleEmitters[i].current) {
			particleEmitters[i].draw();
		}
	}
}

void WorldState::checkPlayerForEnemyBulletCollisions() {

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