#if !defined(PLAYER_ACTION)

// lotta @HARDCODE
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
        worldBoundX = world.wallBounds.BX;

        if(playerBoundX > worldBoundX) {
            finalOffsetX = worldBoundX - 0.5f;
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
        worldBoundY = world.wallBounds.AY;

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
            finalOffsetY = worldBoundY + 0.5f;
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

    Bullet bullet;
    bullet.direction = glm::normalize(glm::vec2(x, y));
    bullet.speed = 1.0f;

    bullet.model = &models.bullet;
    
    bullet.worldOffset = world.player.worldOffset;

    world.bullets.push_back(bullet);

    world.player.timeSinceLastShot = 0.0f;
}

#define PLAYER_ACTION
#endif
