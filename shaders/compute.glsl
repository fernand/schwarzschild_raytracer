#version 430
layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f, binding = 0) uniform image2D pixels;

struct Material {
    vec3 albedo;
    float fuzz;
};

struct Sphere {
    vec3 center;
    float radius;
    uint mat_i;
};

struct Camera {
    vec3 origin;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
};

const Material materials[4] = Material[4](
    Material(vec3(0.8, 0.3, 0.3), 0.0),
    Material(vec3(0.8, 0.8, 0.0), 0.0),
    Material(vec3(0.8, 0.6, 0.2), 1.0),
    Material(vec3(0.8, 0.8, 0.8), 0.3)
);

const Sphere objects[4] = Sphere[4](
    Sphere(vec3(0.0, 0.0, -1.0), 0.5, 0),
    Sphere(vec3(0.0, -100.5, -1.0), 100.0, 1),
    Sphere(vec3(1.0, 0.0, -1.0), 0.5, 2),
    Sphere(vec3(-1.0, 0.0, -1.0), 0.5, 3)
);

const Camera cam = Camera(
    vec3(-2.0, 2.0, 1.0),
    vec3(-2.0859715983286593, 1.1254692789725564, 0.05650831512990695),
    vec3(1.029463283198752, -0.0, 1.029463283198752),
    vec3(0.2971804518378177, 0.5943609036756354, -0.2971804518378177)
);

uint rng_state;

// Interleaved Gradient Noise to seed xorshift per pixel.
float inoise(uvec2 xy) {
    return fract(
        52.9829189f * fract(xy.x * 0.06711056f + xy.y * 0.00583715f)
    );
}

uint xorshift() {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

float randFloat() {
    return float(xorshift()) * (1.0 / float(0xffffffffU));
}

// TODO: improve this function.
vec3 randInUnitSphere() {
    vec3 p = vec3(xorshift(), xorshift(), xorshift());
    return normalize(p);
}

vec3 reflect(vec3 v, vec3 n) {
    return v - 2.0 * dot(v, n) * n;
}

#define FLT_MAX 3.402823466e+38
#define NUM_RAYS 100
#define MAX_DEPTH 50
#define NX 2048
#define NY 1024

void main() {
    vec3 color = vec3(0.0, 0.0, 0.0);
    rng_state = uint(inoise(gl_GlobalInvocationID.xy) * 0xffffffffU);

    for (int k=0; k<NUM_RAYS; k++) {
        float u = (gl_GlobalInvocationID.x + randFloat()) / NX;
        float v = (gl_GlobalInvocationID.y + randFloat()) / NY;
        vec3 ray_origin = cam.origin;
        vec3 ray_direction = cam.lower_left_corner + u * cam.horizontal + v * cam.vertical - cam.origin; 
        vec3 attenuation = vec3(1.0);
        for (int d=0; d<MAX_DEPTH; d++) {
            // Collision detection with all the spheres
            bool hit = false;
            float hit_t = FLT_MAX;
            vec3 hit_p;
            vec3 hit_normal;
            uint hit_mat_i;
            for (int i=0; i<objects.length(); i++) {
                vec3 oc = ray_origin - objects[i].center;
                float a = dot(ray_direction, ray_direction);
                float b = dot(oc, ray_direction);
                float c = dot(oc, oc) - objects[i].radius * objects[i].radius;
                float discriminant = b * b - a * c;
                if (discriminant > 0.0) {
                    float a_t_max = a * hit_t;
                    float a_t_min = a * 0.001;
                    float temp = -b - sqrt(discriminant);
                    if (temp < a_t_max && temp > a_t_min) {
                        hit = true;
                        hit_t = temp / a;
                        hit_p = ray_origin + hit_t * ray_direction;
                        hit_normal = (hit_p - objects[i].center) / objects[i].radius;
                        hit_mat_i = objects[i].mat_i;
                        continue;
                    }
                    temp = -b + sqrt(discriminant);
                    if (temp < a_t_max && temp > a_t_min) {
                        hit = true;
                        hit_t = temp / a;
                        hit_p = ray_origin + hit_t * ray_direction;
                        hit_normal = (hit_p - objects[i].center) / objects[i].radius;
                        hit_mat_i = objects[i].mat_i;
                        continue;
                    }
                }
            }
            if (hit) {
                Material mat = materials[hit_mat_i];
                // Branch by metal or not metal.
                if (mat.fuzz < 0.01) {
                    vec3 target = hit_p + hit_normal + randInUnitSphere();
                    ray_origin = hit_p;
                    ray_direction = target - hit_p;
                    attenuation *= mat.albedo;
                    continue;
                } else {
                    vec3 reflected = reflect(normalize(ray_direction), hit_normal);
                    vec3 scattered_direction = reflected + mat.fuzz * randInUnitSphere();
                    if (dot(scattered_direction, hit_normal) > 0.0) {
                        ray_origin = hit_p;
                        ray_direction = scattered_direction;
                        attenuation *= mat.albedo;
                        continue;
                    } else {
                        break;
                    }
                }

            } else {
                float alpha = 0.5 * (normalize(ray_direction).y + 1.0);
                color += attenuation * mix(vec3(1.0), vec3(0.5, 0.7, 1.0), alpha);
                break;
            }
        }
    }
    color /= NUM_RAYS;

    imageStore(pixels, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1.0));
}
