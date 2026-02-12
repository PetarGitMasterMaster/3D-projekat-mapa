#include "Input3D.h"
#include "Globals.h"

std::vector<MeasurePoint> measurePoints;
float measureDistance = 0.0f;

extern glm::vec3 cameraPos;

extern Mode mode;

MeasurePoint::MeasurePoint(float _x, float _z)
    : x(_x), z(_z) {
}

bool screenToWorldOnMap(
    double mouseX, double mouseY,
    int screenW, int screenH,
    const glm::mat4& view,
    const glm::mat4& projection,
    glm::vec3& outWorld
) {
    
    float x = (2.0f * (float)mouseX) / screenW - 1.0f;
    float y = 1.0f - (2.0f * (float)mouseY) / screenH;
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    
    glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));

    
    if (fabs(rayWorld.y) < 0.0001f) return false;

    float t = -cameraPos.y / rayWorld.y;
    if (t < 0.0f) return false;

    outWorld = cameraPos + rayWorld * t;

   
    if (outWorld.x < -MAP_WIDTH / 2.0f || outWorld.x > MAP_WIDTH / 2.0f ||
        outWorld.z < -MAP_HEIGHT / 2.0f || outWorld.z > MAP_HEIGHT / 2.0f)
        return false;

    return true;
}


int findPointIndexNear3D(const glm::vec3& p, float radius)
{
    for (int i = 0; i < (int)measurePoints.size(); i++) {
        float dx = measurePoints[i].x - p.x;
        float dz = measurePoints[i].z - p.z;
        if (std::sqrt(dx * dx + dz * dz) < radius)
            return i;
    }
    return -1;
}

void addMeasurePoint3D(const MeasurePoint& p)
{
    if (!measurePoints.empty()) {
        MeasurePoint last = measurePoints.back();
        float dx = p.x - last.x;
        float dz = p.z - last.z;
        measureDistance += std::sqrt(dx * dx + dz * dz) * mapScaleFactor;

    }
    measurePoints.push_back(p);
}

void removeMeasurePoint3D(int index)
{
    if (index < 0 || index >= (int)measurePoints.size()) return;

    if (index > 0) {
        MeasurePoint a = measurePoints[index - 1];
        MeasurePoint b = measurePoints[index];
        float dx = b.x - a.x;
        float dz = b.z - a.z;
        measureDistance -= std::sqrt(dx * dx + dz * dz) * mapScaleFactor;;
    }

    if (index < (int)measurePoints.size() - 1) {
        MeasurePoint a = measurePoints[index];
        MeasurePoint b = measurePoints[index + 1];
        float dx = b.x - a.x;
        float dz = b.z - a.z;
        measureDistance -= std::sqrt(dx * dx + dz * dz) * mapScaleFactor;;
    }

    if (index > 0 && index < (int)measurePoints.size() - 1) {
        MeasurePoint a = measurePoints[index - 1];
        MeasurePoint b = measurePoints[index + 1];
        float dx = b.x - a.x;
        float dz = b.z - a.z;
        measureDistance += std::sqrt(dx * dx + dz * dz) * mapScaleFactor;;
    }

    measurePoints.erase(measurePoints.begin() + index);
}

void processMouse3D(
    GLFWwindow* window,
    int screenW, int screenH,
    const glm::mat4& view,
    const glm::mat4& projection
) {
    static bool leftWasDown = false;
    bool leftNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (leftNow && !leftWasDown && mode == MODE_MEASURE) {
        double mx = screenW * 0.5;
        double my= screenH *0.5;
        

        glm::vec3 hit;
        if (screenToWorldOnMap(mx, my, screenW, screenH, view, projection, hit)) {

            int idx = findPointIndexNear3D(hit, 1.0f);
            if (idx != -1) {
                removeMeasurePoint3D(idx);
            }
            else {
                addMeasurePoint3D(MeasurePoint(hit.x, hit.z));
            }
        }
    }

    leftWasDown = leftNow;
}
