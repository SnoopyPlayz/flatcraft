#include "map.hpp"
#include "player.hpp"
#include "vector.hpp"
#include <cmath>
#include <raylib.h>
#include <raymath.h>

bool AABBColBox2d(int x, int y, int size, int xb, int yb, int sizeb) {
        return (x < xb + sizeb && x + size > xb && y < yb + sizeb && y + size > yb);
}

bool AABBColBox3d(Vector3 pos, Vector3 size, Vector3 otherPos,
                Vector3 otherSize) {
        return (pos.x < otherPos.x + otherSize.x && pos.x + size.x > otherPos.x &&
                        pos.y < otherPos.y + otherSize.y && pos.y + size.y > otherPos.y &&
                        pos.z < otherPos.z + otherSize.z && pos.z + size.z > otherPos.z);
}

static auto &vec3Axis(auto &v, int axis) {
        if (axis == 0) {
                return v.x;
        }
        if (axis == 1) {
                return v.y;
        }
        return v.z;
}

float physicsReaction(Vector3 &pos, float &vel, int axis) {
        float &posAxis = vec3Axis(pos, axis);

        Vector3 playerTopLeft = getPlayerTopLeft();
        const float playerAxis = vec3Axis(playerTopLeft, axis);

        Vec3Int pTopLeft = toVec3Int(playerTopLeft);
        RADUIS(1) {
                Vec3Int blockPos = pTopLeft + (Vec3Int){x, y, z};
                // skip air blocks around player
                if (map.getBlock(blockPos) == AIR) {
                        continue;
                }
                // player collision with block
                if (AABBColBox3d(playerTopLeft, {1, 1, 1}, blockPos.toVec3(), {1, 1, 1})) {
                        Vec3Int posx = toVec3Int(pos);
                        posx.y += 1;
                        const float blockAxis = vec3Axis(blockPos, axis);

                        // if player in block. move more
                        if (map.getBlock(posx) != AIR) {
                                // move player to air block if in full block
                                vec3Axis(posx, axis) += 1;
                                if (map.getBlock(posx) == AIR && playerAxis > blockAxis) {
                                        posAxis += 0.5;
                                }
                                vec3Axis(posx, axis) -= 2;
                                if (map.getBlock(posx) == AIR && playerAxis < blockAxis) {
                                        posAxis -= 0.5;
                                }
                        }

                        // move player to edge of block
                        posAxis -= vel;
                        vel = 0.0;
                        if (axis == 0) {
                                posAxis = std::floor(posAxis) + 0.5;
                        } else if (axis == 1) {
                                posAxis = std::floor(posAxis);
                        } else {
                                posAxis = std::floor(posAxis) + 0.5;
                        }
                }
        }

        return posAxis;
}
