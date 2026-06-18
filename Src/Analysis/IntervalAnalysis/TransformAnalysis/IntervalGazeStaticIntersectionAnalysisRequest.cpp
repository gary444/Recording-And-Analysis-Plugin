#include "IntervalGazeStaticIntersectionAnalysisRequest.h"

IntervalGazeStaticIntersectionAnalysisRequest::IntervalGazeStaticIntersectionAnalysisRequest(
    int a, int ax, GazeIntersectionShapeType st,
    float r, glm::vec3 he, float chh, int ca, float co,
    glm::vec3 pos, glm::quat rot, glm::vec3 scale, std::string lbl)
    : id_a(a), axis(ax), shape_type(st),
      radius(r * scale.x),
      half_extents(he * scale),
      capsule_half_height(chh * scale[ca]),
      capsule_axis(ca),
      capsule_offset_along_axis(co * scale[ca]),
      position(pos), rotation(rot), label(std::move(lbl)) {}

glm::vec3 IntervalGazeStaticIntersectionAnalysisRequest::compute_ray_direction() const {
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

void IntervalGazeStaticIntersectionAnalysisRequest::process_request(
    std::shared_ptr<TransformData> t_data, std::shared_ptr<SoundData> s_data)
{
    if (!t_data) return;

    if (t_data->id != id_a) return;

    last_a    = current_a;
    current_a = *t_data;

    glm::vec3 ray_origin = current_a.global_position;
    glm::vec3 ray_dir    = compute_ray_direction();

    bool hit = false;
    switch (shape_type) {
        case IntersectionSphere:
            hit = GazeIntersectionShapes::intersects_sphere(
                ray_origin, ray_dir, position, radius);
            break;
        case IntersectionCuboid:
            hit = GazeIntersectionShapes::intersects_cuboid(
                ray_origin, ray_dir, position, rotation, half_extents);
            break;
        case IntersectionCapsule:
            hit = GazeIntersectionShapes::intersects_capsule(
                ray_origin, ray_dir, position, rotation,
                capsule_axis, capsule_half_height, capsule_offset_along_axis, radius);
            break;
    }

    if (hit) {
        if (currentInterval.start <= 0.0f)
            currentInterval.start = current_a.time;
    } else {
        currentInterval.end = last_a.time;
        if (currentInterval.start >= 0.0f && currentInterval.end >= currentInterval.start)
            intervals.push_back(currentInterval);
        currentInterval.start = -1.0f;
        currentInterval.end   = -1.0f;
    }
}

std::string IntervalGazeStaticIntersectionAnalysisRequest::get_description(
    MetaInformation& meta_info) const
{
    std::string s = "GazeStaticIntersectionAnalysis";
    s += " idA: " + meta_info.get_object_name(id_a);
    if (!label.empty())
        s += " label: " + label;
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

void IntervalGazeStaticIntersectionAnalysisRequest::update_parameters(
    MetaInformation& original_meta_file, MetaInformation& new_meta_file)
{
    id_a = new_meta_file.get_old_uuid(original_meta_file.get_object_name(id_a));
}

std::shared_ptr<IntervalAnalysisRequest> IntervalGazeStaticIntersectionAnalysisRequest::clone() const {
    return std::make_shared<IntervalGazeStaticIntersectionAnalysisRequest>(
        id_a, axis, shape_type, radius, half_extents, capsule_half_height, capsule_axis,
        capsule_offset_along_axis, position, rotation, glm::vec3{1.0f, 1.0f, 1.0f}, label);
}

TransformAnalysisType IntervalGazeStaticIntersectionAnalysisRequest::get_type() const {
    return GazeStaticIntersectionAnalysis;
}

void IntervalGazeStaticIntersectionAnalysisRequest::clear_recent_data() {
    current_a = {};
    last_a    = {};
}

int  IntervalGazeStaticIntersectionAnalysisRequest::get_id_a() const { return id_a; }
void IntervalGazeStaticIntersectionAnalysisRequest::set_id_a(int new_id) { id_a = new_id; }
