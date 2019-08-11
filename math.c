typedef union {
    struct {
        float x, y, z;
    };
    float e[3];
} v3;

static inline v3 newV3(float x, float y, float z) {
    v3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

static inline v3 mulV3(float a, v3 u) {
    v3 result;
    result.x = a * u.x;
    result.y = a * u.y;
    result.z = a * u.z;
    return result;
}

static inline v3 addV3(v3 u, v3 v) {
    v3 result;
    result.x = u.x + v.x;
    result.y = u.y + v.y;
    result.z = u.z + v.z;
    return result;
}

static inline v3 subtractV3(v3 u, v3 v) {
    v3 result;
    result.x = u.x - v.x;
    result.y = u.y - v.y;
    result.z = u.z - v.z;
    return result;
}

static inline float dotV3(v3 u, v3 v) {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

static inline v3 crossV3(v3 u, v3 v) {
    v3 result;
    result.x = u.y * v.z - u.z * v.y;
    result.y = -(u.x * v.z - u.z * v.x);
    result.z = u.x * v.y - u.y * v.x;
    return result;
}

static inline v3 normalizeV3(v3 v) {
    v3 result = {0};
    float norm = sqrtf(dotV3(v, v));
    if (norm > 0.0001f * 0.0001f) {
        result.x = v.x / norm;
        result.y = v.y / norm;
        result.z = v.z / norm;
    }
    return result;
}

static inline v3 lookAt(v3 cP, v3 u, v3 v, v3 w, v3 p) {
    v3 result;
    result.x = u.x * p.x + u.y * p.y + u.z * p.z -dotV3(u, cP);
    result.y = v.x * p.x + v.y * p.y + v.z * p.z -dotV3(v, cP);
    result.z = w.x * p.x + w.y * p.y + w.z * p.z -dotV3(w, cP);
    return result;
}

static inline v3 perspective(float f, float aspect, float zNear, float zFar, v3 p) {
    v3 result;
    result.x = (f / aspect) * p.x / p.z;
    result.y = f * p.y / p.z;
    result.z = ((zFar+zNear)/(zNear-zFar) * p.z + (2*zFar*zNear)/(zNear-zFar)) / p.z;
    return result;
}

typedef union {
    struct {
        float x, y, z, w;
    };
    float e[4];
} v4;

static inline v4 fromV3(v3 v) {
    v4 result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    result.w = 0.0f;
    return result;
}

