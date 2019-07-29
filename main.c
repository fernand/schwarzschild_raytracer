#include "include/glad/glad.h"
#define GLFW_DLL
#include "include/GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#include <stdio.h>
#include <stdlib.h>
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
    float lookAt[4][4];
} ShaderData;

ShaderData setupData(int nx, int ny, int xSkyMap, int ySkyMap) {
    hmm_vec3 eye = HMM_Vec3(0, 0, -20);
    hmm_vec3 center = HMM_Vec3(0, 0, 0);
    hmm_vec3 up = HMM_Vec3(-0.3, 1, 0);
    mat3 lookAt = getLookAt(eye, center, up);
    ShaderData shader_data;
    shader_data.nx = (float)nx;
    shader_data.ny = (float)ny;
    shader_data.xSkyMap = (float)xSkyMap;
    shader_data.ySkyMap = (float)ySkyMap;
    for (int i=0; i<3; i++) {
        shader_data.eyeAndTanFov[i] = eye.Elements[i];
        for(int j=0; j<3; j++) {
            shader_data.lookAt[i][j] = lookAt.Elements[i][j];
        }
        shader_data.lookAt[i][3] = 0.0f;
    }
    for (int i=3; i<4; i++) {
        shader_data.eyeAndTanFov[i] = tan(M_PI / 180.0f) * 55.0f;
        for (int j=0; j<4; j++) {
            shader_data.lookAt[i][j] = 0.0f;
        }
    }
    return shader_data;
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
    GLFWwindow* window = glfwCreateWindow(nx, ny, "Ray GL", NULL, NULL);
    if (!window) {
        printf("Could not init GLFW window\n");
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        printf("Could not init OpenGL context\n");
        exit(-1);
    }

    GLuint outputTextureId = createAndBindEmptyTexture(0, nx, ny);

    int xSkyMap, ySkyMap, nSkyMap;
    u8 *skyMap = stbi_load("data/sky8k.jpg", &xSkyMap, &ySkyMap, &nSkyMap, STBI_rgb_alpha);
    GLuint skyMapTextureId = createAndBindTextureFromImage(1, xSkyMap, ySkyMap, skyMap);
    stbi_image_free(skyMap);

    GLuint shaderId = shaderFromSource("rayTracer", "shaders/compute.glsl");
    GLuint programId = shaderProgramFromShader(shaderId);
    glUseProgram(programId);

    ShaderData shader_data = setupData(nx, ny, xSkyMap, ySkyMap);
    GLuint ssbo = createAndBindSSBO(programId, 0, sizeof(shader_data), &shader_data);

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
    }

    glDeleteFramebuffers(1, &fboId);
    glDeleteTextures(1, &outputTextureId);
    glDeleteTextures(1, &skyMapTextureId);
    glDeleteShader(shaderId);
    glDeleteProgram(programId);
    glfwTerminate();
}
