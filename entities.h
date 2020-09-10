#if !defined(ENTITIES)

#include "math.h"

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

        worldOffset = offset;
        direction = dirVec;
        model = newModel;

        speed = newSpeed;
    }

	void draw() {
		model->draw(worldOffset, 1.0f, outlineFactor);
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

// TODO: actually create indpendent particles
struct ParticleEmitter {

	bool current = false;

	float t;
	float maxT = 0.85f;
	float height = 3.5f;

	float speed = 15.0f;
	my_vec3 pos;

	Model *model;
	my_vec2 directions[8];
	my_vec3 positions[8];

	void init(my_vec3 newPos, Model *newModel) {
		pos = newPos;
		t = 0.0f;
		current = true;

		for (int i = 0; i < 8; i++) {
			positions[i] = newPos;
		}

		float angle = 0.0f;
		for (int i = 0; i < 8; i++) {
			directions[i] = normalize(my_vec2((float) (rand() - (RAND_MAX / 2)), (float) (rand() - (RAND_MAX / 2))));
		}

		model = newModel;
	}

	void draw() {
		for (int i = 0; i < 8; i++) {
			model->draw(positions[i]);
		}
	}

	void update(float deltaTime) {
		t += deltaTime;
		if (t > maxT) current = false;

		float preZ = mapToNewRange(t, 0.0f, maxT, 0.0f, PI);
		float sinPreZ = sin(preZ);
		float newZ = sinPreZ * height;
		//printf("t: %.2f, preZ: %.2f, newZ: %.2f\n", t, preZ, newZ);
		
		for (int i = 0; i < 8; i++) {
			positions[i].x += directions[i].x * speed * deltaTime;
			positions[i].y += directions[i].y * speed * deltaTime;
			positions[i].z = newZ;
		}
	}
};

struct Light {

	my_vec3 pos = my_vec3(0.0f);;

	my_vec3 ambient = my_vec3(1.0f);
	my_vec3 diffuse = my_vec3(1.0f);
	my_vec3 specular = my_vec3(1.0f);

	float currentDegrees = 0;
};

struct Entity {
	Model *model;

	my_vec3 worldOffset;
    AABB bounds;

	float timeSinceLastShot = 0.0f; // seconds
	float timeBetweenShots = 0.25f; // seconds

	float speed = 10.0f;
	float shotSpeed = 45.0f;

	void draw() {
		model->draw(worldOffset);
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
		speed = 16.0f;
		shotSpeed = 35.0f;

		strat = newStrat;

        current = true;

		timeSinceLastShot = (float)(rand() % 50) / 100.0f;
    }

	// simple following
	void update(Player *player, float deltaTime) {
		strat->update(this, player, deltaTime);
	}

	void draw() {
		model->draw(worldOffset, 1.0f, 0.0f, mat);
	}
};

#define ENTITIES
#endif
