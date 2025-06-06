#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include <vector>
#include <glm/glm.hpp>
#include "../models/CCube.h"
#include <memory>

// AABB 包圍盒結構
struct AABB {
    glm::vec3 min, max;
    
    AABB() = default;
    AABB(const glm::vec3& minPoint, const glm::vec3& maxPoint)
        : min(minPoint), max(maxPoint) {}
    
    // 檢查兩個AABB是否相交
    bool intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }
    
    // 檢查點是否在AABB內
    bool contains(const glm::vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
    
    // 更新AABB位置
    void updatePosition(const glm::vec3& offset) {
        min += offset;
        max += offset;
    }
};

// 球體包圍盒結構
struct Sphere {
    glm::vec3 center;
    float radius;
    
    Sphere() = default;
    Sphere(const glm::vec3& c, float r) : center(c), radius(r) {}
    
    // 檢查兩個球體是否相交
    bool intersects(const Sphere& other) const {
        float distance = glm::length(center - other.center);
        return distance < (radius + other.radius);
    }
    
    // 檢查球體是否與AABB相交
    bool intersects(const AABB& box) const {
        // 找到AABB上最接近球心的點
        glm::vec3 closestPoint = glm::clamp(center, box.min, box.max);
        float distance = glm::length(center - closestPoint);
        return distance < radius;
    }
    
};

// 碰撞檢測管理器
class CollisionManager {
private:
    std::vector<AABB> walls;           // 牆壁的碰撞盒
    std::vector<AABB> obstacles;       // 障礙物的碰撞盒
    Sphere cameraCollider;             // 攝影機的球體碰撞器
    std::vector<Sphere> sphereObstacles; // 球體障礙物
    float m_cameraRadius = 0.3f;
    
public:
    CollisionManager() {
        // 設置攝影機碰撞器（半徑0.3f）
        cameraCollider.radius = 0.3f;
        initializeWalls();
    }
    
    // 初始化場景中的牆壁
    void initializeWalls() {
        // 範例：創建一個房間的牆壁
        walls.clear();
        
        glm::vec3 roomCenter = glm::vec3(0.0f, 10.1f, 0.0f);
                
        float roomHalfSize = 8.6f;
        // 牆壁厚度
        float wallThickness = 2.0f;

        // 計算房間的內部邊界
        float roomMinX = roomCenter.x - roomHalfSize; // -5.0f
        float roomMaxX = roomCenter.x + roomHalfSize; //  5.0f
        float roomMinY = roomCenter.y - roomHalfSize; //  1.5f
        float roomMaxY = roomCenter.y + roomHalfSize; // 11.5f
        float roomMinZ = roomCenter.z - roomHalfSize; // -5.0f
        float roomMaxZ = roomCenter.z + roomHalfSize; //  5.0f

        // 1. 左牆 (X 軸方向)
        walls.push_back(AABB(glm::vec3(roomMinX - wallThickness, roomMinY, roomMinZ),
                               glm::vec3(roomMinX,roomMaxY, roomMaxZ)));

        // 2. 右牆 (X 軸方向)
        walls.push_back(AABB(glm::vec3(roomMaxX, roomMinY, roomMinZ),
                               glm::vec3(roomMaxX + wallThickness, roomMaxY, roomMaxZ)));

        // 3. 前牆 (Z 軸方向)
        walls.push_back(AABB(glm::vec3(roomMinX, roomMinY, roomMinZ - wallThickness),
                               glm::vec3(roomMaxX, roomMaxY, roomMinZ)));

        // 4. 後牆 (Z 軸方向)
        walls.push_back(AABB(glm::vec3(roomMinX, roomMinY, roomMaxZ),
                               glm::vec3(roomMaxX, roomMaxY, roomMaxZ + wallThickness)));

        // 5. 地板 (Y 軸方向)
        walls.push_back(AABB(glm::vec3(roomMinX, roomMinY - wallThickness, roomMinZ),
                               glm::vec3(roomMaxX, roomMinY, roomMaxZ)));

        // 6. 天花板 (Y 軸方向)
        walls.push_back(AABB(glm::vec3(roomMinX, roomMaxY, roomMinZ),
                               glm::vec3(roomMaxX, roomMaxY + wallThickness, roomMaxZ)));
    }
    
    // 添加障礙物
    void addObstacle(const AABB& obstacle) {
        obstacles.push_back(obstacle);
    }
    
    // 檢查攝影機位置是否會發生碰撞
    bool checkCameraCollision(const glm::vec3& newPosition) {
        cameraCollider.center = newPosition;
        
        // 檢查與牆壁的碰撞
        // 檢查與牆壁的碰撞
        for (size_t i = 0; i < walls.size(); ++i) { // Use index to identify the wall
            const auto& wall = walls[i]; // Get the current wall

            if (cameraCollider.intersects(wall)) {
                std::cout << "checkCameraCollision = true" << std::endl;
                std::cout << "Collision Pos (Camera attempt): (" << newPosition.x << "," << newPosition.y << "," << newPosition.z << ")" << std::endl;

                // --- ADDED DEBUGGING OUTPUT FOR THE COLLIDED WALL ---
                std::cout << "Collided with Wall #" << i << ":" << std::endl;
                std::cout << "  Wall Min: (" << wall.min.x << "," << wall.min.y << "," << wall.min.z << ")" << std::endl;
                std::cout << "  Wall Max: (" << wall.max.x << "," << wall.max.y << "," << wall.max.z << ")" << std::endl;
                // You can also add more descriptive names if you store them, or map index to names
                if (i == 0) std::cout << "  Wall Type: Left Wall" << std::endl;
                else if (i == 1) std::cout << "  Wall Type: Right Wall" << std::endl;
                else if (i == 2) std::cout << "  Wall Type: Front Wall" << std::endl;
                else if (i == 3) std::cout << "  Wall Type: Back Wall" << std::endl;
                else if (i == 4) std::cout << "  Wall Type: Floor" << std::endl;
                else if (i == 5) std::cout << "  Wall Type: Ceiling" << std::endl;
                // --- END ADDED DEBUGGING OUTPUT ---

                return true; // Return true as soon as a collision is found
            }
        }

        // 檢查與障礙物的碰撞
        for (const auto& obstacle : obstacles) {
            if (cameraCollider.intersects(obstacle)) {
                std::cout << "checkCameraCollision = true (Obstacle)" << std::endl;
                std::cout << "Collision Pos (Camera attempt): (" << newPosition.x << "," << newPosition.y << "," << newPosition.z << ")" << std::endl;
                // You might want to print obstacle details here too
                return true;
            }
        }

        return false;
    }
    
    // 計算滑動向量（沿著牆面滑動）
    glm::vec3 calculateSliding(const glm::vec3& originalMovement,
                              const glm::vec3& currentPos) {
        glm::vec3 newPos = currentPos + originalMovement;
        
        if (!checkCameraCollision(newPos)) {
            return originalMovement;
        }
        
        // 嘗試分別在X、Y、Z軸上移動
        glm::vec3 xOnly(originalMovement.x, 0.0f, 0.0f);
        glm::vec3 yOnly(0.0f, originalMovement.y, 0.0f);
        glm::vec3 zOnly(0.0f, 0.0f, originalMovement.z);
        
        glm::vec3 result(0.0f);
        
        if (!checkCameraCollision(currentPos + xOnly)) {
            result += xOnly;
        }
        if (!checkCameraCollision(currentPos + yOnly)) {
            result += yOnly;
        }
        if (!checkCameraCollision(currentPos + zOnly)) {
            result += zOnly;
        }
        
        return result;
    }
    
    // 安全移動攝影機
    glm::vec3 getSafeMovement(const glm::vec3& movement,
                             const glm::vec3& currentPos) {
        glm::vec3 targetPos = currentPos + movement;
        
        // 如果目標位置沒有碰撞，直接返回原始移動
        if (!checkCameraCollision(targetPos)) {
            return movement;
        }
        
        // 使用滑動碰撞
        return calculateSliding(movement, currentPos);
    }
    
    // 射線檢測（用於預測碰撞）
    bool raycast(const glm::vec3& origin, const glm::vec3& direction,
                float maxDistance, glm::vec3& hitPoint) {
        // 簡化的射線檢測實現
        const int steps = 20;
        float stepSize = maxDistance / steps;
        
        for (int i = 1; i <= steps; ++i) {
            glm::vec3 testPos = origin + direction * (stepSize * i);
            if (checkCameraCollision(testPos)) {
                hitPoint = testPos;
                return true;
            }
        }
        return false;
    }
    
    // 獲取攝影機碰撞器半徑
    float getCameraRadius() const { return cameraCollider.radius; }
    
    // 設置攝影機碰撞器半徑
    void setCameraRadius(float radius) { cameraCollider.radius = radius; }
    
    // 清除所有障礙物
    void clearObstacles() { obstacles.clear(); }
    
    // 獲取牆壁數量
    size_t getWallCount() const { return walls.size(); }
    
    // 獲取障礙物數量
    size_t getObstacleCount() const { return obstacles.size(); }
    
    const std::vector<AABB>& getObstacles() const {
            return obstacles;
        }
    // 獲取所有球體障礙物
    const std::vector<Sphere>& getSphereObstacles() const {
        return sphereObstacles;
    }
    
    const std::vector<AABB>& getWalls() const {
        return walls;
    }
    
    // 動態創建牆壁視覺化
    void createWallVisualization(std::vector<std::unique_ptr<CCube>>& wallCubes) const  {
        wallCubes.clear();
        auto walls = getWalls();
        
        // 顏色陣列
        std::vector<glm::vec4> colors = {
            glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), // 紅色 - 左牆
            glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), // 紅色 - 右牆
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), // 綠色 - 前牆
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), // 綠色 - 後牆
            glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), // 藍色 - 地板
            glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)  // 藍色 - 天花板
        };
        
        for (size_t i = 0; i < walls.size(); ++i) {
            std::unique_ptr<CCube> wallCube = std::make_unique<CCube>();
            
            // 計算中心點和尺寸
            glm::vec3 center = (walls[i].min + walls[i].max) * 0.5f;
            glm::vec3 size = walls[i].max - walls[i].min;
            
            wallCube->setPos(center);
            wallCube->setColor(colors[i]);
            wallCube->setScale(size);
            
            wallCubes.push_back(std::move(wallCube));
        }
    }
    
    // 相機碰撞處理
    glm::vec3 handleCameraCollision(const glm::vec3& currentCamPos, const glm::vec3& nextCamPos, float cameraRadius) {
        Sphere cameraSphere(nextCamPos, cameraRadius);
        glm::vec3 newPosition = nextCamPos;

        for (const auto& wall : getWalls()) {
            if (cameraSphere.intersects(wall)) {
                // 如果發生碰撞，將相機位置限制在牆壁之外
                // 這是一個簡化的碰撞響應，可能需要更複雜的投影或推開算法
                // 目前的實現只是阻止相機進入牆壁
                // 這裡的邏輯可以根據您的需求進行調整

                // 為了避免穿牆，我們可以將相機位置回退到碰撞前的位置，或者推開
                // 這裡我們嘗試推開，但需要更精確的向量運算
                // 對於簡單的AABB，我們可以判斷是在哪個軸上發生碰撞，然後調整該軸的位置

                // 獲取相機移動方向
                glm::vec3 moveDir = glm::normalize(nextCamPos - currentCamPos);

                // 嘗試沿每個軸推開
                // x 軸
                if (nextCamPos.x + cameraRadius > wall.min.x && nextCamPos.x - cameraRadius < wall.max.x &&
                    nextCamPos.y + cameraRadius > wall.min.y && nextCamPos.y - cameraRadius < wall.max.y &&
                    nextCamPos.z + cameraRadius > wall.min.x && nextCamPos.z - cameraRadius < wall.max.z) { // 這行有問題
                     // 正確判斷 z 軸的碰撞
                     if (nextCamPos.z + cameraRadius > wall.min.z && nextCamPos.z - cameraRadius < wall.max.z) {
                        // 在 x 軸上發生碰撞
                        if (moveDir.x > 0) { // 從左邊進入
                            newPosition.x = wall.min.x - cameraRadius;
                        } else if (moveDir.x < 0) { // 從右邊進入
                            newPosition.x = wall.max.x + cameraRadius;
                        }
                    }
                }

                // y 軸
                if (nextCamPos.y + cameraRadius > wall.min.y && nextCamPos.y - cameraRadius < wall.max.y &&
                    nextCamPos.x + cameraRadius > wall.min.x && nextCamPos.x - cameraRadius < wall.max.x &&
                    nextCamPos.z + cameraRadius > wall.min.z && nextCamPos.z - cameraRadius < wall.max.z) {
                    // 在 y 軸上發生碰撞
                    if (moveDir.y > 0) { // 從下進入
                        newPosition.y = wall.min.y - cameraRadius;
                    } else if (moveDir.y < 0) { // 從上進入
                        newPosition.y = wall.max.y + cameraRadius;
                    }
                }

                // z 軸
                if (nextCamPos.z + cameraRadius > wall.min.z && nextCamPos.z - cameraRadius < wall.max.z &&
                    nextCamPos.x + cameraRadius > wall.min.x && nextCamPos.x - cameraRadius < wall.max.x &&
                    nextCamPos.y + cameraRadius > wall.min.y && nextCamPos.y - cameraRadius < wall.max.y) {
                    // 在 z 軸上發生碰撞
                    if (moveDir.z > 0) { // 從前進入
                        newPosition.z = wall.min.z - cameraRadius;
                    } else if (moveDir.z < 0) { // 從後進入
                        newPosition.z = wall.max.z + cameraRadius;
                    }
                }
            }
        }
        return newPosition;
    }
};

#endif // COLLISION_SYSTEM_H
