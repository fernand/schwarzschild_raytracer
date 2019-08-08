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

const float oneRadian = PI / 180.0f;
const float fov = 75.0f;
const float speed = 0.1f;
const float sensitivity = 0.05f;

float yaw = -90.f, pitch = 0.f;
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
    float halfWidth;
    v4 u;
    v4 v;
    v4 w;
} ShaderData;

static void setupShaderData(int nx, int ny, int xSkyMap, int ySkyMap, ShaderData *shaderData) {
    cP = newV3(-2.0f, 1.0f, 20.0f);
    wUp = newV3(0.2f, 1.0f, 0.0f);
    updateCamera();
    shaderData->nx = (float)nx;
    shaderData->ny = (float)ny;
    shaderData->xSkyMap = (float)xSkyMap;
    shaderData->ySkyMap = (float)ySkyMap;
    shaderData->eye = cP;
    shaderData->halfWidth = tanf(fov * PI / (180.f * 2.0f));
    shaderData->u = fromV3(u);
    shaderData->v = fromV3(v);
    shaderData->w = fromV3(w);
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
    GLFWwindow *window = glfwCreateWindow(NX, NY, "Ray GL", NULL, NULL);
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
    GLuint outputTextureId = createAndBindEmptyTexture(0, NX, NY);

    int xSkyMap, ySkyMap, nSkyMap;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *skyMap = stbi_load("data/sky8k.jpg", &xSkyMap, &ySkyMap, &nSkyMap, STBI_rgb_alpha);
    GLuint skyMapTextureId = createAndBindTextureFromImage(1, xSkyMap, ySkyMap, skyMap);
    stbi_image_free(skyMap);

    GLuint shaderId = shaderFromSource("rayTracer", GL_COMPUTE_SHADER, "shaders/compute.glsl");
    GLuint programId = shaderProgramFromShader(shaderId);

    setupShaderData(NX, NY, xSkyMap, ySkyMap, &shaderData);
    GLuint ssbo = createAndBindSSBO(0, sizeof(shaderData), &shaderData);

    GLuint fboId = 0;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTextureId, 0);

    GLuint vertexArrayId;
    glGenVertexArrays(1, &vertexArrayId);
    glBindVertexArray(vertexArrayId);
    GLfloat lineData[] = {
        -1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f
    };
    GLuint vboId;
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineData), lineData, GL_DYNAMIC_DRAW);

    GLuint vsShaderId = shaderFromSource("laserVs", GL_VERTEX_SHADER, "shaders/laser.vs");
    GLuint fsShaderId = shaderFromSource("laserFs", GL_FRAGMENT_SHADER, "shaders/laser.fs");
    GLuint laserProgramId = shaderProgramFromShaders(vsShaderId, fsShaderId);

    GLenum err = glGetError();
    GLenum noErr = GL_NO_ERROR;
    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        err = glGetError();
        glUseProgram(programId);
        glDispatchCompute(NX/32, NY/32, 1);
        err = glGetError();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        err = glGetError();
        glBlitFramebuffer(0, 0, NX, NY, 0, 0, NX, NY, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        err = glGetError();
        
        glUseProgram(laserProgramId);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glDrawArrays(GL_LINE_STRIP, 0, 2);
        glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);

        glfwPollEvents();
        //actOnInput(window, &shaderData);

        // No need to rebind the shader storage buffer since it's the only once we use.
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(shaderData), &shaderData);
        err = glGetError();
    }

    glDeleteFramebuffers(1, &fboId);
    glDeleteTextures(1, &outputTextureId);
    glDeleteTextures(1, &skyMapTextureId);
    glDeleteShader(shaderId);
    glDeleteProgram(programId);
    glDeleteShader(vsShaderId);
    glDeleteShader(fsShaderId);
    glDeleteProgram(laserProgramId);
    glfwTerminate();
}
