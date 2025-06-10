#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include <vector>
#include <glm/glm.hpp>
#include "../models/CCube.h"
#include <memory>

// AABB 包圍盒結構
// AABB 包圍盒結構
struct AABB {
    glm::vec3 min, max;
    std::string type; // ADDED: To describe the wall segment

    AABB() = default;
    AABB(const glm::vec3& minPoint, const glm::vec3& maxPoint, const std::string& type_desc = "Generic Wall")
        : min(minPoint), max(maxPoint), type(type_desc) {}

    // ... (rest of AABB methods remain the same) ...

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

// Structure to define door parameters for an arched doorway
struct DoorwayConfig {
    bool hasDoor = false;
    glm::vec3 doorCenter; // Center of the door opening (X,Y,Z based on wall plane)
    float doorWidth = 0.0f; // Total open width
    float doorHeight = 0.0f; // Total open height
    float postWidth = 0.0f; // Width of the side posts
    float lintelHeight = 0.0f; // Height of the top lintel
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
        this->walls.clear();
        float wallThickness = 1.5f;

        float roomX = 26.0f;  // Room 5 X範圍: -13到+13 = 26
        float roomZ = 24.0f;  // Room 5 Z範圍: 0到24 = 24
        float roomY = 20.0f;  // 保持原有高度
        float roomHalfX = roomX / 2.0f;  // 14.0f
        float roomHalfY = roomY / 2.0f;  // 10.0f
        float roomHalfZ = roomZ / 2.0f;  // 12.0f

        // Common door dimensions (adjust as needed)
        float commonDoorWidth = 8.0f;
        float commonDoorHeight = 18.0f;
        float commonPostWidth = 2.0f;
        float commonLintelHeight = 2.0f;
     
     // 門的Y軸位置 - 讓門從地板開始 (拱門效果)
     float doorBottomY = 0.0f;  // 門底部從地板開始 (房間MinY = 0)
     float doorCenterY = doorBottomY + commonDoorHeight / 2.0f;  // 門中心Y = 6.0f
     
        // Room 5 (底部中間) 座標: X=0, Z=12 (Z範圍0-24)
        glm::vec3 room5Center = glm::vec3(0.0f, roomHalfY, 12.0f);
        // Room 2 (頂部中間) 座標: X=0, Z=-12 (Z範圍-24-0)
        glm::vec3 room2Center = glm::vec3(0.0f, roomHalfY, -12.0f);
        // Room 1 (頂部左邊) 座標: X=-28, Z=-12
        glm::vec3 room1Center = glm::vec3(-26.0f, roomHalfY, -12.0f);
        // Room 3 (頂部右邊) 座標: X=+28, Z=-12
        glm::vec3 room3Center = glm::vec3(26.0f, roomHalfY, -12.0f);
        // Room 4 (底部左邊) 座標: X=-28, Z=12
        glm::vec3 room4Center = glm::vec3(-26.0f, roomHalfY, 12.0f);
        // Room 6 (底部右邊) 座標: X=+28, Z=12
        glm::vec3 room6Center = glm::vec3(26.0f, roomHalfY, 12.0f);

        
        // Define doorway configurations
        DoorwayConfig doorConfig_R1_R = {true, glm::vec3(room1Center.x + roomHalfX, doorCenterY, room1Center.z), commonDoorWidth, commonDoorHeight, commonPostWidth, commonLintelHeight};
        DoorwayConfig doorConfig_R2_L = {true, glm::vec3(room2Center.x - roomHalfX, doorCenterY, room2Center.z), commonDoorWidth, commonDoorHeight, commonPostWidth, commonLintelHeight};
        DoorwayConfig doorConfig_R2_B = {true, glm::vec3(room2Center.x, doorCenterY, room2Center.z + roomHalfZ), commonDoorWidth, commonDoorHeight, commonPostWidth, commonLintelHeight};
        DoorwayConfig doorConfig_R3_B = {true, glm::vec3(room3Center.x, doorCenterY, room3Center.z + roomHalfZ), commonDoorWidth, commonDoorHeight, commonPostWidth, commonLintelHeight};
        DoorwayConfig doorConfig_R4_R = {true, glm::vec3(room4Center.x + roomHalfX, doorCenterY, room4Center.z), commonDoorWidth, commonDoorHeight, commonPostWidth, commonLintelHeight};
        DoorwayConfig doorConfig_R5_F = {true, glm::vec3(room5Center.x, doorCenterY, room5Center.z - roomHalfZ), commonDoorWidth, commonDoorHeight, commonPostWidth, commonLintelHeight};
        DoorwayConfig doorConfig_R5_L = {true, glm::vec3(room5Center.x - roomHalfX, doorCenterY, room5Center.z), commonDoorWidth, commonDoorHeight, commonPostWidth, commonLintelHeight};
        DoorwayConfig doorConfig_R5_R = {true, glm::vec3(room5Center.x + roomHalfX, doorCenterY, room5Center.z), commonDoorWidth, commonDoorHeight, commonPostWidth, commonLintelHeight};
        DoorwayConfig doorConfig_R6_F = {true, glm::vec3(room6Center.x, doorCenterY, room6Center.z - roomHalfZ), commonDoorWidth, commonDoorHeight, commonPostWidth, commonLintelHeight};
        DoorwayConfig doorConfig_R6_L = {true, glm::vec3(room6Center.x - roomHalfX, doorCenterY, room6Center.z), commonDoorWidth, commonDoorHeight, commonPostWidth, commonLintelHeight};


        // Room 1: Top-Left
        addRoomWalls(this->walls, 1, room1Center, roomX, roomY, roomZ, wallThickness,
                     {}, {}, {}, doorConfig_R1_R
                     );

        // Room 2: Top-Middle
        addRoomWalls(this->walls, 2, room2Center, roomX, roomY, roomZ, wallThickness,
                     {}, doorConfig_R2_B, doorConfig_R2_L, {}
                     );

        // Room 3: Top-Right
        addRoomWalls(this->walls, 3, room3Center, roomX, roomY, roomZ, wallThickness,
                     {}, doorConfig_R3_B, {}, {}
                     );

        // Room 4: Bottom-Left
        addRoomWalls(this->walls, 4, room4Center, roomX, roomY, roomZ, wallThickness,
                     {}, {}, {}, doorConfig_R4_R
                     );

        // Room 5: Bottom-Middle
        addRoomWalls(this->walls, 5, room5Center, roomX, roomY, roomZ, wallThickness,
                     doorConfig_R5_F, {}, doorConfig_R5_L, doorConfig_R5_R
                     );

        // Room 6: Bottom-Right
        addRoomWalls(this->walls, 6, room6Center, roomX, roomY, roomZ, wallThickness,
                     doorConfig_R6_F, {}, doorConfig_R6_L, {}
                     );
    }
    // 添加障礙物
    void addObstacle(const AABB& obstacle) {
        obstacles.push_back(obstacle);
    }
    
    // 檢查攝影機位置是否會發生碰撞
    bool checkCameraCollision(const glm::vec3& newPosition) {
        cameraCollider.center = newPosition;
        
        for (size_t i = 0; i < walls.size(); ++i) {
            const auto& wall = walls[i];

            if (cameraCollider.intersects(wall)) {
                std::cout << "Collision Detected!" << std::endl;
                std::cout << "  Camera Attempted Position: (" << newPosition.x << ", " << newPosition.y << ", " << newPosition.z << ")" << std::endl;
                std::cout << "  Collided with Wall Segment: " << wall.type << std::endl; // Use the stored type
                std::cout << "  Wall Min: (" << wall.min.x << ", " << wall.min.y << ", " << wall.min.z << ")" << std::endl; //
                std::cout << "  Wall Max: (" << wall.max.x << ", " << wall.max.y << ", " << wall.max.z << ")" << std::endl; //
                return true;
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
    
    // Helper function to create a wall segment
    void createWallSegment(std::vector<AABB>& wallsVec, const glm::vec3& minPoint, const glm::vec3& maxPoint, const std::string& type_desc) {
        wallsVec.push_back(AABB(minPoint, maxPoint, type_desc));
    }

    // Function to add a room's walls - MODIFIED FOR ARCHED DOORS
    void addRoomWalls(std::vector<AABB>& wallsVec, int roomIndex, const glm::vec3& roomCenter, float roomXSize, float roomYSize, float roomZSize, float wallThickness,
                      DoorwayConfig doorFrontConfig = {}, DoorwayConfig doorBackConfig = {},
                      DoorwayConfig doorLeftConfig = {}, DoorwayConfig doorRightConfig = {}) {

        float roomHalfX = roomXSize / 2.0f;
        float roomHalfY = roomYSize / 2.0f;
        float roomHalfZ = roomZSize / 2.0f;

        float roomMinX = roomCenter.x - roomHalfX;
        float roomMaxX = roomCenter.x + roomHalfX;
        float roomMinY = roomCenter.y - roomHalfY;
        float roomMaxY = roomCenter.y + roomHalfY;
        float roomMinZ = roomCenter.z - roomHalfZ;
        float roomMaxZ = roomCenter.z + roomHalfZ;

        std::string roomPrefix = "Room " + std::to_string(roomIndex) + " - ";

        // --- Left Wall (-X) ---
        if (doorLeftConfig.hasDoor) {
            float minX = roomMinX - wallThickness;
            float maxX = roomMinX;
            float doorBottomY = doorLeftConfig.doorCenter.y - doorLeftConfig.doorHeight / 2.0f;
            float doorTopY = doorLeftConfig.doorCenter.y + doorLeftConfig.doorHeight / 2.0f;
            float doorMinZ = doorLeftConfig.doorCenter.z - doorLeftConfig.doorWidth / 2.0f; // Door width on Z-axis for Left/Right walls
            float doorMaxZ = doorLeftConfig.doorCenter.z + doorLeftConfig.doorWidth / 2.0f;

            // Wall segment below the door (Y-axis)
            if (roomMinY < doorBottomY) {
                createWallSegment(wallsVec, glm::vec3(minX, roomMinY, roomMinZ),
                                  glm::vec3(maxX, doorBottomY, roomMaxZ), roomPrefix + "Left Wall (Below Door)");
            }
            // Wall segment above the door (Y-axis)
            if (roomMaxY > doorTopY) {
                createWallSegment(wallsVec, glm::vec3(minX, doorTopY, roomMinZ),
                                  glm::vec3(maxX, roomMaxY, roomMaxZ), roomPrefix + "Left Wall (Above Door)");
            }

            // Left Post of the door (Z-axis)
            createWallSegment(wallsVec, glm::vec3(minX, doorBottomY, doorMinZ - doorLeftConfig.postWidth),
                              glm::vec3(maxX, doorTopY - doorLeftConfig.lintelHeight, doorMinZ), roomPrefix + "Left Wall (Door Left Post)");
            // Right Post of the door (Z-axis)
            createWallSegment(wallsVec, glm::vec3(minX, doorBottomY, doorMaxZ),
                              glm::vec3(maxX, doorTopY - doorLeftConfig.lintelHeight, doorMaxZ + doorLeftConfig.postWidth), roomPrefix + "Left Wall (Door Right Post)");
            // Lintel of the door (Top horizontal part)
            createWallSegment(wallsVec, glm::vec3(minX, doorTopY - doorLeftConfig.lintelHeight, doorMinZ),
                              glm::vec3(maxX, doorTopY, doorMaxZ), roomPrefix + "Left Wall (Door Lintel)");

            // Wall segments to the left/right of the door on the Z-axis
            if (roomMinZ < doorMinZ - doorLeftConfig.postWidth) {
                createWallSegment(wallsVec, glm::vec3(minX, doorBottomY, roomMinZ),
                                  glm::vec3(maxX, doorTopY, doorMinZ - doorLeftConfig.postWidth), roomPrefix + "Left Wall (Left of Doorway)");
            }
            if (roomMaxZ > doorMaxZ + doorLeftConfig.postWidth) {
                createWallSegment(wallsVec, glm::vec3(minX, doorBottomY, doorMaxZ + doorLeftConfig.postWidth),
                                  glm::vec3(maxX, doorTopY, roomMaxZ), roomPrefix + "Left Wall (Right of Doorway)");
            }

        } else {
            createWallSegment(wallsVec, glm::vec3(roomMinX - wallThickness, roomMinY, roomMinZ),
                              glm::vec3(roomMinX, roomMaxY, roomMaxZ), roomPrefix + "Left Wall (Solid)");
        }


        // --- Right Wall (+X) ---
        if (doorRightConfig.hasDoor) {
            float minX = roomMaxX;
            float maxX = roomMaxX + wallThickness;
            float doorBottomY = doorRightConfig.doorCenter.y - doorRightConfig.doorHeight / 2.0f;
            float doorTopY = doorRightConfig.doorCenter.y + doorRightConfig.doorHeight / 2.0f;
            float doorMinZ = doorRightConfig.doorCenter.z - doorRightConfig.doorWidth / 2.0f;
            float doorMaxZ = doorRightConfig.doorCenter.z + doorRightConfig.doorWidth / 2.0f;

            if (roomMinY < doorBottomY) {
                createWallSegment(wallsVec, glm::vec3(minX, roomMinY, roomMinZ),
                                  glm::vec3(maxX, doorBottomY, roomMaxZ), roomPrefix + "Right Wall (Below Door)");
            }
            if (roomMaxY > doorTopY) {
                createWallSegment(wallsVec, glm::vec3(minX, doorTopY, roomMinZ),
                                  glm::vec3(maxX, roomMaxY, roomMaxZ), roomPrefix + "Right Wall (Above Door)");
            }

            createWallSegment(wallsVec, glm::vec3(minX, doorBottomY, doorMinZ - doorRightConfig.postWidth),
                              glm::vec3(maxX, doorTopY - doorRightConfig.lintelHeight, doorMinZ), roomPrefix + "Right Wall (Door Left Post)");
            createWallSegment(wallsVec, glm::vec3(minX, doorBottomY, doorMaxZ),
                              glm::vec3(maxX, doorTopY - doorRightConfig.lintelHeight, doorMaxZ + doorRightConfig.postWidth), roomPrefix + "Right Wall (Door Right Post)");
            createWallSegment(wallsVec, glm::vec3(minX, doorTopY - doorRightConfig.lintelHeight, doorMinZ),
                              glm::vec3(maxX, doorTopY, doorMaxZ), roomPrefix + "Right Wall (Door Lintel)");

            if (roomMinZ < doorMinZ - doorRightConfig.postWidth) {
                createWallSegment(wallsVec, glm::vec3(minX, doorBottomY, roomMinZ),
                                  glm::vec3(maxX, doorTopY, doorMinZ - doorRightConfig.postWidth), roomPrefix + "Right Wall (Left of Doorway)");
            }
            if (roomMaxZ > doorMaxZ + doorRightConfig.postWidth) {
                createWallSegment(wallsVec, glm::vec3(minX, doorBottomY, doorMaxZ + doorRightConfig.postWidth),
                                  glm::vec3(maxX, doorTopY, roomMaxZ), roomPrefix + "Right Wall (Right of Doorway)");
            }

        } else {
            createWallSegment(wallsVec, glm::vec3(roomMaxX, roomMinY, roomMinZ),
                              glm::vec3(roomMaxX + wallThickness, roomMaxY, roomMaxZ), roomPrefix + "Right Wall (Solid)");
        }


        // --- Front Wall (-Z) ---
        if (doorFrontConfig.hasDoor) {
            float minZ = roomMinZ - wallThickness;
            float maxZ = roomMinZ;
            float doorBottomY = doorFrontConfig.doorCenter.y - doorFrontConfig.doorHeight / 2.0f;
            float doorTopY = doorFrontConfig.doorCenter.y + doorFrontConfig.doorHeight / 2.0f;
            float doorMinX = doorFrontConfig.doorCenter.x - doorFrontConfig.doorWidth / 2.0f;
            float doorMaxX = doorFrontConfig.doorCenter.x + doorFrontConfig.doorWidth / 2.0f;

            // Wall segment below the door (Y-axis)
            if (roomMinY < doorBottomY) {
                createWallSegment(wallsVec, glm::vec3(roomMinX, roomMinY, minZ),
                                  glm::vec3(roomMaxX, doorBottomY, maxZ), roomPrefix + "Front Wall (Below Door)");
            }
            // Wall segment above the door (Y-axis)
            if (roomMaxY > doorTopY) {
                createWallSegment(wallsVec, glm::vec3(roomMinX, doorTopY, minZ),
                                  glm::vec3(roomMaxX, roomMaxY, maxZ), roomPrefix + "Front Wall (Above Door)");
            }

            // Left Post of the door (X-axis)
            createWallSegment(wallsVec, glm::vec3(doorMinX - doorFrontConfig.postWidth, doorBottomY, minZ),
                              glm::vec3(doorMinX, doorTopY - doorFrontConfig.lintelHeight, maxZ), roomPrefix + "Front Wall (Door Left Post)");
            // Right Post of the door (X-axis)
            createWallSegment(wallsVec, glm::vec3(doorMaxX, doorBottomY, minZ),
                              glm::vec3(doorMaxX + doorFrontConfig.postWidth, doorTopY - doorFrontConfig.lintelHeight, maxZ), roomPrefix + "Front Wall (Door Right Post)");
            // Lintel of the door (Top horizontal part)
            createWallSegment(wallsVec, glm::vec3(doorMinX, doorTopY - doorFrontConfig.lintelHeight, minZ),
                              glm::vec3(doorMaxX, doorTopY, maxZ), roomPrefix + "Front Wall (Door Lintel)");

            // Wall segments to the left/right of the door on the X-axis
            if (roomMinX < doorMinX - doorFrontConfig.postWidth) {
                createWallSegment(wallsVec, glm::vec3(roomMinX, doorBottomY, minZ),
                                  glm::vec3(doorMinX - doorFrontConfig.postWidth, doorTopY, maxZ), roomPrefix + "Front Wall (Left of Doorway)");
            }
            if (roomMaxX > doorMaxX + doorFrontConfig.postWidth) {
                createWallSegment(wallsVec, glm::vec3(doorMaxX + doorFrontConfig.postWidth, doorBottomY, minZ),
                                  glm::vec3(roomMaxX, doorTopY, maxZ), roomPrefix + "Front Wall (Right of Doorway)");
            }

        } else {
            createWallSegment(wallsVec, glm::vec3(roomMinX, roomMinY, roomMinZ - wallThickness),
                              glm::vec3(roomMaxX, roomMaxY, roomMinZ), roomPrefix + "Front Wall (Solid)");
        }


        // --- Back Wall (+Z) ---
        if (doorBackConfig.hasDoor) {
            float minZ = roomMaxZ;
            float maxZ = roomMaxZ + wallThickness;
            float doorBottomY = doorBackConfig.doorCenter.y - doorBackConfig.doorHeight / 2.0f;
            float doorTopY = doorBackConfig.doorCenter.y + doorBackConfig.doorHeight / 2.0f;
            float doorMinX = doorBackConfig.doorCenter.x - doorBackConfig.doorWidth / 2.0f;
            float doorMaxX = doorBackConfig.doorCenter.x + doorBackConfig.doorWidth / 2.0f;

            if (roomMinY < doorBottomY) {
                createWallSegment(wallsVec, glm::vec3(roomMinX, roomMinY, minZ),
                                  glm::vec3(roomMaxX, doorBottomY, maxZ), roomPrefix + "Back Wall (Below Door)");
            }
            if (roomMaxY > doorTopY) {
                createWallSegment(wallsVec, glm::vec3(roomMinX, doorTopY, minZ),
                                  glm::vec3(roomMaxX, roomMaxY, maxZ), roomPrefix + "Back Wall (Above Door)");
            }

            createWallSegment(wallsVec, glm::vec3(doorMinX - doorBackConfig.postWidth, doorBottomY, minZ),
                              glm::vec3(doorMinX, doorTopY - doorBackConfig.lintelHeight, maxZ), roomPrefix + "Back Wall (Door Left Post)");
            createWallSegment(wallsVec, glm::vec3(doorMaxX, doorBottomY, minZ),
                              glm::vec3(doorMaxX + doorBackConfig.postWidth, doorTopY - doorBackConfig.lintelHeight, maxZ), roomPrefix + "Back Wall (Door Right Post)");
            createWallSegment(wallsVec, glm::vec3(doorMinX, doorTopY - doorBackConfig.lintelHeight, minZ),
                              glm::vec3(doorMaxX, doorTopY, maxZ), roomPrefix + "Back Wall (Door Lintel)");

            if (roomMinX < doorMinX - doorBackConfig.postWidth) {
                createWallSegment(wallsVec, glm::vec3(roomMinX, doorBottomY, minZ),
                                  glm::vec3(doorMinX - doorBackConfig.postWidth, doorTopY, maxZ), roomPrefix + "Back Wall (Left of Doorway)");
            }
            if (roomMaxX > doorMaxX + doorBackConfig.postWidth) {
                createWallSegment(wallsVec, glm::vec3(doorMaxX + doorBackConfig.postWidth, doorBottomY, minZ),
                                  glm::vec3(roomMaxX, doorTopY, maxZ), roomPrefix + "Back Wall (Right of Doorway)");
            }

        } else {
            createWallSegment(wallsVec, glm::vec3(roomMinX, roomMinY, roomMaxZ),
                              glm::vec3(roomMaxX, roomMaxY, roomMaxZ + wallThickness), roomPrefix + "Back Wall (Solid)");
        }


        // Floor (Y-axis)
        createWallSegment(wallsVec, glm::vec3(roomMinX, roomMinY - wallThickness, roomMinZ),
                          glm::vec3(roomMaxX, roomMinY, roomMaxZ), roomPrefix + "Floor");

        // Ceiling (Y-axis)
        createWallSegment(wallsVec, glm::vec3(roomMinX, roomMaxY, roomMinZ),
                          glm::vec3(roomMaxX, roomMaxY + wallThickness, roomMaxZ), roomPrefix + "Ceiling");
    }
};

#endif // COLLISION_SYSTEM_H
