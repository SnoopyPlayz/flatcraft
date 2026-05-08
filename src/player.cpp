#include "player.hpp"
#include "camera.hpp"
#include "debug.hpp"
#include "item.hpp"
#include "map.hpp"
#include "network.hpp"
#include "physics.hpp"
#include "rayUtils.hpp"
#include "vector.hpp"
#include <cmath>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <string>

Player player;
std::vector<BlockUpdatePacket> blockUpdates;

bool inventoryOpen = false;

Vector3 getPlayerTopLeft() {
        return {player.pos.x - 0.5f, player.pos.y + 1.0f, player.pos.z - 0.5f};
}

int in = 0;
void Player::inventoryUpdate() {
        const float windowBorder = 50;
        const float windowRoundess = 0.1;
        const int rowSize = 10; // amount of elements in a row

        const int rowStart = 100;

        if (IsKeyPressed(KEY_E)) {
                inventory[in++] = (Block)(STONE);
                inventory[in++] = (Block)(GRASS);
                inventory[in++] = (Block)(CRAFTING_TABLE);
                std::cout << "inventory update " + std::to_string(in) << std::endl;
                inventoryOpen = !inventoryOpen;
        }
        if (!inventoryOpen) {
                return;
        }

        DrawRectangleRounded((Rectangle){windowBorder, windowBorder,
                        (float)GetScreenWidth() - windowBorder * 2,
                        (float)GetScreenHeight() - windowBorder * 2},
                        windowRoundess, 0, WHITE);

        int y = 100;
        int x = 1;
        // drawGrid and draw SelectedBlock
        static bool selected = false;
        static Block selectedBlock = AIR;
        static int lastPos;

        for (int i{}; i < PLAYER_INVENTORY_SIZE; i++) {
                float pixelPosX = 1.2 * BLOCK_SIZE * x;

                if (x > rowSize || pixelPosX + BLOCK_SIZE > GetScreenWidth()) {
                        x = 1;
                        y += 100;
                }

                pixelPosX = 1.2 * BLOCK_SIZE * x;

                // item background
                const float itemGridOffset = 5;
                DrawRectangleRounded((Rectangle){pixelPosX - itemGridOffset,
                                (float)y - itemGridOffset,
                                BLOCK_SIZE + itemGridOffset * 2,
                                BLOCK_SIZE + itemGridOffset * 2},
                                windowRoundess, 1, DARKGRAY);

                x++;

                // draw item Tex
                if (inventory[i] != AIR) {
                        DrawTexture(useTexture(getEnumName((Block)inventory[i]) + ".png"),
                                        pixelPosX, y, WHITE);
                }

                // colision with cursor
                if (AABBColBox2d(pixelPosX, y, BLOCK_SIZE, GetMouseX(), GetMouseY(), 1)) {
                        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && inventory[i] != AIR) {
                                selected = true;
                                selectedBlock = (Block)inventory[i];
                                inventory[i] = AIR;
                                lastPos = i;
                        }

                        if (selected) {
                                if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                                        inventory[lastPos] = (Block)inventory[i];
                                        inventory[i] = selectedBlock;
                                        selected = false;
                                        selectedBlock = AIR;
                                }
                        }

                        DrawRectangle(pixelPosX, y, BLOCK_SIZE, BLOCK_SIZE,
                                        ColorAlpha(GRAY, 0.5));
                }

                // draw selectedBlock
                if (selected) {
                        DrawTexture(useTexture(getEnumName(selectedBlock) + ".png"),
                                        GetMouseX() - BLOCK_SIZE * 0.5,
                                        GetMouseY() - BLOCK_SIZE * 0.5, WHITE);
                }
        }
}

void Player::updateUI() { inventoryUpdate(); }

void Player::updateMovement() {
        const float playerRunningSpeed = 0.18;
        const float playerSpeed =
                IsKeyDown(KEY_LEFT_SHIFT) ? playerRunningSpeed : 0.12;

        const float GRAVITY = 0.05f;
        const float JUMP_VELOCITY = 0.25f;

        // Ground check: block directly below player feet
        bool onGround = map.getBlock(toVec3Int(pos)) != AIR;

        // Jump impulse (one-shot, only when on ground)
        if (onGround && IsKeyPressed(KEY_SPACE)) {
                velocity.y = JUMP_VELOCITY;
        }

        // Horizontal movement only (Y handled separately via gravity/jump)
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

        // Lerp horizontal velocity independently from vertical
        velocity.x = Lerp(velocity.x, horizTarget.x, PLAYER_ACCELERATION_SPEED);
        velocity.z = Lerp(velocity.z, horizTarget.z, PLAYER_ACCELERATION_SPEED);

        // Gravity: only apply when not on ground
        if (!onGround) {
                velocity.y -= GRAVITY;
        } else if (velocity.y < 0) {
                velocity.y = 0; // Stop downward velocity when landing
        }

        debug.addMessage("velocity: " + vector3ToString(velocity));

        pos.x += velocity.x;
        pos.x = physicsReaction(pos, velocity.x, 0);

        pos.z += velocity.z;
        pos.z = physicsReaction(pos, velocity.z, 2);

        pos.y += velocity.y;
        pos.y = physicsReaction(pos, velocity.y, 1);
}

void Player::updateBlockPlacingBreaking() {
        // Select block
        for (int key = KEY_ZERO; key <= KEY_NINE; key++) {
                if (IsKeyPressed(key)) {
                        selectedBlock = static_cast<Block>(key - KEY_ZERO);
                }
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
                                DrawRectangle((int)blockPreviewPos.x, (int)blockPreviewPos.z, BLOCK_SIZE,
                                                BLOCK_SIZE, blockPreviewTint);
                                });
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (topBlock.has_value()) {
                        map.setBlock({x, topBlock->y + 1, z}, selectedBlock);
                        blockUpdates.push_back({{x, topBlock->y + 1, z}, selectedBlock});
                } else {
                        map.setBlock({x, 0, z}, selectedBlock);
                        blockUpdates.push_back({{x, 0, z}, selectedBlock});
                }
        }

        // --- Timed block breaking (right mouse button) ---
        constexpr float BLOCK_BREAK_TIME = 1.0f; // seconds to break a block
        constexpr float FRAME_DELTA = 1.0f / 60.0f;

        Vec3Int currentTarget = topBlock.has_value()
                ? Vec3Int{x, topBlock->y, z}
                : Vec3Int{x, 0, z};

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                blockBreakingProgress = 0.0f;
                blockBreakingPos = currentTarget;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                if (blockBreakingPos == currentTarget) {
                        blockBreakingProgress += FRAME_DELTA / BLOCK_BREAK_TIME;
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
        }

        // Draw breaking overlay texture
        if (blockBreakingProgress > 0.0f && IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                const Vector3 breakBlockWorldPos = {
                        (float)blockBreakingPos.x * BLOCK_SIZE,
                        (float)blockBreakingPos.y * BLOCK_SIZE,
                        (float)blockBreakingPos.z * BLOCK_SIZE
                };
                const Texture2D breakTex = useTexture("block_breaking.png");
                const float alpha = blockBreakingProgress;
                queueDraw3D(breakBlockWorldPos.y + 0.01,
                        [breakTex, breakBlockWorldPos, alpha]() {
                                DrawTextureWithRot(breakTex,
                                        breakBlockWorldPos.x,
                                        breakBlockWorldPos.z,
                                        0, ColorAlpha(WHITE, alpha), 1.0f);
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
        pickUpItems();
        // Draw player
        Vector3 playerCenter = pos;
        playerCenter.x -= 0.5;
        playerCenter.y += 0.01;
        playerCenter.z -= 0.5;
        const Vector3 playerCenterWorld = playerCenter * BLOCK_SIZE;
        const Texture2D playerTexture = useTexture("player.png");
        queueDraw3D(playerCenterWorld.y, [playerTexture, playerCenterWorld]() {
                        DrawTextureWithRot(playerTexture, playerCenterWorld.x, playerCenterWorld.z,
                                        0, WHITE, 1.0f);
                        });

        debug.addMessage("Player pos: " + vector3ToString(pos));
}
