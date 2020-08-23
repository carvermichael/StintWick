#if !defined(ENTITIES)

struct AABB {
    float AX;
    float AY;
    float BX;
    float BY;
};

struct Bullet {

    bool current = false;

    glm::vec2 direction;

    float speed;

    Model *model;

	glm::vec3 worldOffset;
    AABB bounds;

    void init(glm::vec3 offset, glm::vec2 dirVec, Model *newModel) {
        current = true;

        worldOffset = offset;
        direction = dirVec;
        model = newModel;

        speed = 0.28f;
    }

	void draw() {
		model->draw(worldOffset);
	}

    void updateWorldOffset(float x, float y) {
        worldOffset.x = x;
        worldOffset.y = y;
        
        bounds.AX = x;
        bounds.AY = y;

        //@HARDCODE
        bounds.BX = x + 0.5f;
        bounds.BY = y - 0.5f;
    }
};

struct Light {

	glm::vec3 pos = glm::vec3(0.0f);;

	// TODO: maybe make a material???
	glm::vec3 ambient = glm::vec3(1.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);

	float currentDegrees = 0;
};

struct Entity {
	Model *model;

	glm::vec3 worldOffset;
    AABB bounds;

	float speed = 0.2f;

	void draw() {
		model->draw(worldOffset);
	}

    void updateWorldOffset(float x, float y) {
        worldOffset.x = x;
        worldOffset.y = y;
        
        bounds.AX = x;
        bounds.AY = y;

        bounds.BX = x + 0.5f;
        bounds.BY = y - 0.5f;
    }
};

struct Player : Entity {

    float timeSinceLastShot = 0.0f; // seconds
    float timeBetweenShots = 0.3f; // seconds

};

struct Enemy : Entity {

    bool current = false;
	int actionState;

    void init(glm::vec3 offset, Model *newModel) {
        worldOffset.z = offset.z;
        updateWorldOffset(offset.x, offset.y);

        model = newModel;

        current = true; 
    }
};

#define ENTITIES
#endif
