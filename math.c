typedef union {
    struct {
        float X, Y, Z;
    };
    struct {
        float E[3];
    };
} vec3;

inline vec3 newVec3(float x, float y, float z) {
    return (vec3) {x, y, z};
}

inline vec3 addVec3(vec3 u, vec3 v) {
    vec3 result;
    result.X = u.X + v.X;
    result.Y = u.Y + v.Y;
    result.Z = u.Z + v.Z;
    return result;
}

inline vec3 subtractVec3(vec3 u, vec3 v) {
    vec3 result;
    result.X = u.X - v.X;
    result.Y = u.Y - v.Y;
    result.Z = u.Z - v.Z;
    return result;
}

inline float dotVec3(vec3 u, vec3 v) {
    return u.X * v.X + u.Y * v.Y + u.Z * v.Z;
}

inline vec3 crossVec3(vec3 u, vec3 v) {
    vec3 result;
    result.X = u.Y * v.Z - u.Z * v.Y;
    result.Y = -(u.X * v.Z - u.Z * v.X);
    result.Z = u.X * v.Y - u.Y * v.X;
    return result;
}

inline vec3 normalizeVec3(vec3 v) {
    vec3 result = {0};
    float norm = sqrt(dotVec3(v, v));
    if (norm > 0.0001f * 0.0001f) {
        result.X = v.X / norm;
        result.Y = v.Y / norm;
        result.Z = v.Z / norm;
    }
    return result;
}

typedef struct {
    float E[3][3];
} mat3;

mat3 getLookAt(vec3 eye, vec3 center, vec3 up) {
    mat3 result;

    vec3 F = normalizeVec3(subtractVec3(center, eye));
    vec3 L = normalizeVec3(crossVec3(up, F));
    vec3 U = crossVec3(F, L);

    result.E[0][0] = L.X;
    result.E[0][1] = U.X;
    result.E[0][2] = F.X;

    result.E[1][0] = L.Y;
    result.E[1][1] = U.Y;
    result.E[1][2] = F.Y;

    result.E[2][0] = L.Z;
    result.E[2][1] = U.Z;
    result.E[2][2] = F.Z;

    return result;
}

