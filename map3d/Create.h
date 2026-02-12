#pragma once
#include <string>
#include <algorithm>
#include <cmath>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

GLuint createSphereVAO(int stacks, int slices, GLuint& indexCount);
GLuint createCylinderVAO(int slices, GLuint& indexCount);
GLuint createOverlayQuad();
GLuint createMapPlaneVAO();
