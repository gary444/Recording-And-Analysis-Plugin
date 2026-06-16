//
// Created by Anton-Lammert on 27.10.2022.
//

#include "IntervalGazeIntersectionAnalysisRequest.h"
#include <cfloat>
#include <cmath>
#include <algorithm>

IntervalGazeIntersectionAnalysisRequest::IntervalGazeIntersectionAnalysisRequest(
    int a, int b, int ax, GazeIntersectionShapeType st,
    float r, glm::vec3 he, float chh, int ca)
    : id_a(a), id_b(b), axis(ax), shape_type(st),
      radius(r), half_extents(he), capsule_half_height(chh), capsule_axis(ca) {}

glm::vec3 IntervalGazeIntersectionAnalysisRequest::compute_ray_direction() const {
    glm::mat4 rot = glm::toMat4(current_a.global_rotation);
    glm::vec4 axis_dir;
    switch (axis) {
        case 0:  axis_dir = { 0.0f,  0.0f, -1.0f, 1.0f}; break;
        case 1:  axis_dir = { 0.0f,  0.0f,  1.0f, 1.0f}; break;
        case 2:  axis_dir = { 0.0f, -1.0f,  0.0f, 1.0f}; break;
        case 3:  axis_dir = { 0.0f,  1.0f,  0.0f, 1.0f}; break;
        case 4:  axis_dir = {-1.0f,  0.0f,  0.0f, 1.0f}; break;
        default: axis_dir = { 1.0f,  0.0f,  0.0f, 1.0f}; break; // axis == 5
    }
    glm::vec4 dir = rot * axis_dir;
    return glm::normalize(glm::vec3{dir.x / dir.w, dir.y / dir.w, dir.z / dir.w});
}

bool IntervalGazeIntersectionAnalysisRequest::ray_hits_sphere(
    const glm::vec3& ray_origin, const glm::vec3& ray_dir,
    const glm::vec3& center, float r) const
{
    glm::vec3 oc = ray_origin - center;
    float half_b = glm::dot(oc, ray_dir);
    float c      = glm::dot(oc, oc) - r * r;
    float disc   = half_b * half_b - c;
    if (disc < 0.0f) return false;
    float sqd = std::sqrt(disc);
    return (-half_b - sqd >= 0.0f) || (-half_b + sqd >= 0.0f);
}

bool IntervalGazeIntersectionAnalysisRequest::intersects_sphere(
    const glm::vec3& ray_origin, const glm::vec3& ray_dir) const
{
    return ray_hits_sphere(ray_origin, ray_dir, current_b.global_position, radius);
}

bool IntervalGazeIntersectionAnalysisRequest::intersects_cuboid(
    const glm::vec3& ray_origin, const glm::vec3& ray_dir) const
{
    // Transform the ray into id_b's local frame so an OBB becomes an AABB test.
    glm::quat inv_rot    = glm::inverse(current_b.global_rotation);
    glm::vec3 local_orig = inv_rot * (ray_origin - current_b.global_position);
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

bool IntervalGazeIntersectionAnalysisRequest::intersects_capsule(
    const glm::vec3& ray_origin, const glm::vec3& ray_dir) const
{
    // Build the capsule segment in world space using id_b's rotation.
    glm::vec3 cap_local;
    switch (capsule_axis) {
        case 0:  cap_local = {1.0f, 0.0f, 0.0f}; break;
        case 2:  cap_local = {0.0f, 0.0f, 1.0f}; break;
        default: cap_local = {0.0f, 1.0f, 0.0f}; break; // capsule_axis == 1 (Y)
    }
    glm::vec3 cap_dir = current_b.global_rotation * cap_local;

    glm::vec3 P0 = current_b.global_position - capsule_half_height * cap_dir;
    glm::vec3 P1 = current_b.global_position + capsule_half_height * cap_dir;
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

void IntervalGazeIntersectionAnalysisRequest::process_request(
    std::shared_ptr<TransformData> t_data, std::shared_ptr<SoundData> s_data)
{
    if (!t_data) return;

    if (t_data->id == id_a) {
        last_a    = current_a;
        current_a = *t_data;
        present_a = true;
    } else if (t_data->id == id_b) {
        last_b    = current_b;
        current_b = *t_data;
        present_b = true;
    }

    if ((t_data->id == id_a || t_data->id == id_b) && present_a && present_b) {
        glm::vec3 ray_origin = current_a.global_position;
        glm::vec3 ray_dir    = compute_ray_direction();

        bool hit = false;
        switch (shape_type) {
            case IntersectionSphere:  hit = intersects_sphere (ray_origin, ray_dir); break;
            case IntersectionCuboid:  hit = intersects_cuboid (ray_origin, ray_dir); break;
            case IntersectionCapsule: hit = intersects_capsule(ray_origin, ray_dir); break;
        }

        if (hit) {
            if (currentInterval.start <= 0.0f)
                currentInterval.start = current_a.time > current_b.time
                                        ? current_a.time : current_b.time;
        } else {
            currentInterval.end = last_a.time < last_b.time ? last_a.time : last_b.time;
            if (currentInterval.start >= 0.0f && currentInterval.end >= currentInterval.start)
                intervals.push_back(currentInterval);
            currentInterval.start = -1.0f;
            currentInterval.end   = -1.0f;
        }
    }
}

std::string IntervalGazeIntersectionAnalysisRequest::get_description(
    MetaInformation& meta_info) const
{
    std::string s = "GazeIntersectionAnalysis";
    s += " idA: " + meta_info.get_object_name(id_a);
    s += " idB: " + meta_info.get_object_name(id_b);
    switch (shape_type) {
        case IntersectionSphere:
            s += " shape: Sphere radius: " + std::to_string(radius);
            break;
        case IntersectionCuboid:
            s += " shape: Cuboid halfExtents: "
               + std::to_string(half_extents.x) + ","
               + std::to_string(half_extents.y) + ","
               + std::to_string(half_extents.z);
            break;
        case IntersectionCapsule:
            s += " shape: Capsule radius: " + std::to_string(radius)
               + " halfHeight: " + std::to_string(capsule_half_height);
            break;
    }
    return s;
}

void IntervalGazeIntersectionAnalysisRequest::update_parameters(
    MetaInformation& original_meta_file, MetaInformation& new_meta_file)
{
    id_a = new_meta_file.get_old_uuid(original_meta_file.get_object_name(id_a));
    id_b = new_meta_file.get_old_uuid(original_meta_file.get_object_name(id_b));
}

std::shared_ptr<IntervalAnalysisRequest> IntervalGazeIntersectionAnalysisRequest::clone() const {
    return std::make_shared<IntervalGazeIntersectionAnalysisRequest>(
        id_a, id_b, axis, shape_type, radius, half_extents, capsule_half_height, capsule_axis);
}

TransformAnalysisType IntervalGazeIntersectionAnalysisRequest::get_type() const {
    return GazeIntersectionAnalysis;
}

void IntervalGazeIntersectionAnalysisRequest::clear_recent_data() {
    current_a = {};
    current_b = {};
    last_a    = {};
    last_b    = {};
}

int  IntervalGazeIntersectionAnalysisRequest::get_id_a() const { return id_a; }
int  IntervalGazeIntersectionAnalysisRequest::get_id_b() const { return id_b; }
void IntervalGazeIntersectionAnalysisRequest::set_id_a(int new_id) { id_a = new_id; }
void IntervalGazeIntersectionAnalysisRequest::set_id_b(int new_id) { id_b = new_id; }
