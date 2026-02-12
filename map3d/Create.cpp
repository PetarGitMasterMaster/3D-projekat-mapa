#include "Create.h"
#include "Globals.h"

double M_PI = 3.14159265358979323846;
GLuint createSphereVAO(int stacks, int slices, GLuint& indexCount)
{
    std::vector<float> verts;
    std::vector<unsigned int> idx;

    for (int i = 0; i <= stacks; i++) {
        float v = (float)i / stacks;
        float theta = v * M_PI;

        for (int j = 0; j <= slices; j++) {
            float u = (float)j / slices;
            float phi = u * 2.0f * M_PI;

            float x = sin(theta) * cos(phi);
            float y = cos(theta);
            float z = sin(theta) * sin(phi);

            
            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);

           
            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);
        }
    }

    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < slices; j++) {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            idx.push_back(first);
            idx.push_back(second);
            idx.push_back(first + 1);

            idx.push_back(second);
            idx.push_back(second + 1);
            idx.push_back(first + 1);
        }
    }

    indexCount = (GLuint)idx.size();

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);

    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

   
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
        (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    return vao;
}

GLuint createCylinderVAO(int slices, GLuint& indexCount)
{
    std::vector<float> verts;
    std::vector<unsigned int> idx;

    for (int i = 0; i <= slices; i++) {
        float a = 2.0f * M_PI * i / slices;
        float x = cos(a);
        float z = sin(a);

        verts.push_back(x);
        verts.push_back(0.0f);
        verts.push_back(z);


        verts.push_back(x);
        verts.push_back(0.0f);
        verts.push_back(z);


        verts.push_back(x);
        verts.push_back(1.0f);
        verts.push_back(z);


        verts.push_back(x);
        verts.push_back(0.0f);
        verts.push_back(z);
    }

    for (int i = 0; i < slices; i++) {
        int k = i * 2;
        idx.push_back(k);
        idx.push_back(k + 1);
        idx.push_back(k + 2);

        idx.push_back(k + 1);
        idx.push_back(k + 3);
        idx.push_back(k + 2);
    }

    indexCount = (GLuint)idx.size();

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
        (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    return vao;
}

GLuint createOverlayQuad()
{
    float quad[] = {
        -0.5f, -0.5f,  0, 0,
         0.5f, -0.5f,  1, 0,
         0.5f,  0.5f,  1, 1,
        -0.5f,  0.5f,  0, 1
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return vao;
}
struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

GLuint createMapPlaneVAO()
{
    Vertex vertices[] = {
        { {-MAP_WIDTH / 2, 0.0f, -MAP_HEIGHT / 2}, {0,1,0}, {0,0} },
        { { MAP_WIDTH / 2, 0.0f, -MAP_HEIGHT / 2}, {0,1,0}, {1,0} },
        { { MAP_WIDTH / 2, 0.0f,  MAP_HEIGHT / 2}, {0,1,0}, {1,1} },
        { {-MAP_WIDTH / 2, 0.0f,  MAP_HEIGHT / 2}, {0,1,0}, {0,1} },
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);


    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));


    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    glBindVertexArray(0);
    return vao;
}
