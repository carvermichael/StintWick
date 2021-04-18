#if !defined(ENTITIES)

#include "math.h"
#include "global_manip.h"

// TODO: not ideal place for these floor structs...shrug
struct FloorTile {
	my_ivec2 location;
	int up;
	int down;
	int left;
	int right;
	bool visited;
	bool onPath;
};

struct Floor
{
	unsigned int size;
	FloorTile tiles[MAX_FLOORS];	
	int adjLists[MAX_FLOORS][4];
};

struct Bullet {

    bool current = false;

    my_vec2 direction;

    float speed;

    Model *model;
	float outlineFactor = 1.0f;

	my_vec3 worldOffset;
    AABB bounds;

	my_vec3 prevWorldOffset;
	AABB prevBounds;

	void init(my_vec3 offset, my_vec2 dirVec, Model *newModel, float newSpeed);
	void draw();
	void update(float deltaTime, bool *collided, my_vec2 *collisionWorldOffset);
	void updateWorldOffset(float x, float y);
};

struct Light {
	bool current = false;

	my_vec3 pos = my_vec3(0.0f);;

	my_vec3 ambient = my_vec3(1.0f);
	my_vec3 diffuse = my_vec3(1.0f);
	my_vec3 specular = my_vec3(1.0f);	

	float currentDegrees = 0; // TODO: this should be elsewhere
};

Light* getAvailableLight(Light lights[]);

struct Entity {
	Model *model;

	my_vec3 worldOffset;
    AABB bounds;

	float timeSinceLastShot = 0.0f; // seconds
	float timeBetweenShots = 0.25f; // seconds

	float speed = 10.0f;
	float shotSpeed = 45.0f;

	void draw();
	void draw(Material *mat);
	void draw(float outlineFactor, Material *mat);
	void updateWorldOffset(float x, float y);
	void updateWorldOffset(my_vec2 worldOffset);
};

struct Player : Entity {

	void init(my_vec3 offset, Model *newModel);
};

struct EnemyStrat {
	virtual void update(Entity *entity, Player *player, Floor *floor, float deltaTime) {};
};

struct Follow : EnemyStrat {
	void update(Entity *entity, Player *player, Floor *floor, float deltaTime);
};

struct Shoot : EnemyStrat {

	Shoot() : EnemyStrat() {};

	void update(Entity *entity, Player *player, FloorTile floor[], int numFloors, float deltaTime);
};

struct Enemy : Entity {

    bool current = false;
	EnemyStrat *strat;
	Material *mat;

	void init(my_vec3 offset, Model *newModel, Material *newMat, EnemyStrat *newStrat);
	void update(Player *player, Floor *floor, float deltaTime);
	void draw();
};

#define NUM_ENEMY_TYPES 2
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

	void init(my_vec3 newPos, Model *newModel, Light inLights[]);
	void draw();
	void update(float deltaTime);
};

#define ENTITIES
#endif
