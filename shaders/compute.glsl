#version 430
layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f, binding = 0) uniform image2D pixels;
layout(rgba32f, binding = 1) uniform image2D skyMap;
layout(std430, binding = 0) readonly buffer data
{
    int nx;
    int ny;
//    vec3 eye;
    mat3 lookAt;
    float tanFov;
    int xSkyMap;
    int ySkyMap;
};


const float PI = 3.1415926535897932384626433832795;
const int NUM_ITER = 10000;
const float STEP = 0.16;
const float POTENTIAL_COEF = -1.5;
const float SKY_SPHERE_RADIUS = 30.0;

void main() {
//    vec4 color = imageLoad(skyMap, ivec2(gl_GlobalInvocationID.xy));
//    vec4 color = vec4(lookAt[0][0], -lookAt[0][1], lookAt[0][2], 1.0);
//    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
//    uint x = gl_GlobalInvocationID.x;
//    uint y = gl_GlobalInvocationID.y;
//    vec3 view = vec3(
//        (float(x) / nx - 0.5) * tanFov,
//        ((-float(y) / ny + 0.5) * ny / nx) * tanFov,
//        1.0
//    );
//    view = normalize(lookAt * view);
//
//    vec3 velocity = view;
//    vec3 point = eye;
//    float sqrNorm = dot(point, point);
//    vec3 crossed = cross(point, velocity);
//    float h2 = dot(crossed, crossed);
//
//    for (int i=0; i<NUM_ITER; i++) {
//        float stp = (sqrt(sqrNorm) / SKY_SPHERE_RADIUS) * STEP;
//        point += velocity * stp;
//        sqrNorm = dot(point, point);
//        vec3 accel = POTENTIAL_COEF * h2 * point / pow(dot(point, point), 2.5);
//        velocity += accel * stp;
//
//        float phi = atan(point.y, point.x);
//        float theta = acos(point.z / length(point));
//
//        if (sqrNorm > SKY_SPHERE_RADIUS) {
//            int xSky= int((phi / (2*PI)) * xSkyMap) % xSkyMap;
//            int ySky= int((theta / PI) * ySkyMap) % ySkyMap;
//            if (xSky< 0) { xSky = xSkyMap + xSky; }
//            if (ySky< 0) { ySky = ySkyMap + ySky; }
//            color = imageLoad(skyMap, ivec2(xSky, ySky));
//            break;
//        }
//    }

    imageStore(pixels, ivec2(gl_GlobalInvocationID.xy), color);
}
