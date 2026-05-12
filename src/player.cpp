#include "player.hpp"
#include "camera.hpp"
#include "debug.hpp"
#include "gameConfig.hpp"
#include "item.hpp"
#include "map.hpp"
#include "network.hpp"
#include "physics.hpp"
#include "rayUtils.hpp"
#include "vector.hpp"
#include "rayUtils.hpp"
#include <cmath>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <string>

Player player;
std::vector<BlockUpdatePacket> blockUpdates;
std::vector<InventoryMovePacket> inventoryMoves;

Vector3 getPlayerTopLeft() {
        return {player.pos.x - 0.5f, player.pos.y + 1.0f, player.pos.z - 0.5f};
}

void Player::updateMovement() {
        const float playerRunningSpeed = 0.18;
        const float playerSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? playerRunningSpeed : 0.12;

        const float GRAVITY = 0.05f;
        const float JUMP_VELOCITY = 0.25f;

        // ground check
        bool onGround = map.getBlock(toVec3Int(pos)) != AIR;

        // jump
        if (onGround && IsKeyPressed(KEY_SPACE)) {
                velocity.y = JUMP_VELOCITY;
        }

        // horizontal movement
        Vector3 vel = {0, 0, 0};
        vel.x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
        vel.z = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);

        Vector3 horizTarget = {vel.x, 0, vel.z};
        float horizLen = sqrtf(horizTarget.x * horizTarget.x +
                                horizTarget.z * horizTarget.z);
        if (horizLen > 0) {
                horizTarget.x /= horizLen;
                horizTarget.z /= horizLen;
                horizTarget.x *= playerSpeed;
                horizTarget.z *= playerSpeed;
        }

        // lerp horizontal
        velocity.x = Lerp(velocity.x, horizTarget.x, PLAYER_ACCELERATION_SPEED);
        velocity.z = Lerp(velocity.z, horizTarget.z, PLAYER_ACCELERATION_SPEED);

        // apply gravity
        if (!onGround) velocity.y -= GRAVITY;
        

        debug.addMessage("velocity: " + vector3ToString(velocity));

        // colision detection
        pos.x += velocity.x;
        pos.x = physicsReaction(pos, velocity.x, 0);

        pos.z += velocity.z;
        pos.z = physicsReaction(pos, velocity.z, 2);

        pos.y += velocity.y;
        pos.y = physicsReaction(pos, velocity.y, 1);
}

void Player::updateBlockPlacingBreaking() {
        if (inventoryOpen) return;
        // select hotbar slot
        for (int key = KEY_ONE; key <= KEY_NINE; key++) {
        	if (IsKeyPressed(key)) {
        		selectedSlot = key - KEY_ONE;
        		selectedBlock = (Block)inventory[selectedSlot];
        	}
        }

        // mouse wheel to cycle hotbar slots
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
                selectedSlot -= (wheel > 0 ? 1 : -1);
                if (selectedSlot < 0) selectedSlot = 9;
                if (selectedSlot > 9) selectedSlot = 0;
                selectedBlock = (Block)inventory[selectedSlot];
        }

        Vector2 mouseScreen = GetMousePosition();
        Vector2 m = GetScreenToWorld2D(mouseScreen, playerCamera.camera);
        Vector2 worldOffset = getWorldRenderOffset();
        m.x += worldOffset.x;
        m.y += worldOffset.y;
        int x = std::floor(m.x / (float)BLOCK_SIZE);
        int z = std::floor(m.y / (float)BLOCK_SIZE);

        auto topBlock = map.findTopBlock(x, z);

        if (topBlock.has_value()) {
                const Vector3 blockPreviewPos = {(float)x * BLOCK_SIZE,
                        (float)(topBlock->y + 1) * BLOCK_SIZE,
                        (float)z * BLOCK_SIZE};
                const Color blockPreviewTint = ColorAlpha(DARKGRAY, 0.4f);
                queueDraw3D(blockPreviewPos.y, [blockPreviewPos, blockPreviewTint]() {
                                DrawRectangle((int)blockPreviewPos.x, (int)blockPreviewPos.z, BLOCK_SIZE, BLOCK_SIZE, blockPreviewTint); 
                                });
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        	if (inventory[selectedSlot] != AIR) {
        		Block placing = (Block)inventory[selectedSlot];
        		inventory[selectedSlot] = AIR;
      
        		if (topBlock.has_value()) {
        			map.setBlock({x, topBlock->y + 1, z}, placing);
        			blockUpdates.push_back({{x, topBlock->y + 1, z}, placing});
        		} else {
        			map.setBlock({x, 0, z}, placing);
        			blockUpdates.push_back({{x, 0, z}, placing});
        		}
        	}
        }

        // timed block breaking
        const float BlockBreakTime = debug.enabled ? 0.3 : 1.00; // in sec

        Vec3Int currentTarget = topBlock.has_value()
                ? Vec3Int{x, topBlock->y, z}
                : Vec3Int{x, 0, z};

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                if (blockBreakingPos == currentTarget) {
                        blockBreakingProgress += FIXED_FRAME_TIME.count() / BlockBreakTime;
                        if (blockBreakingProgress >= 1.0f) {
                                map.setBlock(blockBreakingPos, AIR);
                                blockUpdates.push_back({blockBreakingPos, AIR});
                                blockBreakingProgress = 0.0f;
                        }
                } else {
                        blockBreakingProgress = 0.0f;
                        blockBreakingPos = currentTarget;
                }
        } else {
                blockBreakingProgress = 0.0f;
                blockBreakingPos = currentTarget;
        }

        // Draw breaking overlay texture
        if (blockBreakingProgress > 0.0f && IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                const Vector3 breakBlockWorldPos = blockBreakingPos.toVec3() * (float)BLOCK_SIZE;
                const Texture2D breakTex = useTexture("block_breaking.png");
                const float alpha = blockBreakingProgress;
                queueDraw3D(breakBlockWorldPos.y + 0.01,
                        [breakTex, breakBlockWorldPos, alpha]() {
                                DrawTextureWithRot(breakTex, breakBlockWorldPos.x, breakBlockWorldPos.z, 0, ColorAlpha(WHITE, alpha), 1.0f);
                        });
        }

        debug.addMessage(
                        "cursor pos: %R x: " + std::to_string(x) +
                        " %G Y: " + (topBlock.has_value() ? std::to_string(topBlock->y) : "N/A") +
                        " %B Z: " + std::to_string(z));
}

const float PLAYER_ACCELERATION_SPEED = 0.3;
void Player::update() {
        // zoom
        static float scroll;
        scroll += GetMouseWheelMove() * 0.1f;
        if (IsKeyDown(KEY_C)) {
                if (scroll < 0.1f) {
                        scroll = 0.1f;
                }
                playerCamera.camera.zoom = scroll;
        } else {
                playerCamera.camera.zoom = 1.0f;
                scroll = 1;
        }

        updateMovement();
        updateBlockPlacingBreaking();
        // Draw player
        Vector3 playerCenter = pos;
        playerCenter.x -= 0.5;
        playerCenter.y += 0.01;
        playerCenter.z -= 0.5;
        const Vector3 playerCenterWorld = playerCenter * BLOCK_SIZE;
        const Texture2D playerTexture = useTexture("player.png");
        queueDraw3D(playerCenterWorld.y, [playerTexture, playerCenterWorld]() {
                        DrawTextureWithRot(playerTexture, playerCenterWorld.x, playerCenterWorld.z, 0, WHITE, 1.0f);
                        });

	// draw held block
	const Texture2D selectedBlockTex = useTexture(getEnumName((Block)inventory[selectedSlot]) + ".png");
        queueDraw3D(playerCenterWorld.y + 0.1, [selectedBlockTex, playerCenterWorld]() {
                        DrawTextureWithRot(selectedBlockTex, playerCenterWorld.x, playerCenterWorld.z, 0, WHITE, 0.3f);
                        });

        debug.addMessage("Player pos: " + vector3ToString(pos));
}
