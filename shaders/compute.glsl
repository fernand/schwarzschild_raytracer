#version 430
layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f, binding = 0) uniform image2D pixels;
layout(std430, binding = 1) buffer data
{
    mat3 lookAt;
    float fov;
};

void main() {
    vec3 color = vec3(0.0, 0.0, 0.0);
    imageStore(pixels, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1.0));
}
