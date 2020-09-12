#if !defined(PLAYER_ACTION)

void movePlayer(float x, float y) {
    x *= world.player.speed;
    y *= world.player.speed;

    float finalOffsetX = world.player.worldOffset.x;
    float finalOffsetY = world.player.worldOffset.y;
    
    // X
    float playerBoundX;
    float worldBoundX;

    if(x > 0) {
        playerBoundX = world.player.bounds.BX + x;
        worldBoundX = world.wallBounds.BX - 1.0f;

        if(playerBoundX > worldBoundX) {
            finalOffsetX = worldBoundX;
        } else {
            
            finalOffsetX = world.player.worldOffset.x + x;
        }
    }
    if(x < 0)  {
        playerBoundX = world.player.bounds.AX + x;
        worldBoundX = world.wallBounds.AX;

        if(playerBoundX < worldBoundX) {
            finalOffsetX = worldBoundX;
        } else {
            finalOffsetX = world.player.worldOffset.x + x;
        }
    }

    // Y
    float playerBoundY;
    float worldBoundY;

    if(y > 0) {
        playerBoundY = world.player.bounds.AY + y;
        worldBoundY = world.wallBounds.AY - 1.0f;

        if(playerBoundY > worldBoundY) {
            finalOffsetY = worldBoundY;
        } else {
            finalOffsetY = world.player.worldOffset.y + y;
        }
    }
    if(y < 0)  {
        playerBoundY = world.player.bounds.BY + y;
        worldBoundY = world.wallBounds.BY;

        if(playerBoundY < worldBoundY) {
            finalOffsetY = worldBoundY;
        } else {
            finalOffsetY = world.player.worldOffset.y + y;
        }
    }

    world.player.updateWorldOffset(finalOffsetX, finalOffsetY);
}

void playerShoot(float x, float y, float deltaTime) {
    if(x == 0.0f && y == 0.0f) return;

    world.player.timeSinceLastShot += deltaTime;
    if(world.player.timeSinceLastShot < world.player.timeBetweenShots) return;

    bool foundBullet = false;
    for(int i = 0; i < MAX_BULLETS; i++) {
        if(!world.playerBullets[i].current) {
            world.playerBullets[i].init(world.player.worldOffset, 
                                  normalize(my_vec2(x, y)),
                                  &models.bullet, world.player.shotSpeed);
            foundBullet = true;
            break;
        }
    }

    if(!foundBullet) printf("Bullet array full! Ah!\n");
    else world.player.timeSinceLastShot = 0.0f;
}

#define PLAYER_ACTION
#endif
