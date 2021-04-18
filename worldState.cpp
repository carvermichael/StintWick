#include "worldState.h"
#include <glfw3.h>
#include "randomUtils.h"
#include "levels.h"
#include "collision.h"

void WorldState::init(Models *inModels, Textbox *inEventTextBox, EnemyStrats *inEnemyStrats, Materials *inMaterials) {
	this->models = inModels;
	this->eventTextBox = inEventTextBox;
	this->enemyStrats = inEnemyStrats;
	this->materials = inMaterials;
}

// TODO: don't need to completely rebuild adjLists everytime a floor is added
void buildFloorAdjLists(Floor *floor)
{
	// pre-process adjacency lists
	for (int i = 0; i < floor->size; i++) {

		int currX = floor->tiles[i].location.x;
		int currY = floor->tiles[i].location.y;

		floor->adjLists[i][0] = -1;
		floor->adjLists[i][1] = -1;
		floor->adjLists[i][2] = -1;
		floor->adjLists[i][3] = -1;

		for (int j = 0; j < floor->size; j++) {

			int refX = floor->tiles[j].location.x;
			int refY = floor->tiles[j].location.y;

			if (currX == refX && currY == refY - 1) {
				floor->adjLists[i][0] = j;
			}

			if (currX == refX && currY == refY + 1) {
				floor->adjLists[i][1] = j;
			}

			if (currX == refX - 1 && currY == refY) {
				floor->adjLists[i][2] = j;
			}

			if (currX == refX + 1 && currY == refY) {
				floor->adjLists[i][3] = j;
			}
		}
	}
}

void WorldState::resetToLevel(Level *level) {
	srand(250);

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

	// FLOOR
	buildFloorAdjLists(&floor);

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

	my_vec2 dirVec = my_vec2(x, y);

	bool collided;
	my_vec2 finalOffset = adjustForWallCollisions(player.bounds, x, y, &collided);
	

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

void WorldState::addFloorToWorld(my_ivec2 gridCoords) {
	floor.tiles[floor.size++].location = gridCoords;		
	buildFloorAdjLists(&floor);
}

int WorldState::getFloorIndex(my_ivec2 location) {
	for (int i = 0; i < floor.size; i++) {
		if (floor.tiles[i].location.x == location.x &&
			floor.tiles[i].location.y == location.y) {
			return i;
		}
	}

	return -1;
}

int WorldState::getWallIndex(my_ivec2 location) {
	for (int i = 0; i < numWalls; i++) {
		if (wallLocations[i].x == location.x &&
			wallLocations[i].y == location.y) {
			return i;
		}
	}

	return -1;
}

void WorldState::resetFloorGrid() {
	for (int i = 0; i < floor.size; i++) {
		floor.tiles[i].visited = false;
		floor.tiles[i].onPath = false;

		int x = floor.tiles[i].location.x;
		int y = floor.tiles[i].location.y;

		floor.tiles[i].up	= getFloorIndex(my_ivec2(x, y + 1));
		floor.tiles[i].down	= getFloorIndex(my_ivec2(x, y - 1));
		floor.tiles[i].left	= getFloorIndex(my_ivec2(x - 1, y));
		floor.tiles[i].right = getFloorIndex(my_ivec2(x + 1, y));
	}
}

bool isInFrontier(my_ivec2 frontier[MAX_FLOORS], my_ivec2 loc) {
	for (int i = 0; i < MAX_FLOORS; i++) {
		if (frontier[i].x == loc.x &&
			frontier[i].y == loc.y) {
			return true;
		}
	}

	return false;
}

void WorldState::fillFloor(my_ivec2 loc) {
	my_ivec2 frontier[MAX_FLOORS];
	frontier[0] = loc;
	int numLeft = 1;
	int curr = 0;
	int size = 1;

	while (numLeft > 0 && size < MAX_FLOORS && curr < size) {

		my_ivec2 currLoc = frontier[curr];
		if (getFloorIndex(currLoc) == -1) {
			addFloorToWorld(currLoc);
			
			if (getWallIndex(my_ivec2(currLoc.x, currLoc.y + 1)) == -1) {
				if (!isInFrontier(frontier, my_ivec2(currLoc.x, currLoc.y + 1))) {
					frontier[size++] = my_ivec2(currLoc.x, currLoc.y + 1);
					numLeft++;
				}
			}
			if (getWallIndex(my_ivec2(currLoc.x, currLoc.y - 1)) == -1) {
				if (!isInFrontier(frontier, my_ivec2(currLoc.x, currLoc.y - 1))) {
					frontier[size++] = (my_ivec2(currLoc.x, currLoc.y - 1));
					numLeft++;
				}				
			}
			if (getWallIndex(my_ivec2(currLoc.x - 1, currLoc.y)) == -1) {
				if (!isInFrontier(frontier, my_ivec2(currLoc.x - 1, currLoc.y))) {
					frontier[size++] = (my_ivec2(currLoc.x - 1, currLoc.y));
					numLeft++;
				}				
			}
			if (getWallIndex(my_ivec2(currLoc.x + 1, currLoc.y)) == -1) {
				if (!isInFrontier(frontier, my_ivec2(currLoc.x + 1, currLoc.y))) {
					frontier[size++] = (my_ivec2(currLoc.x + 1, currLoc.y));
					numLeft++;
				}
			}
		}

		numLeft--;
		curr++;
	}
}

void WorldState::copyFloorToLevel(Level* level) {
	// TODO: will need to use this after filling a floor (not going to run that on both world and level)

	level->numFloors = floor.size;

	for (int i = 0; i < floor.size; i++) {
		level->floorLocations[i].x = floor.tiles->location.x;
		level->floorLocations[i].y = floor.tiles->location.y;
	}
}

void WorldState::removeEntityAtOffset(my_vec3 worldOffset) {

	// remove target enemy from world
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i].current) {
			if (worldOffset.x == enemies[i].worldOffset.x &&
				worldOffset.y == enemies[i].worldOffset.y) {
				enemies[i].current = false;
				numEnemies--;
			}
		}
	}
}

void WorldState::updateEnemies(float deltaTime) {
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i].current) enemies[i].update(&player, &floor, deltaTime);
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

void WorldState::update(InputRecord currentInput, InputRecord prevInput, InputRecord recordedInput[], int *currentInputIndex) {

	GLFWgamepadstate gamepadStateToUse = currentInput.gamepadState;
	float deltaTime = currentInput.deltaTime;
	float deltaTimeForUpdate;

	if (gamepadStateToUse.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS
		&& prevInput.gamepadState.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_RELEASE) {
		if (mode == MODE_PLAY) mode = MODE_PAUSED;
		else if (mode == MODE_PAUSED) mode = MODE_PLAY;
		else if (mode == MODE_REPLAY) {
			goForwardOneLevel();
			loadCurrentLevel();
			mode = MODE_PAUSED;
		}
	}
	
	if (mode == MODE_REPLAY) {
		// printf("replay: currentInputIndex: %d\n", *currentInputIndex);
		
		gamepadStateToUse = recordedInput[*currentInputIndex].gamepadState;
		deltaTimeForUpdate = recordedInput[*currentInputIndex].deltaTime;

		*currentInputIndex = *currentInputIndex + 1;
	}
	else if (mode == MODE_PLAY) {
		// printf("recording: currentInputIndex: %d\n", *currentInputIndex);

		recordedInput[*currentInputIndex].gamepadState = gamepadStateToUse;
		recordedInput[*currentInputIndex].deltaTime = deltaTime;
		*currentInputIndex = *currentInputIndex + 1;
		deltaTimeForUpdate = deltaTime;
	}
	else if (mode == MODE_PAUSED) {
		deltaTimeForUpdate = 0.0f;
	}

	if (mode != MODE_PAUSED) {
		moveWithController(gamepadStateToUse, deltaTimeForUpdate);

		updateBullets(deltaTimeForUpdate);
		checkBulletsForEnemyCollisions();
		checkPlayerForEnemyCollisions();
		checkPlayerForEnemyBulletCollisions();
		updateEnemies(deltaTimeForUpdate);
		updateParticleEmitters(deltaTimeForUpdate);
	}



	if (mode != MODE_PAUSED) {





	}
}

// NOTE: The check for GLFW_RELEASE relies on Windows repeat logic,
//		 probably don't want to rely on that long-term.		
//									-carver (8-10-20)
void WorldState::moveWithController(GLFWgamepadstate state, float deltaTime) {
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

	for (unsigned int i = 0; i < floor.size; i++) {
		FloorTile currTile = floor.tiles[i];
		if (currTile.visited) 
		{
			models->wall.draw(my_vec3((float)currTile.location.x, (float)currTile.location.y, -1.0f), 1.0f, 0.0f);
			floor.tiles[i].visited = false;
		}
		else if(currTile.onPath)
		{
			models->wallTopModel.draw(my_vec3((float)currTile.location.x, (float)currTile.location.y, -1.0f), 1.0f, 0.0f);
		}
		else
		{
			models->floorModel.draw(my_vec3((float)currTile.location.x, (float)currTile.location.y, -1.0f), 1.0f, 0.0f);
		}

		floor.tiles[i].visited = false;
		floor.tiles[i].onPath = false;
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