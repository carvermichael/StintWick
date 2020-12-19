#if !defined(WORLDSTATE)

#include "constants.h"
#include "camera.h"
#include "model.h"
#include "entities.h"

struct WorldState {
	unsigned int seed;

	// level v1 stuff
	unsigned int gridSizeX;
	unsigned int gridSizeY;
    AABB wallBounds;

	// level v2 stuff
	unsigned int numWalls;
	my_ivec2 wallLocations[MAX_WALLS];
	Model *wallModel;

	Camera camera;

	Light lights[MAX_LIGHTS];
	
	Player player;
	int numEnemies = 0;
    Enemy enemies[MAX_ENEMIES];
    
    Bullet playerBullets[MAX_BULLETS];
	Bullet enemyBullets[MAX_BULLETS];

	ParticleEmitter particleEmitters[MAX_PARTICLE_EMITTERS];

	void init(unsigned int newGridSizeX, unsigned int newGridSizeY, Model *inWallModel) {
		gridSizeX = newGridSizeX;
		gridSizeY = newGridSizeY;

        // @HARDCODE: this is set relative to wall cube sizes
        wallBounds.AX =  1.0f;
        wallBounds.AY = -1.0f;

        wallBounds.BX =  (float)(gridSizeX - 1);
        wallBounds.BY = -(float)(gridSizeY - 1);

		lights[0].current = true;
		lights[0].pos = my_vec3(-2.0f, -5.0f, 4.0f);
		lights[0].ambient = my_vec3(1.0f, 1.0f, 1.0f);
		lights[0].diffuse = my_vec3(1.0f, 1.0f, 1.0f);
		lights[0].specular = my_vec3(1.0f, 1.0f, 1.0f);

		numEnemies = 0;

		this->wallModel = inWallModel;
    }

	void draw() {
		player.draw();

		drawGrid();
		drawBullets();
		drawEnemies();
		drawParticleEmitters();
	}

	void drawGrid() {
		for (unsigned int i = 0; i < numWalls; i++) {
			wallModel->draw(my_vec3((float)wallLocations[i].x, (float)wallLocations[i].y, 0.0f), 1.0f, 0.0f);
		}
	}

	void drawBullets() {
		for (int i = 0; i < MAX_BULLETS; i++) {
			if (playerBullets[i].current) playerBullets[i].draw();
		}

		for (int i = 0; i < MAX_BULLETS; i++) {
			if (enemyBullets[i].current) enemyBullets[i].draw();
		}
	}

	void drawEnemies() {
		for (int i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i].current) enemies[i].draw();
		}
	}

	void drawParticleEmitters() {
		for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
			if (particleEmitters[i].current) {
				particleEmitters[i].draw();
			}
		}
	}

};

#define MAX_ENTITIES 100
#define NUM_TURN_ENTITIES 2

#define WORLDSTATE
#endif
