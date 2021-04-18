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

void Enemy::update(Player *player, Floor *floor, float deltaTime) {
	strat->update(this, player, floor, deltaTime);
}

void Enemy::draw() {
	model->draw(worldOffset, 1.0f, 0.0f, mat);
}

int getClosestTile(Floor *floor, my_vec3 p)
{
	int matchedIndex = -1;
	int x = (int)p.x;
	int y = (int)p.y;

	for (int i = 0; i < floor->size; i++)
	{
		if (floor->tiles[i].location.x == x && floor->tiles[i].location.y == y) 
		{
			matchedIndex = i;
			break;
		}
	}

	return matchedIndex;
}

// ENEMY STRATS

// EnemyStrat::Follow
void Follow::update(Entity *entity, Player *player, Floor *floor, float deltaTime) {
	static int frame = 0;
	if (deltaTime < 0.00001) return;
	
	// find closest tile, light it up
	int enemyMatchIndex = getClosestTile(floor, entity->worldOffset);
	if (enemyMatchIndex == -1)
	{
		printf("Couldn't find floor near enemy.\n");
	}
	else 
	{
		floor->tiles[enemyMatchIndex].visited = true;
	}

	// get player tile index

	int playerX = (int) player->worldOffset.x;
	int playerY = (int) player->worldOffset.y;

	int playerMatchIndex = getClosestTile(floor, player->worldOffset);
	if (playerMatchIndex == -1)
	{
		printf("Couldn't find floor near player.\n");
	}
	else
	{
		floor->tiles[playerMatchIndex].visited = true;
	}

	if (enemyMatchIndex == -1 || playerMatchIndex == -1) return;

	int edgeTo[MAX_FLOORS];
	int distTo[MAX_FLOORS];
	bool visited[MAX_FLOORS];

	for (int i = 0; i < MAX_FLOORS; i++)
	{
		edgeTo[i] = -1;
		distTo[i] = 9999;
		visited[i] = false;
	}

	edgeTo[enemyMatchIndex] = enemyMatchIndex;
	distTo[enemyMatchIndex] = 0;

	PriorityQueue queue;
	queue.init();

	// add tiles adjacent to enemyMatchIndex to the PriorityQueue
	for (int i = 0; i < 4; i++)
	{
		int adj = floor->adjLists[enemyMatchIndex][i];

		if (adj != -1)
		{
			int weight = floor->tiles[adj].weight;
			queue.push(adj, weight);
			visited[adj] = true;
		}
	}
	
	// while there's something in the queue
	while (!queue.isEmpty())
	{
		// pop next thing off the queue
		int currIndex;
		int currWeight;

		queue.pop(&currIndex, &currWeight);
		
		int smallestDistTo = 1000000;
		int smallestIndex = -1;
		
		// find smallest distTo among adjacents
		for (int i = 0; i < 4; i++)
		{
			int adj = floor->adjLists[currIndex][i];

			if (adj != -1 && distTo[adj] < smallestDistTo)
			{
				smallestDistTo = distTo[adj];
				smallestIndex = adj;

			}

			// add all adjacents that haven't been visited yet to PQueue
			if (!visited[adj])
			{
				queue.push(adj, floor->tiles[adj].weight);
				visited[adj] = true;
			}
		}

		// attach current to node with smallest distTo
		if (smallestIndex != -1)
		{
			edgeTo[currIndex] = smallestIndex;
			distTo[currIndex] = smallestDistTo + currWeight;
		}
	}

	// When you're here, the edgeTo graph should have the shortest weighted path to any given destination.
	// Just need to walk back from goal point to origin.

	int currIndex = playerMatchIndex;
	// assumption: enemyMatchIndex and playerMatchIndex are on the same graph
	while (currIndex != enemyMatchIndex)
	{
		currIndex = edgeTo[currIndex];
		floor->tiles[currIndex].visited = true;
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