
#include <iostream>
#include <string>
#include "Util.h"
#include "Input3D.h"
#include "Globals.h"
#include "Create.h"
#include "shader.hpp"
#include "Model.hpp"
#include <chrono>
#include <thread>

const double TARGET_FPS = 75.0;
const double FRAME_TIME = 1.0 / TARGET_FPS;
auto lastFrame = std::chrono::high_resolution_clock::now();

GLuint digitsTex = 0, iconWalkTex = 0, iconMeasureTex = 0;
GLuint overlayShader = 0;
GLuint overlayVAO = 0;
GLint loc_ov_uTex = -1;
GLint loc_ov_uPos = -1;
GLint loc_ov_uSize = -1;
GLint loc_ov_uAlpha = -1;
GLint loc_ov_uUVScale = -1;
GLint loc_ov_uUVOffset = -1;

double totalDistance = 0.0;
GLuint sphereVAO = 0;
GLuint sphereIndexCount = 0;

GLuint cylinderVAO = 0;
GLuint cylinderIndexCount = 0;
GLuint lineVAO = 0, lineVBO = 0;
GLuint crosshairVAO = 0, crosshairVBO = 0;
glm::vec3 lastMoveDir = glm::vec3(0, 0, 1);
float humanoidYaw = 0.0f;


void initCrosshair()
{
    float size = 0.02f;

    float verts[] = {
        
        -size, 0.0f, 0.0f,
         size, 0.0f, 0.0f,

          0.0f, -size, 0.0f,
          0.0f,  size, 0.0f
    };

    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}
void initMeasureLines()
{
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}
void overlayResetUV()
{
    glUniform2f(loc_ov_uUVScale, 1.0f, 1.0f);
    glUniform2f(loc_ov_uUVOffset, 0.0f, 0.0f);
}

// Map camera

static const float WALK_Y = 8.0f;     
static const float MEASURE_Y = 30.0f;    
static const float BORDER_MARGIN = 1.0f; 

Mode mode = MODE_WALK;

// Camera
bool firstMouse = true;
float lastX = 500.0f, lastY = 500.0f;

float yaw = -90.0f;
float pitch = -30.0f; 
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0);
glm::vec3 cameraPos = glm::vec3(0.0f, WALK_Y, 15.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float fov = 45.0f;

float cameraSpeed = 0.3f;

glm::vec3 humPos = glm::vec3(0.0f, 0.5f, 0.0f); 
float humSpeed = 0.30f;

static bool rWasPressed = false;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.10f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    
    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)  fov = 1.0f;
    if (fov > 45.0f) fov = 45.0f;
}

void handleModeSwitch(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!rWasPressed) {
            mode = (mode == MODE_WALK) ? MODE_MEASURE : MODE_WALK;
            cameraPos.y = (mode == MODE_WALK) ? WALK_Y : MEASURE_Y;
        }
        rWasPressed = true;
    }
    else {
        rWasPressed = false;
    }
}

void moveCameraArrows(GLFWwindow* window)
{
    glm::vec3 forwardXZ = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));

    if (glm::length(forwardXZ) < 1e-6f) forwardXZ = glm::vec3(0, 0, -1);

    glm::vec3 rightXZ = glm::normalize(glm::cross(forwardXZ, cameraUp));

    glm::vec3 oldPos = cameraPos;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cameraPos += cameraSpeed * forwardXZ;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cameraPos -= cameraSpeed * forwardXZ;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cameraPos -= cameraSpeed * rightXZ;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cameraPos += cameraSpeed * rightXZ;

    float minX = -MAP_WIDTH / 2 + BORDER_MARGIN;
    float maxX = MAP_WIDTH / 2 - BORDER_MARGIN;
    float minZ = -MAP_HEIGHT / 2 + BORDER_MARGIN;
    float maxZ = MAP_HEIGHT / 2 - BORDER_MARGIN;

    if (cameraPos.x < minX) cameraPos.x = minX;
    if (cameraPos.x > maxX) cameraPos.x = maxX;

    if (cameraPos.z < minZ) cameraPos.z = minZ;
    if (cameraPos.z > maxZ) cameraPos.z = maxZ;

    cameraPos.y = oldPos.y;
}

void moveHumWASD(GLFWwindow* window)
{
    if (mode != MODE_WALK) return;

    glm::vec3 oldPos = humPos;

    glm::vec3 forward = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));

    if (glm::length(forward) < 1e-6f)
        forward = glm::vec3(0, 0, -1);
 
    glm::vec3 right = glm::normalize(glm::cross(forward, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        humPos += humSpeed * forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        humPos -= humSpeed * forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        humPos -= humSpeed * right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        humPos += humSpeed * right;
    glm::vec3 move = humPos - oldPos;
    move.y = 0.0f;

    if (glm::length(move) > 1e-6f) {
        lastMoveDir = glm::normalize(move);
        humanoidYaw = atan2(lastMoveDir.x, lastMoveDir.z);
    }
 
    float minX = -MAP_WIDTH / 2 + 1.0f;
    float maxX = MAP_WIDTH / 2 - 1.0f;
    float minZ = -MAP_HEIGHT / 2 + 1.0f;
    float maxZ = MAP_HEIGHT / 2 - 1.0f;

    humPos.x = std::max(minX, std::min(humPos.x, maxX));
    humPos.z = std::max(minZ, std::min(humPos.z, maxZ));

   
    float dx = humPos.x - oldPos.x;
    float dz = humPos.z - oldPos.z;

    double segment = std::sqrt(dx * dx + dz * dz);
    totalDistance += segment * mapScaleFactor;
}

void drawMeasureLines(const glm::mat4& vp, GLuint colorShader)
{
    if (measurePoints.size() < 2) return;

    std::vector<float> verts;
    verts.reserve(measurePoints.size() * 3);

    for (auto& p : measurePoints) {
        verts.push_back(p.x);
        verts.push_back(0.02f); 
        verts.push_back(p.z);
    }

    glUseProgram(colorShader);

    glUniformMatrix4fv(glGetUniformLocation(colorShader, "uMVP"),1, GL_FALSE, glm::value_ptr(vp));

    
    glUniform3f(glGetUniformLocation(colorShader, "uColor"),0.0f, 1.0f, 0.0f);

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER,verts.size() * sizeof(float),verts.data(),GL_DYNAMIC_DRAW);

    glLineWidth(10.0f);
    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)measurePoints.size());

    glBindVertexArray(0);
}
void drawNeedle(const glm::mat4& view,const glm::mat4& projection,Shader& phongShader,float x,float z) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.3f, z));
    model = glm::scale(model, glm::vec3(0.12f, 1.6f, 0.12f));

    phongShader.use();

    phongShader.setMat4("uM", model);
    phongShader.setMat4("uV", view);
    phongShader.setMat4("uP", projection);

    phongShader.setVec3("uMaterial.kA", 0.2f, 0.2f, 0.2f);
    phongShader.setVec3("uMaterial.kD", 0.6f, 0.6f, 0.6f);
    phongShader.setVec3("uMaterial.kS", 0.9f, 0.9f, 0.9f);
    phongShader.setFloat("uMaterial.shine", 128.0f);

    glBindVertexArray(cylinderVAO);
    glDrawElements(GL_TRIANGLES, cylinderIndexCount, GL_UNSIGNED_INT, 0);
}
void drawPinHead(const glm::mat4& view,const glm::mat4& projection,Shader& phongShader,float x,float z) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, 2.05f, z));
    model = glm::scale(model, glm::vec3(0.25f));

    phongShader.use();

    phongShader.setMat4("uM", model);
    phongShader.setMat4("uV", view);
    phongShader.setMat4("uP", projection);

    // Red
    phongShader.setVec3("uMaterial.kA", 0.3f, 0.0f, 0.0f);
    phongShader.setVec3("uMaterial.kD", 0.9f, 0.1f, 0.1f);
    phongShader.setVec3("uMaterial.kS", 1.0f, 1.0f, 1.0f);
    phongShader.setFloat("uMaterial.shine", 64.0f);

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
}

void drawPins(const glm::mat4& view,const glm::mat4& projection,Shader& phongShader) {
    for (auto& p : measurePoints) {
        drawNeedle(view, projection, phongShader, p.x, p.z);
        drawPinHead(view, projection, phongShader, p.x, p.z);
    }
}


void drawModeIcon2D(GLuint tex, int screenW, int screenH)
{
    glUseProgram(overlayShader);
    overlayResetUV();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(loc_ov_uTex, 0);

    float aspect = (float)screenW / (float)screenH;
    float iconSize = 0.25f;
    float posX = 0.85f;
    float posY = 0.85f;

    glUniform2f(loc_ov_uSize, (iconSize) * 0.5f * aspect * 0.5f, iconSize * 0.5f * aspect * 0.5f);
    glUniform2f(loc_ov_uPos, posX, posY);
    glUniform1f(loc_ov_uAlpha, 1.0f);

    glBindVertexArray(overlayVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawDistanceNumbers2D(int screenW, int screenH, int distInt)
{
    glUseProgram(overlayShader);
	overlayResetUV();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, digitsTex);
    glUniform1i(loc_ov_uTex, 0);
    glUniform1f(loc_ov_uAlpha, 1.0f);

    std::string s = std::to_string(distInt);

    float aspect = (float)screenW / (float)screenH;
    float charHeight = 0.1f;
    float charWidth = charHeight * aspect * 0.5f;

    float startX = -0.9f;
    float startY = -0.8f;

    glUniform2f(loc_ov_uUVScale, 0.1f, 1.0f);

    for (int i = 0; i < (int)s.length(); ++i) {
        int digit = s[i] - '0';
        if (digit < 0 || digit > 9) continue;

        glUniform2f(loc_ov_uUVOffset, digit * 0.1f, 0.0f);

        glUniform2f(loc_ov_uSize, charWidth * 0.4f, charHeight * 0.5f);
        glUniform2f(loc_ov_uPos, startX + (i * charWidth), startY);

        glBindVertexArray(overlayVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}
void drawCrosshair(const glm::mat4& projection, const glm::mat4& view, GLuint colorShader)
{
    glUseProgram(colorShader);
    glm::mat4 mvp = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(colorShader, "uMVP"),1, GL_FALSE,glm::value_ptr(mvp));
    glUniform3f(glGetUniformLocation(colorShader, "uColor"),1.0f, 0.0f, 1.0f);

    glBindVertexArray(crosshairVAO);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 4);
    glBindVertexArray(0);
}

static void setLight(Shader& sh, int i,
    const glm::vec3& pos,
    const glm::vec3& kA,
    const glm::vec3& kD,
    const glm::vec3& kS)
{
    std::string base = "uLights[" + std::to_string(i) + "]";
    sh.setVec3(base + ".pos", pos);
    sh.setVec3(base + ".kA", kA);
    sh.setVec3(base + ".kD", kD);
    sh.setVec3(base + ".kS", kS);
}

int main()
{

    if (!glfwInit()) {
        std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
   // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* screen = glfwGetVideoMode(monitor);

    int wWidth = screen->width;
    int wHeight = screen->height;

    GLFWwindow* window = glfwCreateWindow(wWidth,wHeight,"3D Map",monitor, NULL);


    if (!window) {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        system("pause");
        return -1;
    }



    glfwMakeContextCurrent(window);
    glViewport(0, 0, wWidth, wHeight);

    if (!window) {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        glfwTerminate();
        return 3;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.12f, 0.14f, 0.17f, 1.0f);

    //Shader/Texture
    
    GLuint mapShader = createShader("map3d.vert", "map3d.frag");
    GLuint colorShader = createShader("map3d.vert", "fragColor.frag");
    Shader phongShader("phong.vert", "phong.frag");
	overlayShader = createShader("overlay.vert", "overlay.frag");
    glUseProgram(overlayShader);
    loc_ov_uUVScale = glGetUniformLocation(overlayShader, "uUVScale");
    loc_ov_uUVOffset = glGetUniformLocation(overlayShader, "uUVOffset");

    glUseProgram(mapShader);
    glUniform1i(glGetUniformLocation(mapShader, "uTexture"), 0);

    loc_ov_uTex = glGetUniformLocation(overlayShader, "uTexture");
    loc_ov_uPos = glGetUniformLocation(overlayShader, "uPos");
    loc_ov_uSize = glGetUniformLocation(overlayShader, "uSize");
    loc_ov_uAlpha = glGetUniformLocation(overlayShader, "uAlpha");
    loc_ov_uUVScale = glGetUniformLocation(overlayShader, "uUVScale");
    loc_ov_uUVOffset = glGetUniformLocation(overlayShader, "uUVOffset");


    Model humanoid("res/Terminator.obj");
    digitsTex = preprocessTexture("res/digits.png");
    iconWalkTex = preprocessTexture("res/walk_icon.png");
    iconMeasureTex = preprocessTexture("res/ruler.png");
    GLuint mapTex = preprocessTexture("res/map_angeles.jpg");
    
    GLuint mapVAO = createMapPlaneVAO();
	overlayVAO = createOverlayQuad();
    sphereVAO = createSphereVAO(16, 16, sphereIndexCount);
    cylinderVAO = createCylinderVAO(20, cylinderIndexCount);

    initMeasureLines();
    initCrosshair();
 
    cameraPos.y = WALK_Y;

    //Rendering
    while (!glfwWindowShouldClose(window))
    {
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        handleModeSwitch(window);
        moveCameraArrows(window);
        moveHumWASD(window);

        // Camera/Matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(
            glm::radians(fov),
            (float)wWidth / (float)wHeight,
            0.1f,
            500.0f
        );

        // Lighting
        phongShader.use();
        phongShader.setVec3("uViewPos", cameraPos);
        int lightCount = 0;
        setLight(phongShader, 0,
            glm::vec3(0, 40, 0),
            glm::vec3(0.5f),
            glm::vec3(1.5f),
            glm::vec3(2.0f));
        lightCount = 1;

        if (mode == MODE_MEASURE)
        {
            for (auto& p : measurePoints)
            {
                if (lightCount >= 16) break;
                setLight(phongShader, lightCount,
                    glm::vec3(p.x, 2.05f, p.z),
                    glm::vec3(0.10f, 0.0f, 0.0f),
                    glm::vec3(0.60f, 0.0f, 0.0f),
                    glm::vec3(0.10f, 0.05f, 0.05f)
                );
                lightCount++;
            }
        }

        phongShader.setInt("uLightCount", lightCount);

 
        processMouse3D(window, wWidth, wHeight, view, projection);

        //Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Draw map
        phongShader.use();
        phongShader.setMat4("uM", glm::mat4(1.0f));
        phongShader.setMat4("uV", view);
        phongShader.setMat4("uP", projection);

        phongShader.setVec3("uMaterial.kA", 0.2f, 0.2f, 0.2f);
        phongShader.setVec3("uMaterial.kD", 1.0f, 1.0f, 1.0f);
        phongShader.setVec3("uMaterial.kS", 0.05f, 0.05f, 0.05f);
        phongShader.setFloat("uMaterial.shine", 8.0f);

        phongShader.setInt("uUseTexture", 1);
        phongShader.setInt("uDiffMap", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mapTex);

        glBindVertexArray(mapVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        glm::mat4 vp = projection * view;

        if (mode == MODE_MEASURE)
        {
            drawMeasureLines(vp, colorShader);
            drawPins(view, projection, phongShader);
        }

        if (mode == MODE_WALK)
        {
            phongShader.use();
            phongShader.setMat4("uV", view);
            phongShader.setMat4("uP", projection);
            phongShader.setVec3("uViewPos", cameraPos);

            glm::mat4 M = glm::mat4(1.0f);
            M = glm::translate(M, humPos);
            M = glm::rotate(M, humanoidYaw, glm::vec3(0, 1, 0));
            M = glm::scale(M, glm::vec3(0.15f));

            phongShader.setMat4("uM", M);

            phongShader.setVec3("uMaterial.kA", 0.2f, 0.2f, 0.2f);
            phongShader.setVec3("uMaterial.kD", 0.8f, 0.8f, 0.8f);
            phongShader.setVec3("uMaterial.kS", 0.6f, 0.6f, 0.6f);
            phongShader.setFloat("uMaterial.shine", 32.0f);

            humanoid.Draw(phongShader);
        }


        drawCrosshair(projection, view, colorShader);
        drawModeIcon2D(
            mode == MODE_WALK ? iconWalkTex : iconMeasureTex,
            wWidth, wHeight
        );

        int distInt = (mode == MODE_WALK)
            ? (int)totalDistance
            : (int)measureDistance;

        drawDistanceNumbers2D(wWidth, wHeight, distInt);


        glfwSwapBuffers(window);
        glfwPollEvents();

        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now - lastFrame;
        if (elapsed.count() < FRAME_TIME)
        {
            std::this_thread::sleep_for(
                std::chrono::duration<double>(FRAME_TIME - elapsed.count())
            );
        }
        lastFrame = std::chrono::high_resolution_clock::now();
    }

 
    glDeleteVertexArrays(1, &mapVAO);
    glDeleteVertexArrays(1, &overlayVAO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteVertexArrays(1, &cylinderVAO);
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteVertexArrays(1, &crosshairVAO);

    glDeleteBuffers(1, &lineVBO);
    glDeleteBuffers(1, &crosshairVBO);
    
    glDeleteTextures(1, &mapTex);
    glDeleteTextures(1, &digitsTex);
    glDeleteTextures(1, &iconWalkTex);
    glDeleteTextures(1, &iconMeasureTex);
    
    glDeleteProgram(mapShader);
    glDeleteProgram(colorShader);
    glDeleteProgram(overlayShader);

  
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
