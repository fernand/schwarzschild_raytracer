typedef union {
    struct {
        float X, Y, Z;
    };
    float E[3];
} vec3;

static inline vec3 newVec3(float x, float y, float z) {
    return (vec3) {x, y, z};
}

static inline vec3 addVec3(vec3 u, vec3 v) {
    vec3 result;
    result.X = u.X + v.X;
    result.Y = u.Y + v.Y;
    result.Z = u.Z + v.Z;
    return result;
}

static inline vec3 subtractVec3(vec3 u, vec3 v) {
    vec3 result;
    result.X = u.X - v.X;
    result.Y = u.Y - v.Y;
    result.Z = u.Z - v.Z;
    return result;
}

static inline float dotVec3(vec3 u, vec3 v) {
    return u.X * v.X + u.Y * v.Y + u.Z * v.Z;
}

static inline vec3 crossVec3(vec3 u, vec3 v) {
    vec3 result;
    result.X = u.Y * v.Z - u.Z * v.Y;
    result.Y = -(u.X * v.Z - u.Z * v.X);
    result.Z = u.X * v.Y - u.Y * v.X;
    return result;
}

static inline vec3 normalizeVec3(vec3 v) {
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
    float E[4][4];
} mat4;

static vec3 transform(mat4 m, vec3 v) {
    vec3 result;
    result.X = m.E[0][0] * v.X + m.E[1][0] * v.Y + m.E[2][0] * v.Z;
    result.Y = m.E[0][1] * v.X + m.E[1][1] * v.Y + m.E[2][1] * v.Z;
    result.Z = m.E[0][2] * v.X + m.E[1][2] * v.Y + m.E[2][2] * v.Z;
    return result;
}

static mat4 multiplyMatrix(mat4 a, mat4 b) {
    mat4 result;
    for (int c=0; c<4; c++) {
        for (int r=0; r<4; r++) {
            for(int i=0; i<4; i++) {
                result.E[c][r] = a.E[i][r] * b.E[c][i];
            }
        }
    }
    return result;
}

static mat4 rotationMatrix(float alpha, float beta) {
    float cAlpha = cosf(alpha);
    float cBeta = cosf(beta);
    float sAlpha = sinf(alpha);
    float sBeta = sinf(beta);
    return (mat4) {{
        {cAlpha, sAlpha * sBeta, sAlpha * cBeta, 0.0f},
        {0.0f, cBeta, -sBeta, 0.0f},
        {-sAlpha, cAlpha * sBeta, cAlpha * cBeta, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
}

static mat4 getLookAt(vec3 eye, vec3 center, vec3 up) {
    mat4 result;

    vec3 F = normalizeVec3(subtractVec3(center, eye));
    vec3 L = normalizeVec3(crossVec3(up, F));
    vec3 U = crossVec3(F, L);

    result.E[0][0] = L.X;
    result.E[0][1] = U.X;
    result.E[0][2] = F.X;
    result.E[0][3] = 0.0f;

    result.E[1][0] = L.Y;
    result.E[1][1] = U.Y;
    result.E[1][2] = F.Y;
    result.E[1][3] = 0.0f;

    result.E[2][0] = L.Z;
    result.E[2][1] = U.Z;
    result.E[2][2] = F.Z;
    result.E[2][3] = 0.0f;

    result.E[3][0] = dotVec3(L, eye);
    result.E[3][1] = -dotVec3(U, eye);
    result.E[3][2] = -dotVec3(F, eye);
    result.E[3][3] = 1.0f;

    return result;
}

