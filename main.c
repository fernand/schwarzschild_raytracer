#include "include/glad/glad.h"
#define GLFW_DLL
#include "include/GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define PI 3.14159265358979323846f

#include "io.c"
#include "math.c"
#include "opengl.c"

#define NX 1920
#define NY 1024
#define TRAIL_LEN 1000
const float skyR2 = 30.0f * 30.0f;

const float potentialCoef = -1.5f;

const float oneRadian = PI / 180.0f;
const float fovy = 45.0f;
const float speed = 0.1f;
const float sensitivity = 0.05f;

float yaw = -90.f, pitch = 0.0f;
//float yaw = -90.f, pitch = -11.0f;
double lastX = NX / 2, lastY = NY / 2;
bool cursorPosSet = false;

v3 cP, cFront, cRight, cUp, wUp;
v3 u, v, w;

static void updateCamera() {
    cFront.x = cosf(pitch * oneRadian) * cosf(yaw * oneRadian);
    cFront.y = sinf(pitch * oneRadian);
    cFront.z = cosf(pitch * oneRadian) * sinf(yaw * oneRadian);
    cFront = normalizeV3(cFront);
    cRight = normalizeV3(crossV3(cFront, wUp));
    cUp = normalizeV3(crossV3(cRight, cFront));
    w = normalizeV3(subtractV3(cP, addV3(cP, cFront)));
    u = normalizeV3(crossV3(cUp, w));
    v = crossV3(w, u);
}

typedef struct {
    float nx;
    float ny;
    float xSkyMap;
    float ySkyMap;
    v3 eye;
    float halfHeight;
    v4 u;
    v4 v;
    v4 w;
} ShaderData;

static ShaderData initShaderData(int nx, int ny, int xSkyMap, int ySkyMap) {
    ShaderData shaderData;
    cP = newV3(0.0f, 0.0f, 20.0f);
    wUp = newV3(0.2f, 1.0f, 0.0f);
    updateCamera();
    shaderData.nx = (float)nx;
    shaderData.ny = (float)ny;
    shaderData.xSkyMap = (float)xSkyMap;
    shaderData.ySkyMap = (float)ySkyMap;
    shaderData.eye = cP;
    shaderData.halfHeight = tanf(fovy * PI / (180.f * 2.0f));
    shaderData.u = fromV3(u);
    shaderData.v = fromV3(v);
    shaderData.w = fromV3(w);
    return shaderData;
}

static void actOnInput(GLFWwindow *window, ShaderData *shaderData) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if (!cursorPosSet) {
        lastX = xpos;
        lastY = ypos;
        cursorPosSet = true;
    }
    double xOffset = xpos - lastX;
    double yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    yaw += sensitivity * (float)xOffset;
    pitch += sensitivity * (float)yOffset;
    if (pitch > 89.0f) {
        pitch = 89.0f;
    } else if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    updateCamera();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cP = addV3(cP, mulV3(speed, cFront));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cP = subtractV3(cP, mulV3(speed, cFront));
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cP = subtractV3(cP, mulV3(speed, cRight));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cP = addV3(cP, mulV3(speed, cRight));
    }

    shaderData->eye = cP;
    shaderData->u = fromV3(u);
    shaderData->v = fromV3(v);
    shaderData->w = fromV3(w);
}

void main() {
    if (!glfwInit()) {
        printf("Could not init GLFW\n");
        exit(-1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(NX, NY, "Sailing", NULL, NULL);
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

    // Create and bind and empty texture
    GLuint outputTextureUnit = 0;
    GLuint outputTextureId;
    glGenTextures(1, &outputTextureId);
    glActiveTexture(GL_TEXTURE0 + outputTextureUnit);
    glBindTexture(GL_TEXTURE_2D, outputTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, NX, NY, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(outputTextureUnit, outputTextureId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    int xSkyMap, ySkyMap, nSkyMap;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *skyMap = stbi_load("data/sky8k.jpg", &xSkyMap, &ySkyMap, &nSkyMap, STBI_rgb_alpha);

    // Create and bind a texture from the skymap image
    GLuint skyMapTextureUnit = 1;
    GLuint skyMapTextureId;
    glGenTextures(1, &skyMapTextureId);
    glActiveTexture(GL_TEXTURE0 + skyMapTextureUnit);
    glBindTexture(GL_TEXTURE_2D, skyMapTextureId);
    // Todo: try to use skyMapTextureId samplers instead of the raw image to remove glistening?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, xSkyMap, ySkyMap, 0, GL_RGBA, GL_UNSIGNED_BYTE, skyMap);
    glBindImageTexture(skyMapTextureUnit, skyMapTextureId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    stbi_image_free(skyMap);

    GLuint computeShaderId = shaderFromSource("rayTracer", GL_COMPUTE_SHADER, "shaders/compute.glsl");
    GLuint computeProgramId = shaderProgramFromShader(computeShaderId);

    ShaderData shaderData = initShaderData(NX, NY, xSkyMap, ySkyMap);

    // Create and bind the SSBO
    GLuint ssboLocation = 0;
    GLuint ssboId;
    glGenBuffers(1, &ssboId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboId);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(shaderData), &shaderData, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssboLocation, ssboId);

    GLuint fboId;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTextureId, 0);

    GLuint vaoId;
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);
    v3 *trailPos = calloc(TRAIL_LEN*sizeof(v3), sizeof(v3));
    v3 *trailView = calloc(TRAIL_LEN*sizeof(v3), sizeof(v3));

    // Find the correct first point depending on the camera.
    v3 laserP = cP;
    v3 laserVelocity = cFront;
    v3 laserCrossed = crossV3(laserP, laserVelocity);
    float laserH2 = dotV3(laserCrossed, laserCrossed);
    float sqrNorm = dotV3(laserP, laserP);
    float coef = 1.0f - 1.0f / sqrtf(sqrNorm);
    trailPos[0] = laserP;
    int trailNumPoints = 1;

    const float f = 1.0f / shaderData.halfHeight;
    const float zFar = 100.0f, zNear = 0.1f;
    const float aspect = (float)NX / NY;
    v3 laserPView = lookAt(cP, u, v, w, laserP);
    trailView[0] = perspective(f, aspect, zNear, zFar, laserPView);

    GLuint vboId;
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, 3*TRAIL_LEN*sizeof(float), trailView, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    GLuint vsShaderId = shaderFromSource("laserVs", GL_VERTEX_SHADER, "shaders/laser.vs");
    GLuint fsShaderId = shaderFromSource("laserFs", GL_FRAGMENT_SHADER, "shaders/laser.fs");
    GLuint laserProgramId = shaderProgramFromShaders(vsShaderId, fsShaderId);

    while(!glfwWindowShouldClose(window)) {
        actOnInput(window, &shaderData);

        if (sqrNorm > 2.6f * 2.6f && sqrNorm < skyR2 && trailNumPoints < TRAIL_LEN) {
            coef = 1.0f - 1.0f / sqrtf(sqrNorm);
            float step = 0.1f * coef;
            laserP = addV3(laserP, mulV3(step, laserVelocity));
            sqrNorm = dotV3(laserP, laserP);
            v3 laserAccel = mulV3(potentialCoef * laserH2 / powf(sqrNorm, 2.5), laserP);
            laserVelocity = addV3(laserVelocity, mulV3(step, laserAccel));
            trailPos[trailNumPoints++] = laserP;
        }

        for (int i=0; i<trailNumPoints; i++) {
            laserPView = lookAt(cP, u, v, w, trailPos[i]);
            trailView[i] = perspective(f, aspect, zNear, zFar, laserPView);
        }

        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(shaderData), &shaderData);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*trailNumPoints, trailView);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(computeProgramId);
        glDispatchCompute(NX/32, NY/32, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBlitFramebuffer(0, 0, NX, NY, 0, 0, NX, NY, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        
        //glUseProgram(laserProgramId);
        //glDrawArrays(GL_LINE_STRIP, 0, trailNumPoints);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
}
