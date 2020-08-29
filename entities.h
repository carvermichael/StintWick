#if !defined(ENTITIES)

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

    void init(glm::vec3 offset, glm::vec2 dirVec, Model *newModel) {
        current = true;

        worldOffset = offset;
        direction = dirVec;
        model = newModel;

        speed = 25.00f;
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
	}
};

struct EnemyStrat {
	virtual void update(Entity *entity, Player *player, float deltaTime) {};
};

struct Enemy : Entity {

    bool current = false;
	int actionState;
	EnemyStrat *strat;	

	void init(glm::vec3 offset, Model *newModel, EnemyStrat *newStrat) {
		model = newModel;

		// bump up to see outline better
		worldOffset.z = offset.z + 0.05f;
        updateWorldOffset(offset.x, offset.y);

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
