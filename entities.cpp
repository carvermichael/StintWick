#include "entities.h"

// Bullet
void Bullet::init(my_vec3 offset, my_vec2 dirVec, Model *newModel, float newSpeed) {
	current = true;

	direction = dirVec;
	model = newModel;
	updateWorldOffset(offset.x, offset.y);

	speed = newSpeed;
}

void Bullet::draw() {
	model->draw(worldOffset, 1.0f, outlineFactor);
}

void Bullet::update(float deltaTime, bool *collided, my_vec2 *collisionWorldOffset) {
	my_vec2 moveAdjust = my_vec2(direction.x * speed * deltaTime, direction.y * speed * deltaTime);

	*collided = false;
	my_vec2 finalOffset = adjustForWallCollisions(bounds, moveAdjust.x, moveAdjust.y, collided);
	
	if (!*collided) {
		updateWorldOffset(finalOffset.x, finalOffset.y);
	}
	else {
		*collisionWorldOffset = finalOffset;
	}
}

void Bullet::updateWorldOffset(float x, float y) {
	prevWorldOffset = worldOffset;
	prevBounds = bounds;
	
	worldOffset.x = x;
	worldOffset.y = y;

	bounds.AX = x;
	bounds.AY = y;

	bounds.BX = x + model->scaleFactor.x;
	bounds.BY = y - model->scaleFactor.y;
}

// RANDOM HELPER FOR ENTITY
Light* getAvailableLight(Light lights[]) {
	for (int i = 0; i < MAX_LIGHTS; i++) {
		if (!lights[i].current) {
			return &lights[i];
		}
	}

	printf("Lights array full! Ah!\n");
	return 0;
}


// Entity
void Entity::draw() {
	float outlineFactor = 0.0f;

	model->draw(worldOffset, 1.0f, outlineFactor);
}

void Entity::draw(Material *mat) {
	model->draw(worldOffset, 1.0f, 0.0f, mat);
}

void Entity::draw(float outlineFactor, Material *mat) {
	model->draw(worldOffset, 1.0f, outlineFactor, mat);
}

void Entity::updateWorldOffset(float x, float y) {
	worldOffset.x = x;
	worldOffset.y = y;

	bounds = AABB(my_vec2(x, y));

	/*bounds.AX = x;
	bounds.AY = y;

	bounds.BX = x + model->scaleFactor.x;
	bounds.BY = y - model->scaleFactor.y;*/
}

void Entity::updateWorldOffset(my_vec2 worldOffset) {
	updateWorldOffset(worldOffset.x, worldOffset.y);
}

// Entity::Player
void Player::init(my_vec3 offset, Model *newModel) {
	model = newModel;

	// bump up to see outline better
	worldOffset.z = offset.z + 0.05f;
	updateWorldOffset(offset.x, offset.y);

	timeBetweenShots = 0.3f;

	speed = 15.0f;
	shotSpeed = 45.0f;
}

// Entity::Enemy
void Enemy::init(my_vec3 offset, Model *newModel, Material *newMat, EnemyStrat *newStrat) {
	model = newModel;
	mat = newMat;

	// bump up to see outline better
	worldOffset.z = offset.z + 0.05f;
	updateWorldOffset(offset.x, offset.y);
	timeBetweenShots = 1.8f;
	speed = 10.0f;
	shotSpeed = 35.0f;

	strat = newStrat;

	current = true;

	timeSinceLastShot = (float)(rand() % 50) / 100.0f;
}

void Enemy::update(Player *player, FloorTile floor[], int numFloors, float deltaTime) {
	strat->update(this, player, floor, numFloors, deltaTime);
}

void Enemy::draw() {
	model->draw(worldOffset, 1.0f, 0.0f, mat);
}

// ENEMY STRATS

// EnemyStrat::Follow
void Follow::update(Entity *entity, Player *player, FloorTile floor[], int numFloors, float deltaTime) {
	static int frame = 0;
	
	// find closest tile, light it up
	int x = (int)entity->worldOffset.x;
	int y = (int)entity->worldOffset.y;
	
	int enemyMatchIndex = -1;
	for (int i = 0; i < numFloors; i++)
	{
		if (floor[i].location.x == x && floor[i].location.y == y) 
		{
			enemyMatchIndex = i;
			break;
		}
	}

	if (enemyMatchIndex == -1)
	{
		printf("Couldn't find floor near enemy.\n");
	}
	else 
	{
		floor[enemyMatchIndex].visited = true;
	}

	// get player tile index

	int playerX = (int) player->worldOffset.x;
	int playerY = (int) player->worldOffset.y;

	int playerMatchIndex = -1;

	for (int i = 0; i < numFloors; i++)
	{
		if (floor[i].location.x == playerX && floor[i].location.y == playerY)
		{
			playerMatchIndex = i;
			break;
		}
	}

	if (playerMatchIndex == -1)
	{
		printf("Couldn't find floor near player.\n");
	}
	else
	{
		floor[playerMatchIndex].visited = true;
	}

	int enemyDestinationFloorIndex = -1;
	if (playerMatchIndex != -1 && enemyMatchIndex != -1) {


		// pre-process adjacency lists
		int adjLists[MAX_FLOORS][4];

		for (int i = 0; i < numFloors; i++) {

			int currX = floor[i].location.x;
			int currY = floor[i].location.y;

			adjLists[i][0] = -1;
			adjLists[i][1] = -1;
			adjLists[i][2] = -1;
			adjLists[i][3] = -1;

			for (int j = 0; j < numFloors; j++) {

				int refX = floor[j].location.x;
				int refY = floor[j].location.y;

				if (currX == refX && currY == refY - 1) {
					adjLists[i][0] = j;
				}

				if (currX == refX && currY == refY + 1) {
					adjLists[i][1] = j;
				}

				if (currX == refX - 1 && currY == refY) {
					adjLists[i][2] = j;
				}

				if (currX == refX + 1 && currY == refY) {
					adjLists[i][3] = j;
				}
			}
		}

		// create edgeTo array	
		int edgeTo[MAX_FLOORS];

		bool marked[MAX_FLOORS] = { false };

		int frontier[MAX_FLOORS];
		frontier[0] = playerMatchIndex;
		int lastFrontierIndex = 1;
		int currentFrontierIndex = 0;
		int numLeft = 1;

		// start at index of player (playerMatchIndex)
		// add playerMatchIndex to frontier

		while (true) {
			int currentIndex = frontier[currentFrontierIndex++];
			
			if (!marked[currentIndex]) {
				marked[currentIndex] = true;

				int adj0 = adjLists[currentIndex][0];
				int adj1 = adjLists[currentIndex][1];
				int adj2 = adjLists[currentIndex][2];
				int adj3 = adjLists[currentIndex][3];

				if (adj0 != -1 && !marked[adj0]) {
					edgeTo[adj0] = currentIndex;
					frontier[lastFrontierIndex++] = adj0;
					numLeft++;

					if (adj0 == enemyMatchIndex) break;
				}

				if (adj1 != -1 && !marked[adj1]) {
					edgeTo[adj1] = currentIndex;
					frontier[lastFrontierIndex++] = adj1;
					numLeft++;

					if (adj1 == enemyMatchIndex) break;
				}

				if (adj2 != -1 && !marked[adj2]) {
					edgeTo[adj2] = currentIndex;
					frontier[lastFrontierIndex++] = adj2;
					numLeft++;

					if (adj2 == enemyMatchIndex) break;
				}

				if (adj3 != -1 && !marked[adj3]) {
					edgeTo[adj3] = currentIndex;
					frontier[lastFrontierIndex++] = adj3;
					numLeft++;

					if (adj3 == enemyMatchIndex) break;
				}
			}

			numLeft--;
			if (numLeft == 0) break;
		}

		// traverse edgeToArray
		int currIndex = enemyMatchIndex;
		while (currIndex != playerMatchIndex) {
			floor[currIndex].onPath = true;
			currIndex = edgeTo[currIndex];
		}

		/*
			frame++;
			if (frame > 200000) frame = 0;
			if (frame % 5 == 0) 
			{
				enemyDestinationFloorIndex = edgeTo[enemyMatchIndex];
			}
		*/

		int candDest = enemyMatchIndex;
		for (int i = 0; i < 3; i++) {
			candDest = edgeTo[candDest];
			if (candDest > 0) enemyDestinationFloorIndex = candDest;
			else break;
		}


		//enemyDestinationFloorIndex = edgeTo[edgeTo[edgeTo[enemyMatchIndex]]];
	}

	// float distFromPlayer = length(player->worldOffset - entity->worldOffset);
	// if (distFromPlayer > 15.0f) return;

	if (enemyDestinationFloorIndex != -1) {
		FloorTile targetFloorTile = floor[enemyDestinationFloorIndex];
		my_vec3 destination;
		if (enemyDestinationFloorIndex == playerMatchIndex) 
		{
			destination = player->worldOffset;
		}
		else 
		{
			destination = my_vec3(targetFloorTile.location.x + 0.5, targetFloorTile.location.y - 0.5, 0.0);
		}
		
		my_vec3 dirVec = normalize(destination - entity->worldOffset);

		my_vec3 moveAdjust = dirVec * entity->speed * deltaTime;

		bool collided;
		my_vec2 finalOffset = adjustForWallCollisions(entity->bounds, moveAdjust.x, moveAdjust.y, &collided);

		entity->updateWorldOffset(finalOffset);
	}

	/*
		// naive follow player logic
		float distFromPlayer = length(player->worldOffset - entity->worldOffset);
		if (distFromPlayer > 15.0f) return;

		my_vec3 dirVec = normalize(player->worldOffset - entity->worldOffset);

		my_vec3 moveAdjust = dirVec * entity->speed * deltaTime;

		my_vec3 newWorldOffset = entity->worldOffset + (dirVec * entity->speed * deltaTime);

		bool collided;
		my_vec2 finalOffset = adjustForWallCollisions(entity->bounds, moveAdjust.x, moveAdjust.y, &collided);
		//my_vec2 finalOffset = my_vec2(entity->worldOffset.x + moveAdjust.x, entity->worldOffset.y + moveAdjust.y);

		entity->updateWorldOffset(finalOffset);
	*/
}

// EnemyStrat::Shoot
void Shoot::update(Entity *entity, Player *player, FloorTile floor[], int numFloors, float deltaTime) {
	float distFromPlayer = length(player->worldOffset - entity->worldOffset);
	if (distFromPlayer > 25.0f) return;

	my_vec3 dirVec = normalize(player->worldOffset - entity->worldOffset);

	entity->timeSinceLastShot += deltaTime;

	if (entity->timeSinceLastShot >= entity->timeBetweenShots) {

		createBullet(entity->worldOffset, dirVec, entity->shotSpeed);

		entity->timeSinceLastShot = 0.0f;
	}
}

// Particle Emitter
void ParticleEmitter::init(my_vec3 newPos, Model *newModel, Light inLights[]) {
	maxT = 0.85f;
	height = 3.5f;
	speed = 15.0f;

	pos = newPos;
	t = 0.0f;
	current = true;

	for (int i = 0; i < 8; i++) {
		positions[i] = newPos;
	}

	for (int i = 0; i < 8; i++) {
		directions[i] = randomVec2();
	}

	for (int i = 0; i < 8; i++) {
		lights[i] = getAvailableLight(inLights);

		lights[i]->current = true;

		lights[i]->pos = newPos;

		lights[i]->ambient = my_vec3(0.2f);
		lights[i]->diffuse = my_vec3(0.2f);
		lights[i]->specular = my_vec3(0.2f);
	}

	model = newModel;
}

void ParticleEmitter::draw() {
	for (int i = 0; i < 8; i++) {
		model->draw(positions[i], 1.0f, 0.75f);
	}
}

void ParticleEmitter::update(float deltaTime) {
	t += deltaTime;
	if (t > maxT) {
		current = false;
		for (int i = 0; i < 8; i++) {
			lights[i]->current = false;
		}
		return;
	}

	float preZ = mapToNewRange(t, 0.0f, maxT, 0.0f, PI);
	float sinPreZ = sin(preZ);
	float newZ = sinPreZ * height;

	for (int i = 0; i < 8; i++) {
		positions[i].x += directions[i].x * speed * deltaTime;
		positions[i].y += directions[i].y * speed * deltaTime;
		positions[i].z = newZ;
		
		lights[i]->pos = positions[i];
	}
}