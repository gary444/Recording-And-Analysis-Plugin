#include "GazeIntersectionShapes.h"
#include <cfloat>
#include <cmath>
#include <algorithm>

namespace GazeIntersectionShapes {

    bool ray_hits_sphere(const glm::vec3& ray_origin, const glm::vec3& ray_dir,
                          const glm::vec3& center, float r)
    {
        glm::vec3 oc = ray_origin - center;
        float half_b = glm::dot(oc, ray_dir);
        float c      = glm::dot(oc, oc) - r * r;
        float disc   = half_b * half_b - c;
        if (disc < 0.0f) return false;
        float sqd = std::sqrt(disc);
        return (-half_b - sqd >= 0.0f) || (-half_b + sqd >= 0.0f);
    }

    bool intersects_sphere(const glm::vec3& ray_origin, const glm::vec3& ray_dir,
                            const glm::vec3& center, float radius)
    {
        return ray_hits_sphere(ray_origin, ray_dir, center, radius);
    }

    bool intersects_cuboid(const glm::vec3& ray_origin, const glm::vec3& ray_dir,
                            const glm::vec3& center, const glm::quat& rotation,
                            const glm::vec3& half_extents)
    {
        // Transform the ray into the shape's local frame so an OBB becomes an AABB test.
        glm::quat inv_rot    = glm::inverse(rotation);
        glm::vec3 local_orig = inv_rot * (ray_origin - center);
        glm::vec3 local_dir  = inv_rot * ray_dir;

        // Slab method: t_min starts at 0 so only forward intersections are returned.
        float t_min = 0.0f;
        float t_max = FLT_MAX;

        for (int i = 0; i < 3; i++) {
            if (std::abs(local_dir[i]) < 1e-6f) {
                if (local_orig[i] < -half_extents[i] || local_orig[i] > half_extents[i])
                    return false;
            } else {
                float inv_d = 1.0f / local_dir[i];
                float t1    = (-half_extents[i] - local_orig[i]) * inv_d;
                float t2    = ( half_extents[i] - local_orig[i]) * inv_d;
                t_min = std::max(t_min, std::min(t1, t2));
                t_max = std::min(t_max, std::max(t1, t2));
            }
        }
        return t_min <= t_max;
    }

    bool intersects_capsule(const glm::vec3& ray_origin, const glm::vec3& ray_dir,
                             const glm::vec3& center, const glm::quat& rotation,
                             int capsule_axis, float capsule_half_height,
                             float capsule_offset_along_axis, float radius)
    {
        // Build the capsule segment in world space using the shape's rotation.
        glm::vec3 cap_local;
        switch (capsule_axis) {
            case 0:  cap_local = {1.0f, 0.0f, 0.0f}; break;
            case 2:  cap_local = {0.0f, 0.0f, 1.0f}; break;
            default: cap_local = {0.0f, 1.0f, 0.0f}; break; // capsule_axis == 1 (Y)
        }
        glm::vec3 cap_dir = rotation * cap_local;

        glm::vec3 P0 = center + (-capsule_half_height + capsule_offset_along_axis) * cap_dir;
        glm::vec3 P1 = center + (capsule_half_height + capsule_offset_along_axis) * cap_dir;
        glm::vec3 AB = P1 - P0;
        glm::vec3 AO = ray_origin - P0;

        float AB_len_sq = glm::dot(AB, AB);
        float AB_dot_d  = glm::dot(AB, ray_dir);
        float AB_dot_AO = glm::dot(AB, AO);

        // Quadratic for intersection with the infinite cylinder perpendicular to AB.
        float a     = 1.0f - (AB_dot_d * AB_dot_d) / AB_len_sq;
        float b     = glm::dot(AO, ray_dir) - (AB_dot_AO * AB_dot_d) / AB_len_sq;
        float c_val = glm::dot(AO, AO) - (AB_dot_AO * AB_dot_AO) / AB_len_sq - radius * radius;

        // Test the cylindrical body only when the ray is not parallel to the capsule axis.
        if (std::abs(a) > 1e-6f) {
            float disc = b * b - a * c_val;
            if (disc >= 0.0f) {
                float sqd = std::sqrt(disc);
                for (float t : {(-b - sqd) / a, (-b + sqd) / a}) {
                    if (t < 0.0f) continue;
                    // s is the normalised position along the segment [0,1]
                    float s = (AB_dot_d * t + AB_dot_AO) / AB_len_sq;
                    if (s >= 0.0f && s <= 1.0f) return true;
                }
            }
        }

        // Fall through to hemisphere cap tests (also handles the parallel-ray case).
        return ray_hits_sphere(ray_origin, ray_dir, P0, radius) ||
               ray_hits_sphere(ray_origin, ray_dir, P1, radius);
    }

}
