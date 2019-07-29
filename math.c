#define HANDMADE_MATH_IMPLEMENTATION
#include "include/handmade_math.h"

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

