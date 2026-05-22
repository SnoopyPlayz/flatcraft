#include "pathfinding.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <unordered_set>

static int getGroundY(Map& map, int x, int z) {
	for (int y = MAX_BLOCK_SEARCH_HEIGHT - 1; y >= 0; y--) {
		if (map.getBlock({x, y, z}) != AIR) {
			return y + 1;
		}
	}
	return 0;
}

struct AStarNode {
	Vec3Int pos;
	float gCost;
	float fCost;

	bool operator>(const AStarNode& other) const {
		return fCost > other.fCost;
	}
};

// Cantor pairing for 2D coords into single hash key
static uint64_t packKey(int x, int z) {
	uint64_t a = (uint64_t)(x >= 0 ? 2 * (int64_t)x : -2 * (int64_t)x - 1);
	uint64_t b = (uint64_t)(z >= 0 ? 2 * (int64_t)z : -2 * (int64_t)z - 1);
	return (a + b) * (a + b + 1) / 2 + b;
}

static bool isWalkable(Map& map, int x, int z) {
	int groundY = getGroundY(map, x, z);
	if (groundY <= 0) return false;
	// air at feet level
	if (map.getBlock({x, groundY, z}) != AIR) return false;
	// solid block under feet
	if (map.getBlock({x, groundY - 1, z}) == AIR) return false;
	return true;
}

static float heightCost(Map& map, int x1, int z1, int x2, int z2) {
	int y1 = getGroundY(map, x1, z1);
	int y2 = getGroundY(map, x2, z2);
	int dy = y2 - y1;
	if (dy > 1) return 999.0f; // cannot climb more than 1 block
	if (dy < -3) return 999.0f; // cannot fall more than 3 blocks
	// block step-up through wall: target cell at current standing height must be air
	if (dy > 0 && map.getBlock({x2, y1 + 1, z2}) != AIR) return 999.0f;
	if (dy == 1) return 2.0f; // climbing costs extra
	return 0.0f;
}

// 8-directional neighbors with corner-cutting check
static std::vector<std::pair<Vec3Int, float>> getNeighbors(Map& map, Vec3Int pos) {
	static const int dx[4] = {1, 0, -1, 0};
	static const int dz[4] = {0, 1, 0, -1};

	std::vector<std::pair<Vec3Int, float>> neighbors;
	for (int i = 0; i < 4; i++) {
		int nx = pos.x + dx[i];
		int nz = pos.z + dz[i];

		if (!isWalkable(map, nx, nz)) continue;

		float hc = heightCost(map, pos.x, pos.z, nx, nz);
		if (hc >= 999.0f) continue;

		float totalCost = 1.0f + hc;
		int ny = getGroundY(map, nx, nz);
		neighbors.push_back({{nx, ny, nz}, totalCost});
	}
	return neighbors;
}

std::vector<Vec3Int> findPath(Map& map, Vec3Int start, Vec3Int goal) {
	std::vector<Vec3Int> path;

	// snap to ground
	start.y = getGroundY(map, start.x, start.z);
	goal.y = getGroundY(map, goal.x, goal.z);

	if (!isWalkable(map, start.x, start.z) || !isWalkable(map, goal.x, goal.z)) {
		return path;
	}

	std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openSet;
	std::unordered_map<uint64_t, float> gScores;
	std::unordered_map<uint64_t, Vec3Int> cameFrom;
	std::unordered_set<uint64_t> closedSet;

	uint64_t startKey = packKey(start.x, start.z);
	uint64_t goalKey = packKey(goal.x, goal.z);

	float h = (float)(std::abs(goal.x - start.x) + std::abs(goal.z - start.z));
	openSet.push({start, 0.0f, h});
	gScores[startKey] = 0.0f;

	constexpr int maxIterations = 2000;
	int iterations = 0;

	while (!openSet.empty() && iterations++ < maxIterations) {
		AStarNode current = openSet.top();
		openSet.pop();

		uint64_t curKey = packKey(current.pos.x, current.pos.z);

		if (curKey == goalKey) {
			// reconstruct path
			Vec3Int node = goal;
			while (!(node.x == start.x && node.z == start.z)) {
				path.push_back(node);
				uint64_t nodeKey = packKey(node.x, node.z);
				auto it = cameFrom.find(nodeKey);
				if (it == cameFrom.end()) break;
				node = it->second;
			}
			std::reverse(path.begin(), path.end());
			return path;
		}

		if (closedSet.count(curKey)) continue;
		closedSet.insert(curKey);

		for (const auto& [neighborPos, moveCost] : getNeighbors(map, current.pos)) {
			uint64_t neighborKey = packKey(neighborPos.x, neighborPos.z);
			if (closedSet.count(neighborKey)) continue;

			float tentativeG = current.gCost + moveCost;
			auto gIt = gScores.find(neighborKey);
			if (gIt == gScores.end() || tentativeG < gIt->second) {
				gScores[neighborKey] = tentativeG;
				cameFrom[neighborKey] = current.pos;
				float hNew = (float)(std::abs(goal.x - neighborPos.x) + std::abs(goal.z - neighborPos.z));
				openSet.push({neighborPos, tentativeG, tentativeG + hNew});
			}
		}
	}

	return path; // empty = no path found
}
