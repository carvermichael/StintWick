#if !defined(ENTITIES)

#include "math.h"
#include "global_manip.h"

struct Bullet {

    bool current = false;

    my_vec2 direction;

    float speed;

    Model *model;
	float outlineFactor = 1.0f;

	my_vec3 worldOffset;
    AABB bounds;

    void init(my_vec3 offset, my_vec2 dirVec, Model *newModel, float newSpeed) {
        current = true;
				
        direction = dirVec;
        model = newModel;
		updateWorldOffset(offset.x, offset.y);

        speed = newSpeed;
    }

	void draw() {
		model->draw(worldOffset, 1.0f, outlineFactor);
	}

	void update(float deltaTime) {
		my_vec2 moveAdjust = my_vec2(direction.x * speed * deltaTime, direction.y * speed * deltaTime);

		bool collided;
		my_vec2 finalOffset = adjustForWallCollisions(bounds, moveAdjust.x, moveAdjust.y, &collided);

		if (collided) {
			current = false;
			createParticleEmitter(my_vec3(finalOffset.x, finalOffset.y, 1.5f));
		}
		else {
			updateWorldOffset(finalOffset.x, finalOffset.y);
		}
	}

    void updateWorldOffset(float x, float y) {
        worldOffset.x = x;
        worldOffset.y = y;
        
        bounds.AX = x;
        bounds.AY = y;

        bounds.BX = x + model->scaleFactor.x;
        bounds.BY = y - model->scaleFactor.y;
    }
};

struct Light {
	bool current = false;

	my_vec3 pos = my_vec3(0.0f);;

	my_vec3 ambient = my_vec3(1.0f);
	my_vec3 diffuse = my_vec3(1.0f);
	my_vec3 specular = my_vec3(1.0f);

	float currentDegrees = 0; // TODO: this shouldn't be here
};

Light* getAvailableLight(Light lights[]) {
	for (int i = 0; i < MAX_LIGHTS; i++) {
		if (!lights[i].current) {
			return &lights[i];
		}
	}

	printf("Lights array full! Ah!\n");
	return 0;
}

struct Entity {
	Model *model;

	my_vec3 worldOffset;
    AABB bounds;

	float timeSinceLastShot = 0.0f; // seconds
	float timeBetweenShots = 0.25f; // seconds

	bool blinking = false;
	bool blinkOn = false;
	float blinkTime = 0.35f;
	float lastBlinkTime = 0.0f;

	float speed = 10.0f;
	float shotSpeed = 45.0f;

	void draw() {
		float outlineFactor = 0.0f;
		
		//if (blinking) {
		//	lastBlinkTime += deltaTime;
		//	if (lastBlinkTime >= blinkTime) {
		//		blinkOn = !blinkOn;
		//		lastBlinkTime = 0.0f;
		//	}
		//	if (blinkOn) {
		//		outlineFactor = 1.0f;
		//	}
		//}

		model->draw(worldOffset, 1.0f, outlineFactor);
	}

	void draw(Material *mat) {
		model->draw(worldOffset, 1.0f, 0.0f, mat);
	}

	void draw(float outlineFactor, Material *mat) {
		model->draw(worldOffset, 1.0f, outlineFactor, mat);
	}

    void updateWorldOffset(float x, float y) {
        worldOffset.x = x;
        worldOffset.y = y;
        
        bounds.AX = x;
        bounds.AY = y;

		bounds.BX = x + model->scaleFactor.x;
		bounds.BY = y - model->scaleFactor.y;
    }

	void updateWorldOffset(my_vec2 worldOffset) {
		updateWorldOffset(worldOffset.x, worldOffset.y);
	}
};

struct Player : Entity {

	void init(my_vec3 offset, Model *newModel) {
		model = newModel;

		// bump up to see outline better
		worldOffset.z = offset.z + 0.05f;
		updateWorldOffset(offset.x, offset.y);

		timeBetweenShots = 0.3f;

		speed = 15.0f;
		shotSpeed = 45.0f;
	}
};

struct EnemyStrat {
	virtual void update(Entity *entity, Player *player, float deltaTime) {};
};

struct Enemy : Entity {

    bool current = false;
	EnemyStrat *strat;
	Material *mat;

	void init(my_vec3 offset, Model *newModel, Material *newMat, EnemyStrat *newStrat) {
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

	void update(Player *player, float deltaTime) {
		strat->update(this, player, deltaTime);
	}

	void draw() {
		model->draw(worldOffset, 1.0f, 0.0f, mat);
	}
};

struct Follow : EnemyStrat {
	void update(Entity *entity, Player *player, float deltaTime) {
		float distFromPlayer = length(player->worldOffset - entity->worldOffset);
		if (distFromPlayer > 15.0f) return;
		
		my_vec3 dirVec = normalize(player->worldOffset - entity->worldOffset);
		
		my_vec3 moveAdjust = dirVec * entity->speed * deltaTime;
		
		//my_vec3 newWorldOffset = entity->worldOffset + (dirVec * entity->speed * deltaTime);
		
		bool collided;
		my_vec2 finalOffset = adjustForWallCollisions(entity->bounds, moveAdjust.x, moveAdjust.y, &collided);

		entity->updateWorldOffset(finalOffset);
	}
};

struct Shoot : EnemyStrat {

	Shoot() : EnemyStrat() {};

	void update(Entity *entity, Player *player, float deltaTime) {
		float distFromPlayer = length(player->worldOffset - entity->worldOffset);
		if (distFromPlayer > 25.0f) return;
		
		my_vec3 dirVec = normalize(player->worldOffset - entity->worldOffset);

		entity->timeSinceLastShot += deltaTime;

		if (entity->timeSinceLastShot >= entity->timeBetweenShots) {

			createBullet(entity->worldOffset, dirVec, entity->shotSpeed);

			entity->timeSinceLastShot = 0.0f;
		}
	}
};

#define NUM_ENEMY_STRATS 2
struct EnemyStrats {
	Shoot shoot;
	Follow follow;
};

// TODO: actually create indpendent particles
struct ParticleEmitter {

	bool current = false;

	float t;
	float maxT;
	float height;

	float speed;
	my_vec3 pos;

	Model *model;
	my_vec2 directions[8];
	my_vec3 positions[8];

	Light* lights[8];

	void init(my_vec3 newPos, Model *newModel, Light inLights[]) {
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
			// get light reference
			lights[i] = getAvailableLight(inLights);

			// set light current to true
			lights[i]->current = true;

			// set light pos
			lights[i]->pos = newPos;

			// set light mat variables
			lights[i]->ambient	= my_vec3(0.2f);
			lights[i]->diffuse	= my_vec3(0.2f);
			lights[i]->specular = my_vec3(0.2f);
		}

		model = newModel;
	}

	void draw() {
		for (int i = 0; i < 8; i++) {
			model->draw(positions[i], 1.0f, 0.75f);
		}
	}

	void update(float deltaTime) {
		t += deltaTime;
		if (t > maxT) {
			current = false;
			// set all lights currents to false
			for (int i = 0; i < 8; i++) {
				lights[i]->current = false;
			}
			return;
		}

		float preZ = mapToNewRange(t, 0.0f, maxT, 0.0f, PI);
		float sinPreZ = sin(preZ);
		float newZ = sinPreZ * height;
		//printf("t: %.2f, preZ: %.2f, newZ: %.2f\n", t, preZ, newZ);

		for (int i = 0; i < 8; i++) {
			positions[i].x += directions[i].x * speed * deltaTime;
			positions[i].y += directions[i].y * speed * deltaTime;
			positions[i].z = newZ;
			// update light pos
			lights[i]->pos = positions[i];
		}
	}
};

#define ENTITIES
#endif
