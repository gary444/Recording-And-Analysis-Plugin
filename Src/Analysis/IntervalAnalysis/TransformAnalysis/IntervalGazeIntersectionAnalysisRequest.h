//
// Created by Anton-Lammert on 27.10.2022.
//

#ifndef RECORDINGPLUGIN_INTERVALGAZEINTERSECTIONANALYSISREQUEST_H
#define RECORDINGPLUGIN_INTERVALGAZEINTERSECTIONANALYSISREQUEST_H

#include "IntervalTransformAnalysisRequest.h"

enum GazeIntersectionShapeType {
    IntersectionSphere  = 0,
    IntersectionCuboid  = 1,
    IntersectionCapsule = 2
};

// Tests whether the gaze ray emitted by id_a intersects a shape positioned and
// oriented by id_b's pose. Produces time intervals during which the ray hits.
class IntervalGazeIntersectionAnalysisRequest : public IntervalTransformAnalysisRequest {
private:
    int id_a;
    int id_b;
    // Gaze axis in id_a's local frame: 0=-z, 1=z, 2=-y, 3=y, 4=-x, 5=x
    int axis;

    GazeIntersectionShapeType shape_type;

    // Sphere / Capsule radius
    float radius;
    // Cuboid half-extents in id_b's local frame
    glm::vec3 half_extents;
    // Capsule half-length of the cylindrical section
    float capsule_half_height;
    // Capsule axis in id_b's local frame: 0=X, 1=Y, 2=Z
    int capsule_axis;

    bool present_a = false;
    bool present_b = false;

    TransformData current_a;
    TransformData current_b;
    TransformData last_a;
    TransformData last_b;

    glm::vec3 compute_ray_direction() const;

    // Returns true if the ray hits the given sphere (handles ray-origin-inside-sphere).
    bool ray_hits_sphere(const glm::vec3& ray_origin, const glm::vec3& ray_dir,
                         const glm::vec3& center, float r) const;

    bool intersects_sphere (const glm::vec3& ray_origin, const glm::vec3& ray_dir) const;
    bool intersects_cuboid (const glm::vec3& ray_origin, const glm::vec3& ray_dir) const;
    bool intersects_capsule(const glm::vec3& ray_origin, const glm::vec3& ray_dir) const;

public:
    // Use the shape-type enum plus the full parameter set; unused parameters are ignored.
    // Convenience wrappers are provided as free functions in the endpoint layer.
    IntervalGazeIntersectionAnalysisRequest(int a, int b, int axis,
        GazeIntersectionShapeType shape_type,
        float radius,
        glm::vec3 half_extents,
        float capsule_half_height,
        int capsule_axis);

    void process_request(std::shared_ptr<TransformData> t_data,
                         std::shared_ptr<SoundData> s_data) override;

    std::string get_description(MetaInformation& meta_info) const override;

    void update_parameters(MetaInformation& original_meta_file,
                           MetaInformation& new_meta_file) override;

    std::shared_ptr<IntervalAnalysisRequest> clone() const override;

    void clear_recent_data() override;

    TransformAnalysisType get_type() const override;

    int get_id_a() const;
    int get_id_b() const;
    void set_id_a(int new_id);
    void set_id_b(int new_id);
};

#endif //RECORDINGPLUGIN_INTERVALGAZEINTERSECTIONANALYSISREQUEST_H
