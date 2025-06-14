//設計一個房間，至少包含六個區域，可以不用柱子，直接用兩個四方形做區隔即可，右邊為示意圖，你可以自己設計，綠色表示鏡頭一開始的位置
//(12%)畫出六個房間，並達成以下的條件（編譯要能過且正常執行才算）
// ✓  每一面牆都必須有貼圖
// ✓  必須至少有三個房間是使用燈光照明，而且是使用 Per Pixel Lighting
// ✓  每一個房間中都必須有放置裝飾品(部分必須有貼圖)
// ✓ (2%) 必須有房間顯示至少(含)一個 OBJ 檔的模型，而且包含貼圖
//(8%)環境與操控
// ✓(2%) 鏡頭的移動是根據目前的視角方向
// ✓(2%) 不會穿牆
// ✓(2%) 會被機關觸動的動態移動光源，使用 Per Pixel Lighting
// ✓(2%) 至少三面牆壁上半透明的玻璃
//(8 %)圖學相關功能的使用，必須在程式碼中以註解清楚標明
// ✓ (1%) 針對特定物件實現 Billboards 的功能
        //在Model.cpp中setBillboard等計算Billboard的功能，從主函式設定camera pos和view Matrix傳回函式使用
// ✓ (1%) 使用到  Mipmapped 的功能 （有具體的說明在程式碼中）
        //在Model.cpp中LoadTexture()中生成midmapp
// ✓ (2%) 有房間使用到 Light Map 的功能 （有具體的說明在程式碼中）
        // 在Model.cpp中讀取obj找欲設定light map模型的材質，詳細內容看Model.cpp的RenderMesh()和ProcessMaterials、shader
// ✓ (2%) 有物件使用到 Normal Map 的功能 （有具體的說明在程式碼中）
        //在Model.cpp中有讀取obj的mtl檔，再寫入shader詳細內容看Model.cpp的RenderMesh()和ProcessMaterials
// ✓ (2%) 有物件使用到 Environment Map 的功能 (有具體的說明在程式碼中）
        // 在Model.cpp中LoadCubeMapFromFiles讀取6張cube map的貼圖，RenderMesh()和ProcessMaterials綁到shader上
//(6%) 其他你覺得可以展示的技術，包含物理或是數學的運算
    //(3%)發射子彈並且在牆壁上留下彈孔
    //(3%)可以破壞房間內的擺設
//(4%) 創意分數
// ✓ //玩家模型跟著鏡頭移動旋轉
// ✓ //有自轉動畫的電風扇模型
//老師給分，請自行發揮

//#define GLM_ENABLE_EXPERIMENTAL 1

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtx/string_cast.hpp>

#include "common/initshader.h"
#include "common/arcball.h"
#include "common/wmhandler.h"
#include "common/CCamera.h"
#include "common/CShaderPool.h"
#include "common/CButton.h"
#include "models/CQuad.h"
#include "models/CBottle.h"
#include "models/CTeapot.h"
#include "models/CTorusKnot.h"
#include "models/CBox.h"
#include "models/CSphere.h"
#include "common/CLightManager.h"
#include "common/CollisionManager.h"

#include "Model.h"


#include "common/CLight.h"
#include "common/CMaterial.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 800 
#define ROW_NUM 30

CollisionManager g_collisionManager;

//CTeapot  g_teapot(5);
CTorusKnot g_tknot(4);
CSphere g_sphere;

glm::vec3 g_eyeloc(-28.0f, 6.0f, 10.0f);
CCube g_centerloc; // view center預設在 (0,0,0)，不做任何描繪操作
CQuad g_floor[ROW_NUM][ROW_NUM]; 

GLuint g_shadingProg;
GLuint g_uiShader;
GLuint g_modelVAO;
int g_modelVertexCount;

// 2d
std::array<CButton, 6> g_button = {
    CButton(50.0f, 50.0f, glm::vec4(0.20f, 0.45f, 0.45f, 1.0f), glm::vec4(0.60f, 0.85f, 0.85f, 1.0f)),
    CButton(50.0f, 50.0f, glm::vec4(0.45f, 0.35f, 0.65f, 1.0f), glm::vec4(0.85f, 0.75f, 0.95f, 1.0f)),
    CButton(50.0f, 50.0f, glm::vec4(0.45f, 0.35f, 0.65f, 1.0f), glm::vec4(0.85f, 0.75f, 0.95f, 1.0f)),
    CButton(50.0f, 50.0f, glm::vec4(0.45f, 0.35f, 0.65f, 1.0f), glm::vec4(0.85f, 0.75f, 0.95f, 1.0f)),
    CButton(50.0f, 50.0f, glm::vec4(0.45f, 0.35f, 0.65f, 1.0f), glm::vec4(0.85f, 0.75f, 0.95f, 1.0f)),
    CButton(50.0f, 50.0f, glm::vec4(0.45f, 0.35f, 0.65f, 1.0f), glm::vec4(0.85f, 0.75f, 0.95f, 1.0f))
};
glm::mat4 g_2dmxView = glm::mat4(1.0f);
glm::mat4 g_2dmxProj = glm::mat4(1.0f);
GLint g_2dviewLoc, g_2dProjLoc;


CLightManager lightManager;
// 全域光源 (位置在 5,5,0)
CLight* g_light = new CLight(
    glm::vec3(0.0f, 8.0f, 7.0f),
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    glm::vec4(0.6f, 0.6f, 0.6f, 1.0f),
    glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
    1.0f, 0.09f, 0.032f
);

CLight* g_light2 = new CLight(
    glm::vec3(24.0f, 8.0f, 7.0f),
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    glm::vec4(0.6f, 0.6f, 0.6f, 1.0f),
    glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
    1.0f, 0.09f, 0.032f
);
CLight* g_light3 = new CLight(
    glm::vec3(-24.0f, 8.0f, 7.0f),
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    glm::vec4(0.6f, 0.6f, 0.6f, 1.0f),
    glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
    1.0f, 0.09f, 0.032f
);
CLight* g_light4 = new CLight(
    glm::vec3(-24.0f, 8.0f, -7.0f),
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    glm::vec4(0.6f, 0.6f, 0.6f, 1.0f),
    glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
    1.0f, 0.09f, 0.032f
);
CLight* g_light5 = new CLight(
    glm::vec3(0.0f, 8.0f, -7.0f),
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    glm::vec4(0.6f, 0.6f, 0.6f, 1.0f),
    glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
    1.0f, 0.09f, 0.032f
);
CLight* g_light6 = new CLight(
    glm::vec3(24.0f, 8.0f, -7.0f),
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    glm::vec4(0.6f, 0.6f, 0.6f, 1.0f),
    glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
    1.0f, 0.09f, 0.032f
);

// 全域材質（可依模型分別設定）
CMaterial g_matBeige;   // 淺米白深麥灰
CMaterial g_matGray;    //  深麥灰材質
CMaterial g_matWaterBlue;
CMaterial g_matWaterGreen;
CMaterial g_matWaterRed;
CMaterial g_matWoodHoney;
CMaterial g_matWoodLightOak;
CMaterial g_matWoodBleached;

std::vector<std::unique_ptr<Model>> models;
std::vector<glm::mat4> modelMatrices;
std::vector<std::string> modelPaths = {
    "models/Room001.obj",
    "models/livingRoomTable.obj",
    "models/sofa.obj",
    "models/bed.obj",
    "models/toilet.obj",
    "models/desk.obj",
    "models/garden.obj",
    "models/woodCube.obj",
    "models/woodCube.obj",
    "models/Robot.obj",
    "models/fan.obj",
    "models/sign.obj",
    "models/Room001Window.obj",
    
};

void renderModel(const std::string& modelName, const glm::mat4& modelMatrix);
void adjustShaderEffects(float normalStrength, float specularStrength, float specularPower);

//----------------------------------------------------------------------------
void loadScene(void)
{
    g_shadingProg = CShaderPool::getInstance().getShader("v_phong.glsl", "f_phong.glsl");
    g_uiShader = CShaderPool::getInstance().getShader("ui_vtxshader.glsl", "ui_fragshader.glsl");
    
    adjustShaderEffects(3.0f, 4.0f, 2.0f);
    
    g_light->setIntensity(3.0);
    g_light2->setIntensity(3.0);
    g_light3->setIntensity(3.0);
    g_light4->setIntensity(3.0);
    g_light5->setIntensity(3.0);
    g_light6->setIntensity(3.0);
//    env->setIntensity(20.0);
    lightManager.addLight(g_light);
    lightManager.addLight(g_light2);
    lightManager.addLight(g_light3);
    lightManager.addLight(g_light4);
    lightManager.addLight(g_light5);
    lightManager.addLight(g_light6);
    
    lightManager.setShaderID(g_shadingProg);
//    g_light.setShaderID(g_shadingProg, "uLight");
    
//    initializeCollisionSystem();
    g_tknot.setupVertexAttributes();
    g_tknot.setShaderID(g_shadingProg, 3);
    g_tknot.setScale(glm::vec3(0.4f, 0.4f, 0.4f));
    g_tknot.setPos(glm::vec3(-2.0f, 0.5f, 2.0f));
    g_tknot.setMaterial(g_matWaterRed);
    
    // 載入模型 - 只需要傳入模型路徑！
    for (const auto& path : modelPaths) {
        auto model = std::make_unique<Model>();
        if (model->LoadModel(path)) {
            models.push_back(std::move(model));
            modelMatrices.push_back(glm::mat4(1.0f));
            std::cout << "Successfully loaded: " << path << std::endl;
        } else {
            std::cout << "Failed to load: " << path << std::endl;
        }
    }
    models[0]->SetLightMap("room.001", "models/textures/Room001_lightmap.png", 0.5);
    models[6]->SetLightMap("garden", "models/textures/garden_lightmap.png", 0.1);
    models[7]->SetEnvironmentMapFromFiles("wood", "models/textures/Sunny", 1.0);
    models[8]->SetEnvironmentMapFromFiles("wood", "models/textures/cubic2", 1.0);
    
	CCamera::getInstance().updateView(g_eyeloc); // 設定 eye 位置
    CCamera::getInstance().updateCenter(glm::vec3(0,4,0));
	CCamera::getInstance().updatePerspective(45.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);
    glm::mat4 mxView = CCamera::getInstance().getViewMatrix();
	glm::mat4 mxProj = CCamera::getInstance().getProjectionMatrix();

    GLint viewLoc = glGetUniformLocation(g_shadingProg, "mxView"); 	// 取得 view matrix 變數的位置
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mxView));

    GLint projLoc = glGetUniformLocation(g_shadingProg, "mxProj"); 	// 取得投影矩陣變數的位置
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(mxProj));
    
    // 產生  UI 所需的相關資源
    g_button[0].setScreenPos(570.0f, 150.0f);
    g_button[0].init(g_uiShader);
    g_button[1].setScreenPos(640.0f, 150.0f);
    g_button[1].init(g_uiShader);
    g_button[2].setScreenPos(710.0f, 150.0f);
    g_button[2].init(g_uiShader);
    g_button[3].setScreenPos(570.0f, 80.0f);
    g_button[3].init(g_uiShader);
    g_button[4].setScreenPos(640.0f, 80.0f);
    g_button[4].init(g_uiShader);
    g_button[5].setScreenPos(710.0f, 80.0f);
    g_button[5].init(g_uiShader);
    g_2dviewLoc = glGetUniformLocation(g_uiShader, "mxView");     // 取得 view matrix 變數位置
    glUniformMatrix4fv(g_2dviewLoc, 1, GL_FALSE, glm::value_ptr(g_2dmxView));

    g_2dProjLoc = glGetUniformLocation(g_uiShader, "mxProj");     // 取得 proj matrix 變數位置
    g_2dmxProj = glm::ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT, -1.0f, 1.0f);
    glUniformMatrix4fv(g_2dProjLoc, 1, GL_FALSE, glm::value_ptr(g_2dmxProj));

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 設定清除 back buffer 背景的顏色
    glEnable(GL_DEPTH_TEST); // 啟動深度測試
    
    setupCameraFollowObject();
}
//----------------------------------------------------------------------------

void render(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 設定 back buffer 的背景顏色
    
    glUseProgram(g_uiShader); // 使用 shader program
    glUniformMatrix4fv(g_2dviewLoc, 1, GL_FALSE, glm::value_ptr(g_2dmxView));
    glUniformMatrix4fv(g_2dProjLoc, 1, GL_FALSE, glm::value_ptr(g_2dmxProj));
    g_button[0].draw();
    g_button[1].draw();
    g_button[2].draw();
    g_button[3].draw();
    g_button[4].draw();
    g_button[5].draw();
    
    glUseProgram(g_shadingProg);
    
    //上傳光源與相機位置
    g_light->updateToShader();
    glUniform3fv(glGetUniformLocation(g_shadingProg, "viewPos"), 1, glm::value_ptr(g_eyeloc));
    glUniform3fv(glGetUniformLocation(g_shadingProg, "lightPos"), 1, glm::value_ptr(g_light->getPos()));
//    g_light.drawRaw();
    lightManager.updateAllLightsToShader();
        
    // 繪製光源視覺表示
    lightManager.draw();
    
    
    g_centerloc.drawRaw();
    
    //繪製obj model
    GLint modelLoc = glGetUniformLocation(g_shadingProg, "mxModel");
    for (size_t i = 0; i < models.size(); ++i) {
        glm::mat4 modelMatrix = modelMatrices[i];
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.7f));
//        if (i == 1) {
//            modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
//            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.7f));
//        }
//        if (i == 2) {
//            modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
//            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.7f));
//        }
        
        if (i == 7) {
            modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f, 1.5f, -6.0f));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(1.3f));
        }
        else if (i == 8) {
            modelMatrix = glm::translate(modelMatrix, glm::vec3(3.0f, 1.5f, -6.0f));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(1.3f));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }
        else if (i == 9 ){
            if (models[9]->isFollowingCamera()) {
                // 取得模型自己計算的矩陣，然後加上縮放
                modelMatrix = models[9]->getModelMatrix();
                modelMatrix = glm::scale(modelMatrix, glm::vec3(0.07f));
                // 可以加上額外的旋轉
                modelMatrix = glm::rotate(modelMatrix, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            } else {
//                // 如果不跟隨攝影機，使用原來的固定位置
                modelMatrix = glm::translate(modelMatrix, glm::vec3(5.0f, 1.15f, 5.0f));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(0.07f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(270.0f), glm::vec3(0.0f, 1.5f, 0.0f));
            }
        }
        else if(i == 10){
            modelMatrix = glm::translate(modelMatrix, glm::vec3(-30.0f, 17.0f, -8.0f));
            modelMatrix = modelMatrix * models[i]->getModelMatrix();
            models[10]->setSelfRotateMode(true, 2.0f);
        }
        else if(i == 11){
            modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 1.0f, -12.0f));
            modelMatrix = modelMatrix * models[i]->getModelMatrix();
            models[11]->setBillboard(true, BillboardType::SPHERICAL);
        }
        
        
        if (modelLoc != -1) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        }
        models[i]->Render(g_shadingProg);
    }
}
//----------------------------------------------------------------------------

void update(float dt)
{
    glm::mat4 mxView = CCamera::getInstance().getViewMatrix();
    g_light->update(dt);
//    models[8]->update(dt);
    models[9]->setCameraPos(g_eyeloc);
    models[9]->setViewMatrix(mxView);
    models[11]->setCameraPos(g_eyeloc);
    models[11]->setViewMatrix(mxView);
    models[9]->update(dt);
    models[10]->update(dt);
    models[11]->update(dt);
}

void releaseAll()
{
//    g_modelManager.cleanup();
    lightManager.clearLights();
}

int main() {
    // ------- 檢查與建立視窗  ---------------  
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 設定 OpenGL 版本與 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //只啟用 OpenGL 3.3 Core Profile（不包含舊版 OpenGL 功能）
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // 禁止視窗大小改變

    // 建立 OpenGL 視窗與該視窗執行時所需的的狀態、資源和環境(context 上下文)
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL_4 Example 4 NPR", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 設定將這個視窗的資源(OpenGL 的圖形上下文）與當前執行緒綁定，讓該執行緒能夠操作該視窗的資源
    glfwMakeContextCurrent(window);

    // 設定視窗大小, 這樣 OpenGL 才能知道如何將視窗的內容繪製到正確的位置
    // 基本上寬與高設定成視窗的寬與高即可
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    // ---------------------------------------

    // 設定相關事件的 callback 函式，以便事件發生時，能呼叫對應的函式
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);// 視窗大小被改變時
    glfwSetKeyCallback(window, keyCallback);                        // 有鍵盤的按鍵被按下時
    glfwSetMouseButtonCallback(window, mouseButtonCallback);        // 有滑鼠的按鍵被按下時
    glfwSetCursorPosCallback(window, cursorPosCallback);            // 滑鼠在指定的視窗上面移動時
	glfwSetScrollCallback(window, scrollCallback);			        // 滑鼠滾輪滾動時

    // 呼叫 loadScene() 建立與載入 GPU 進行描繪的幾何資料 
    loadScene();


    float lastTime = (float)glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime; // 計算前一個 frame 到目前為止經過的時間
        lastTime = currentTime;
        update(deltaTime);      // 呼叫 update 函式，並將 deltaTime 傳入，讓所有動態物件根據時間更新相關內容
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    releaseAll(); // 程式結束前釋放所有的資源
    glfwTerminate();
    return 0;
}

void adjustShaderEffects(float normalStrength, float specularStrength, float specularPower) {
    glUseProgram(g_shadingProg);
    
    GLint normalStrengthLoc = glGetUniformLocation(g_shadingProg, "uNormalStrength");
    if (normalStrengthLoc != -1) {
        glUniform1f(normalStrengthLoc, normalStrength);
    }
    
    GLint specularStrengthLoc = glGetUniformLocation(g_shadingProg, "uSpecularStrength");
    if (specularStrengthLoc != -1) {
        glUniform1f(specularStrengthLoc, specularStrength);
    }
    
    GLint specularPowerLoc = glGetUniformLocation(g_shadingProg, "uSpecularPower");
    if (specularPowerLoc != -1) {
        glUniform1f(specularPowerLoc, specularPower);
    }
}
