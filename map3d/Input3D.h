#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <cmath>
#include <vector>

enum Mode { MODE_WALK, MODE_MEASURE };

struct MeasurePoint {
    float x;
    float z;
    MeasurePoint(float _x, float _z);
};

extern std::vector<MeasurePoint> measurePoints;
extern float measureDistance;

bool screenToWorldOnMap(
    double mouseX, double mouseY,
    int screenW, int screenH,
    const glm::mat4& view,
    const glm::mat4& projection,
    glm::vec3& outWorld
);

void processMouse3D(
    GLFWwindow* window,
    int screenW, int screenH,
    const glm::mat4& view,
    const glm::mat4& projection
);
