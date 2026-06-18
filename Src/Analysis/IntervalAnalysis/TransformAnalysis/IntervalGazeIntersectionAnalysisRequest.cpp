//
// Created by Anton-Lammert on 27.10.2022.
//

#include "IntervalGazeIntersectionAnalysisRequest.h"

IntervalGazeIntersectionAnalysisRequest::IntervalGazeIntersectionAnalysisRequest(
    int a, int b, int ax, GazeIntersectionShapeType st,
    float r, glm::vec3 he, float chh, int ca, float co)
    : id_a(a), id_b(b), axis(ax), shape_type(st),
      radius(r), half_extents(he), capsule_half_height(chh), capsule_axis(ca), capsule_offset_along_axis(co){}

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
            case IntersectionSphere:
                hit = GazeIntersectionShapes::intersects_sphere(
                    ray_origin, ray_dir, current_b.global_position, radius);
                break;
            case IntersectionCuboid:
                hit = GazeIntersectionShapes::intersects_cuboid(
                    ray_origin, ray_dir, current_b.global_position, current_b.global_rotation, half_extents);
                break;
            case IntersectionCapsule:
                hit = GazeIntersectionShapes::intersects_capsule(
                    ray_origin, ray_dir, current_b.global_position, current_b.global_rotation,
                    capsule_axis, capsule_half_height, capsule_offset_along_axis, radius);
                break;
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
        id_a, id_b, axis, shape_type, radius, half_extents, capsule_half_height, capsule_axis, capsule_offset_along_axis);
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
