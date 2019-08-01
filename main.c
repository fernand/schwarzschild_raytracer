#include "include/glad/glad.h"
#define GLFW_DLL
#include "include/GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define M_PI 3.14159265358979323846f

typedef unsigned char u8;

#include "io.c"
#include "math.c"
#include "opengl.c"

typedef struct {
    float nx;
    float ny;
    float xSkyMap;
    float ySkyMap;
    vec3 eye;
    float tanFov;
    mat4 lookAt;
} ShaderData;

const int nx = 1920, ny = 1024;
float lastX = 1920 / 2, lastY = 1024 / 2;
vec3 cP, cFront, cRight, cUp, wUp;
bool cursorPosSet = false;
float yaw = -90.f, pitch = 0.f;
const float oneRadian = M_PI / 180.0f;

static void updateCamera() {
    cFront.X = cosf(pitch * oneRadian) * cosf(yaw * oneRadian);
    cFront.Y = sinf(pitch * oneRadian);
    cFront.Z = cosf(pitch * oneRadian) * sinf(yaw * oneRadian);
    cFront = normalizeVec3(cFront);
    cRight = normalizeVec3(crossVec3(cFront, wUp));
    cUp = normalizeVec3(crossVec3(cRight, cFront));
}

static setupShaderData(int nx, int ny, int xSkyMap, int ySkyMap, ShaderData *shaderData) {
    cP = newVec3(0.0f, 1.0f, 20.0f);
    wUp = newVec3(0.2f, 1.0f, 0.0f);
    updateCamera();
    shaderData->nx = (float)nx;
    shaderData->ny = (float)ny;
    shaderData->xSkyMap = (float)xSkyMap;
    shaderData->ySkyMap = (float)ySkyMap;
    shaderData->eye = cP;
    shaderData->tanFov = tan(oneRadian) * 90.f;
    shaderData->lookAt = getLookAt(cP, addVec3(cP, cFront), cUp);
}

static actOnInput(GLFWwindow *window, ShaderData *shaderData) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if (!cursorPosSet) {
        lastX = xpos;
        lastY = ypos;
        cursorPosSet = true;
    }
    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    yaw += 0.05f * xOffset;
    pitch += 0.05f * yOffset;
    if (pitch > 89.0f) {
        pitch = 89.0f;
    } else if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    updateCamera();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cP = addVec3(cP, mulVec3(0.1f, cFront));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cP = subtractVec3(cP, mulVec3(0.1f, cFront));
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cP = subtractVec3(cP, mulVec3(0.1f, cRight));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cP = addVec3(cP, mulVec3(0.1f, cRight));
    }

    shaderData->lookAt = getLookAt(cP, addVec3(cP, cFront), cUp);
    shaderData->eye = cP;
}

main() {
    if (!glfwInit()) {
        printf("Could not init GLFW\n");
        exit(-1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(nx, ny, "Ray GL", NULL, NULL);
    if (!window) {
        printf("Could not init GLFW window\n");
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        printf("Could not init OpenGL context\n");
        exit(-1);
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    } else {
        printf("raw mouse motion not supported");
        exit(-1);
    }

    ShaderData shaderData;
    GLuint outputTextureId = createAndBindEmptyTexture(0, nx, ny);

    int xSkyMap, ySkyMap, nSkyMap;
    stbi_set_flip_vertically_on_load(true);
    u8 *skyMap = stbi_load("data/sky8k.jpg", &xSkyMap, &ySkyMap, &nSkyMap, STBI_rgb_alpha);
    GLuint skyMapTextureId = createAndBindTextureFromImage(1, xSkyMap, ySkyMap, skyMap);
    stbi_image_free(skyMap);

    GLuint shaderId = shaderFromSource("rayTracer", "shaders/compute.glsl");
    GLuint programId = shaderProgramFromShader(shaderId);
    glUseProgram(programId);

    setupShaderData(nx, ny, xSkyMap, ySkyMap, &shaderData);
    GLuint ssbo = createAndBindSSBO(0, sizeof(shaderData), &shaderData);

    GLuint fboId = 0;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTextureId, 0);

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDispatchCompute(nx/32, ny/32, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBlitFramebuffer(0, 0, nx, ny, 0, 0, nx, ny, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glfwSwapBuffers(window);

        glfwPollEvents();
        actOnInput(window, &shaderData);

        updateSSBO(ssbo, sizeof(shaderData), &shaderData);
    }

    glDeleteFramebuffers(1, &fboId);
    glDeleteTextures(1, &outputTextureId);
    glDeleteTextures(1, &skyMapTextureId);
    glDeleteShader(shaderId);
    glDeleteProgram(programId);
    glfwTerminate();
}
