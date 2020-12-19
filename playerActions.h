#if !defined(PLAYER_ACTION)

#include "global_manip.h"

inline void movePlayer(float x, float y) {
    x *= world->player.speed;
    y *= world->player.speed;

    AABB playerBounds = AABB(my_vec2(world->player.worldOffset.x, world->player.worldOffset.y));

	bool collided;
	my_vec2 finalOffset = adjustForWallCollisions(playerBounds, x, y, &collided);
	//my_vec2 finalOffset = my_vec2(world->player.worldOffset.x + x, world->player.worldOffset.y + y); // for skipping player-wall collisions

    world->player.updateWorldOffset(finalOffset);
}

inline void playerShoot(float x, float y, float deltaTime) {
    if(x == 0.0f && y == 0.0f) return;

    world->player.timeSinceLastShot += deltaTime;
    if(world->player.timeSinceLastShot < world->player.timeBetweenShots) return;

    bool foundBullet = false;
    for(int i = 0; i < MAX_BULLETS; i++) {
        if(!world->playerBullets[i].current) {
            world->playerBullets[i].init(world->player.worldOffset, 
                                  normalize(my_vec2(x, y)),
                                  &models.bullet, world->player.shotSpeed);
            foundBullet = true;
            break;
        }
    }

    if(!foundBullet) printf("Bullet array full! Ah!\n");
    else world->player.timeSinceLastShot = 0.0f;
}

#define PLAYER_ACTION
#endif
