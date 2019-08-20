#version 430
layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f, binding = 0) uniform image2D pixels;
layout(rgba32f, binding = 1) uniform image2D skyMap;
layout(std430, binding = 0) readonly buffer data
{
    float nx;
    float ny;
    float fxSkyMap;
    float fySkyMap;
    vec4 eyeAndHalfHeight;
    vec4 u;
    vec4 v;
    vec4 w;
};

const float PI = 3.1415926535897932384626433832795;
const int NUM_ITER = 50;
const float STEP = 0.2;
const float POTENTIAL_COEF = -1.5;
const float SKY_R2 = 30.0 * 30.0;
const float D_INNER_R = 2.6;
const float D_INNER_R2 = D_INNER_R * D_INNER_R;
const float D_OUTER_R = 14.0;
const float D_OUTER_R2 = D_OUTER_R * D_OUTER_R;

void main() {
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    int xSkyMap = int(fxSkyMap);
    int ySkyMap = int(fySkyMap);
    vec3 origin = eyeAndHalfHeight.xyz;
    float halfHeight = eyeAndHalfHeight.w;
    float halfWidth = halfHeight * float(nx) / ny;
    float s = float(gl_GlobalInvocationID.x) / nx;
    float t = float(gl_GlobalInvocationID.y) / ny;

    vec3 lowerLeftCorner = origin - halfWidth * u.xyz - halfHeight * v.xyz - w.xyz;
    vec3 horizontal = 2.0 * halfWidth * u.xyz;
    vec3 vertical = 2.0 * halfHeight * v.xyz;

    vec3 direction = lowerLeftCorner + s * horizontal + t * vertical - origin;

    vec3 point = origin;
    vec3 nextPoint;
    float pointDist, pointDistNext, potential;
    bool crossedAccretion = false;
    bool hitSky = true;

    for (int i=0; i<NUM_ITER; i++) {
        pointDist = length(point);
        potential = -0.033 / (pointDist * pointDist);
        direction = normalize(STEP * direction + point * potential);
        nextPoint = point + STEP * pointDist * direction;
        pointDistNext = length(nextPoint);

        if (pointDistNext <= 1. && pointDist > 1.0) {
            if (crossedAccretion) {
                color = mix(vec4(0.0, 0.0, 0.0, 1.0), color, color.a);
            }
            hitSky = false;
            break;
//#if 0
        } else if (nextPoint.z >= -10. && nextPoint.z <= -9. && abs(nextPoint.x + 2.) <= 1. && abs(nextPoint.y + 2.) <= 1.) {
            //color = mix(vec4(1.0, 0.0, 0.0, 1.0), color, color.a);
            color = vec4(1.0, 0.0, 0.0, 1.0);
            // TODO: Should we break here?
            break;
        } else if (pointDistNext >= D_INNER_R2 && pointDistNext <= D_OUTER_R2 && ((point.y > 0. && nextPoint.y < 0.) || (point.y < 0. && nextPoint.y > 0.))) {
            if (!crossedAccretion) {
                color = vec4(1.0, 1.0, 0.98, 0.0);
            }
            crossedAccretion = true;
            color.a += sin(PI * pow(((D_OUTER_R - sqrt(pointDistNext)) / (D_OUTER_R - D_INNER_R)), 2));
//#endif
        }
        point = nextPoint;
    }
    if (hitSky) {
        float theta = acos(point.z / length(point));
        float phi = atan(point.y, point.x);
        int u = int((phi / (2*PI)) * xSkyMap);
        int v = int((theta / PI) * ySkyMap);
        if (u < 0) { u = u + xSkyMap; }
        if (v < 0) { v = v + ySkyMap; }
        if (crossedAccretion) {
            color = mix(imageLoad(skyMap, ivec2(u, v)), color, color.a);
        } else {
            color = imageLoad(skyMap, ivec2(u, v));
        }
   }
    imageStore(pixels, ivec2(gl_GlobalInvocationID.xy), color);
}
