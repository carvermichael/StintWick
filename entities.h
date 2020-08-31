#if !defined(ENTITIES)

#include "math.h"

struct AABB {

    union {
        struct {
            float AX;
            float AY;
            float BX;
            float BY;
        };

        struct {
            float left;
            float top;
            float right;
            float bottom;
        };
    };
};

struct Bullet {

    bool current = false;

    glm::vec2 direction;

    float speed;

    Model *model;
	float outlineFactor = 1.0f;

	glm::vec3 worldOffset;
    AABB bounds;

    void init(glm::vec3 offset, glm::vec2 dirVec, Model *newModel, float newSpeed) {
        current = true;

        worldOffset = offset;
        direction = dirVec;
        model = newModel;

        speed = newSpeed;
    }

	void draw() {
		model->draw(worldOffset, outlineFactor);
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
	glm::vec3 pos;

	Model *model;
	glm::vec2 directions[8];
	glm::vec3 positions[8];

	void init(glm::vec3 newPos, Model *newModel) {
		pos = newPos;
		t = 0.0f;
		current = true;

		for (int i = 0; i < 8; i++) {
			positions[i] = newPos;
		}

		float angle = 0.0f;
		for (int i = 0; i < 8; i++) {
			directions[i] = glm::normalize(glm::vec2(rand() - (RAND_MAX / 2), rand() - (RAND_MAX / 2)));
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

	glm::vec3 pos = glm::vec3(0.0f);;

	glm::vec3 ambient = glm::vec3(1.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);

	float currentDegrees = 0;
};

struct Entity {
	Model *model;

	glm::vec3 worldOffset;
    AABB bounds;

	float timeSinceLastShot = 0.0f; // seconds
	float timeBetweenShots = 0.25f; // seconds

	float speed = 10.0f;
	float shotSpeed = 45.0f;

	void draw() {
		model->draw(worldOffset);
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

	void init(glm::vec3 offset, Model *newModel) {
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

	void init(glm::vec3 offset, Model *newModel, EnemyStrat *newStrat) {
		model = newModel;

		// bump up to see outline better
		worldOffset.z = offset.z + 0.05f;
        updateWorldOffset(offset.x, offset.y);
		timeBetweenShots = 1.8f;
		speed = 16.0f;
		shotSpeed = 35.0f;

		strat = newStrat;

        current = true;
    }

	// simple following
	void update(Player *player, float deltaTime) {
		strat->update(this, player, deltaTime);
	}
};

#define ENTITIES
#endif
