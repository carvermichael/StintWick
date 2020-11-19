#if !defined(PLAYER_ACTION)

inline void movePlayer(float x, float y) {
    x *= world->player.speed;
    y *= world->player.speed;

    float finalOffsetX = world->player.worldOffset.x;
    float finalOffsetY = world->player.worldOffset.y;
    
	my_ivec3 playerGridPos = worldOffsetToGridCoords(world->player.worldOffset); // will be used for narrowing candidate wall collisions
	AABB playerBounds = AABB(my_vec2(world->player.worldOffset.x, world->player.worldOffset.y));

	int playerAXFloor = (int)playerBounds.AX;
	int playerBXFloor = (int)playerBounds.BX;
	int playerAYFloor = (int)playerBounds.AY;
	int playerBYFloor = (int)playerBounds.BY;

	// TODO: pull this out, will need it for enemy+wall collision detection

	if (x < 0) {
		if (finalOffsetX + x < playerAXFloor &&						  // checks to see if player will hit an integer boundary (walls only occur on integer boundries)
			(world->grid[playerAXFloor - 1][-playerAYFloor] == WALL || // effectively checks to see if there's a wall where top left will hit or where bottom left will hit
			 world->grid[playerAXFloor - 1][-playerBYFloor] == WALL)) {
				finalOffsetX = (float)playerAXFloor;
		}
		else {
			finalOffsetX += x;
		}		
	}
	if (x > 0) {
		if ((int)(playerBounds.BX + x) > playerBXFloor &&					// checks to see if player will hit an integer boundary (walls only occur on integer boundries)
			(world->grid[playerBXFloor + 1][-playerAYFloor] == WALL ||		// effectively checks to see if there's a wall where top left will hit or where bottom left will hit
			 world->grid[playerBXFloor + 1][-playerBYFloor] == WALL)) {
			finalOffsetX = (float)(playerBXFloor - 0.01f);					// hacky solution: flooring the BX bounds doesn't work when BX is exactly on the integer line
		}
		else {
			finalOffsetX += x;
		}
	}

	if (y < 0) {
		if ((int)(playerBounds.BY + y) <= playerBYFloor - 1 &&						   // checks to see if player will hit an integer boundary (walls only occur on integer boundries)
			(world->grid[playerBXFloor][-playerBYFloor + 1] == WALL ||		   // effectively checks to see if there's a wall where top left will hit or where bottom left will hit
			 world->grid[playerAXFloor][-playerBYFloor + 1] == WALL)) {
			finalOffsetY = (float)playerBYFloor + 0.001f;
		}
		else {
			finalOffsetY += y;
		}
	}

	if (y > 0) {
		if ((int)(playerBounds.BY + y) > playerBYFloor &&					// checks to see if player will hit an integer boundary (walls only occur on integer boundries)
			(world->grid[playerBXFloor][-playerAYFloor - 1] == WALL ||		// effectively checks to see if there's a wall where top left will hit or where bottom left will hit
				world->grid[playerAXFloor][-playerAYFloor - 1] == WALL)) {
			finalOffsetY = (float)playerAYFloor - 0.001f;					// hacky solution: flooring the BX bounds doesn't work when BX is exactly on the integer line
		}
		else {
			finalOffsetY += y;
		}
	}

    world->player.updateWorldOffset(finalOffsetX, finalOffsetY);
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
