#include "include/glad/glad.h"
#define GLFW_DLL
#include "include/GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image_write.h"
#define HANDMADE_MATH_IMPLEMENTATION
#include "include/handmade_math.h"

#include <stdio.h>
#include <stdlib.h> // exit
#include <math.h>

#define M_PI 3.14159265358979323846f

typedef unsigned char u8;

#include "io.c"
#include "shaders.c"

typedef struct {
    float Elements[3][3];
} mat3;

mat3 getLookAt(hmm_vec3 Eye, hmm_vec3 Center, hmm_vec3 Up) {
    mat3 result;

    hmm_vec3 F = HMM_NormalizeVec3(HMM_SubtractVec3(Center, Eye));
    hmm_vec3 L = HMM_NormalizeVec3(HMM_Cross(Up, F));
    hmm_vec3 U = HMM_Cross(F, L);

    result.Elements[0][0] = L.X;
    result.Elements[0][1] = U.X;
    result.Elements[0][2] = F.X;

    result.Elements[1][0] = L.Y;
    result.Elements[1][1] = U.Y;
    result.Elements[1][2] = F.Y;

    result.Elements[2][0] = L.Z;
    result.Elements[2][1] = U.Z;
    result.Elements[2][2] = F.Z;

    return result;
}

GLuint createAndBindEmptyTexture(const GLuint texUnit, const int nx, const int ny) {
    GLuint texture;
    glGenTextures(1, &texture); ck();
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(GL_TEXTURE_2D, texture); ck();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nx, ny, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(texUnit, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    return texture;
}

GLuint createAndBindTextureFromImage(const GLuint texUnit, const int nx, const int ny, const u8 *data) {
    GLuint texture;
    glGenTextures(1, &texture); ck();
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(GL_TEXTURE_2D, texture); ck();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); ck();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nx, ny, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); ck();
    glBindImageTexture(texUnit, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F); ck();
    return texture;
}

typedef struct {
    float nx;
    float ny;
    float xSkyMap;
    float ySkyMap;
    float eyeAndTanFov[4];
    float lookAt[4][4];
} ShaderData;

GLuint createAndBindSSBO(GLuint programId, GLuint ssboLocation, int nx, int ny, int xSkyMap, int ySkyMap) {
    hmm_vec3 eye = HMM_Vec3(0, 3, -20);
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

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(shader_data), &shader_data, GL_STATIC_DRAW); ck();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssboLocation, ssbo);
    return ssbo;
}

void writePNG(const int nx, const int ny, const float *imgData) {
    const int size = nx*ny*3;
    u8 *pixels = malloc(sizeof(u8)*size);
    for (int i=0; i<size; i++) {
        pixels[i] = 255.99 * sqrt(imgData[i]);
    }
    stbi_write_png("image.png", nx, ny, 3, pixels, 3*nx);
    free(pixels);
}

int main() {
    const int nx = 1024;
    const int ny = 1024;

    if (!glfwInit()) {
        printf("Could not init GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(nx, ny, "Ray GL", NULL, NULL);
    if (!window) {
        printf("Could not init GLFW window\n");
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        printf("Could not init OpenGL context\n");
        return -1;
    }


    GLuint outputTextureId = createAndBindEmptyTexture(0, nx, ny);

    int xSkyMap, ySkyMap, nSkyMap;
    u8 *skyMap = stbi_load("data/sky8k.jpg", &xSkyMap, &ySkyMap, &nSkyMap, STBI_rgb_alpha);
    GLuint skyMapTextureId = createAndBindTextureFromImage(1, xSkyMap, ySkyMap, skyMap);

    GLuint shaderId = shaderFromSource("rayTracer", "shaders/compute.glsl");
    GLuint programId = shaderProgramFromShader(shaderId);
    glUseProgram(programId);

    GLuint ssbo = createAndBindSSBO(programId, 0, nx, ny, xSkyMap, ySkyMap);

    glDispatchCompute(nx/32, ny/32, 1); ck();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); ck();

    float *imgData = malloc(sizeof(float)*nx*ny*3);
    glBindTexture(GL_TEXTURE_2D, outputTextureId); ck();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, (GLvoid*)imgData); ck();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    writePNG(nx, ny, imgData);

    free(imgData);
    stbi_image_free(skyMap);
    glDeleteTextures(1, &outputTextureId);
    glDeleteTextures(1, &skyMapTextureId);
    glDeleteShader(shaderId);
    glDeleteProgram(programId);
    glfwTerminate();
    return 0;
}
