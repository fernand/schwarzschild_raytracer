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
    float eyeAndTanFov[4];
    mat4 lookAt;
} ShaderData;

static setupShaderData(int nx, int ny, int xSkyMap, int ySkyMap,
        vec3 eye,
        ShaderData *shader_data) {
    vec3 center = newVec3(0.0f, 0.0f, 0.0f);
    vec3 up = newVec3(-0.3f, 1.0f, 0.0f);
    mat4 lookAt = getLookAt(eye, center, up);
    shader_data->nx = (float)nx;
    shader_data->ny = (float)ny;
    shader_data->xSkyMap = (float)xSkyMap;
    shader_data->ySkyMap = (float)ySkyMap;
    for (int i=0; i<3; i++) {
        shader_data->eyeAndTanFov[i] = eye.E[i];
        for(int j=0; j<3; j++) {
            shader_data->lookAt.E[i][j] = lookAt.E[i][j];
        }
        shader_data->lookAt.E[i][3] = 0.0f;
    }
    for (int i=3; i<4; i++) {
        shader_data->eyeAndTanFov[i] = tan(M_PI / 180.0f) * 55.0f;
        for (int j=0; j<4; j++) {
            shader_data->lookAt.E[i][j] = 0.0f;
        }
    }
}

static double prevCursorX, prevCursorY, cursorX, cursorY;
static bool cursorPosSet = false;
 
static actOnInput(GLFWwindow *window, ShaderData *shaderData) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if (!cursorPosSet) {
        prevCursorX = xpos;
        prevCursorY = ypos;
        cursorX = xpos;
        cursorY = ypos;
        cursorPosSet = true;
    } else {
        prevCursorX = cursorX;
        prevCursorY = cursorY;
        cursorX = xpos;
        cursorY = ypos;
    }
    float dAlpha = 0.05f * -(cursorX - prevCursorX) * M_PI / 180.0f; // yaw
    float dBeta = 0.05f * (cursorY - prevCursorY) * M_PI / 180.0f; // pitch
    mat4 rot = rotationMatrix(dAlpha, dBeta);
    mat4 rotatedLookAt = multiplyMatrix(rot, shaderData->lookAt);
    memcpy(&shaderData->lookAt, &rotatedLookAt, sizeof(rotatedLookAt));

    int state = glfwGetKey(window, GLFW_KEY_W);
    if (state == GLFW_PRESS) {
        vec3 v = transform(shaderData->lookAt, newVec3(0.f, 0.f, 0.1f));
        vec3 eye = newVec3(shaderData->eyeAndTanFov[0], shaderData->eyeAndTanFov[1], shaderData->eyeAndTanFov[2]);
        eye = addVec3(eye, v);
        memcpy(&shaderData->eyeAndTanFov, &eye, sizeof(eye));
    }
#if 0
    state = glfwGetKey(window, GLFW_KEY_S);
    if (state == GLFW_PRESS)
        shaderData->eyeAndTanFov[2] -= 0.1f;
    state = glfwGetKey(window, GLFW_KEY_A);
    if (state == GLFW_PRESS)
        shaderData->eyeAndTanFov[0] -= 0.1f;
    state = glfwGetKey(window, GLFW_KEY_D);
    if (state == GLFW_PRESS)
        shaderData->eyeAndTanFov[0] += 0.1f;
#endif
}

main() {
    const int nx = 1920;
    const int ny = 1024;

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
    u8 *skyMap = stbi_load("data/sky8k.jpg", &xSkyMap, &ySkyMap, &nSkyMap, STBI_rgb_alpha);
    GLuint skyMapTextureId = createAndBindTextureFromImage(1, xSkyMap, ySkyMap, skyMap);
    stbi_image_free(skyMap);

    GLuint shaderId = shaderFromSource("rayTracer", "shaders/compute.glsl");
    GLuint programId = shaderProgramFromShader(shaderId);
    glUseProgram(programId);

    vec3 eye = newVec3(0.0f, 0.0f, -20.0f);
    setupShaderData(nx, ny, xSkyMap, ySkyMap, eye, &shaderData);
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
