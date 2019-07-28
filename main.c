#include "include/glad/glad.h"
#define GLFW_DLL
#include "include/GLFW/glfw3.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#define HANDMADE_MATH_IMPLEMENTATION
#include "include/handmade_math.h"

#include <stdio.h>
#include <stdlib.h> // exit
#include <math.h> // sqrt

typedef unsigned char u8;

#include "io.c"
#include "shaders.c"

GLuint genImageBuffer(const GLuint texI, const int nx, const int ny) {
    GLuint texture;
    glGenTextures(1, &texture); ck();
    glActiveTexture(GL_TEXTURE0+texI); ck();
    glBindTexture(GL_TEXTURE_2D, texture); ck();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nx, ny, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(texI, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    return texture;
}

GLuint createImageBuffer(const GLuint texI, const int nx, const int ny, const u8 *data) {
    GLuint texture;
    glGenTextures(1, &texture); ck();
    glActiveTexture(GL_TEXTURE0+texI); ck();
    glBindTexture(GL_TEXTURE_2D, texture); ck();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); ck();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nx, ny, 0, GL_RGB, GL_UNSIGNED_BYTE, data); ck();
    glBindImageTexture(texI, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F); ck();
    return texture;
}

typedef struct {
    float lookAt[3][3];
    float fov;
} ShaderData;

GLuint setupSSBO(GLuint programId) {
    hmm_vec3 eye = HMM_Vec3(0, 3, -20);
    hmm_vec3 center = HMM_Vec3(0, 0, 0);
    hmm_vec3 up = HMM_Vec3(-0.3, 1, 0);
    hmm_mat4 lookAt = HMM_LookAt(eye, center, up);
    ShaderData shader_data;
    for(int i=0; i<3; i++) {
        for(int j=0; j<3; j++) {
            shader_data.lookAt[i][j] = lookAt.Elements[i][j];
        }
    }
    shader_data.fov = 55.0f;

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(shader_data), &shader_data, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

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
    const int nx = 512;
    const int ny = 256;

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

    GLuint outputTexI = 0;
    GLuint outputTextureId = genImageBuffer(outputTexI, nx, ny);
    int xSkyMap, ySkyMap, nSkyMap;
    u8 *skyMap = stbi_load("data/sky8k.jpg", &xSkyMap, &ySkyMap, &nSkyMap, STBI_rgb);
    GLuint skyMapTexI = 1;
    GLuint skyMapTextureId = createImageBuffer(skyMapTexI, xSkyMap, ySkyMap, skyMap);
    GLuint shaderId = shaderFromSource("rayTracer", "shaders/compute.glsl");
    GLuint programId = shaderProgramFromShader(shaderId);
    glUseProgram(programId);

    GLuint ssbo = setupSSBO(programId);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
    glDispatchCompute(nx/32, ny/32, 1); ck();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); ck();

    float *imgData = malloc(sizeof(float)*nx*ny*3);
    glBindTexture(GL_TEXTURE_2D, outputTextureId); ck();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, (GLvoid*)imgData); ck();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    writePNG(nx, ny, imgData);

    free(imgData);
    glDeleteShader(shaderId);
    glDeleteProgram(programId);
    glfwTerminate();
    return 0;
}
