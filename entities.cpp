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
	/*float finalOffsetX = bounds.AX + moveAdjust.x;
	float finalOffsetY = bounds.AY + moveAdjust.y;*/

	//my_vec2 finalOffset = my_vec2(finalOffsetX, finalOffsetY);

	if (!*collided) {
		updateWorldOffset(finalOffset.x, finalOffset.y);
	}
	else {
		*collisionWorldOffset = finalOffset;
	}
}

void Bullet::updateWorldOffset(float x, float y) {
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

void Enemy::update(Player *player, float deltaTime) {
	strat->update(this, player, deltaTime);
}

void Enemy::draw() {
	model->draw(worldOffset, 1.0f, 0.0f, mat);
}

// ENEMY STRATS

// EnemyStrat::Follow
void Follow::update(Entity *entity, Player *player, float deltaTime) {
	float distFromPlayer = length(player->worldOffset - entity->worldOffset);
	if (distFromPlayer > 15.0f) return;

	my_vec3 dirVec = normalize(player->worldOffset - entity->worldOffset);

	my_vec3 moveAdjust = dirVec * entity->speed * deltaTime;

	my_vec3 newWorldOffset = entity->worldOffset + (dirVec * entity->speed * deltaTime);

	bool collided;
	my_vec2 finalOffset = adjustForWallCollisions(entity->bounds, moveAdjust.x, moveAdjust.y, &collided);
	//my_vec2 finalOffset = my_vec2(entity->worldOffset.x + moveAdjust.x, entity->worldOffset.y + moveAdjust.y);

	entity->updateWorldOffset(finalOffset);
}

// EnemyStrat::Shoot
void Shoot::update(Entity *entity, Player *player, float deltaTime) {
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