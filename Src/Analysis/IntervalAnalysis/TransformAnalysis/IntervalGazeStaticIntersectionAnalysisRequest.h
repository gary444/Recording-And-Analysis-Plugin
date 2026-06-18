#ifndef RECORDINGPLUGIN_INTERVALGAZESTATICINTERSECTIONANALYSISREQUEST_H
#define RECORDINGPLUGIN_INTERVALGAZESTATICINTERSECTIONANALYSISREQUEST_H

#include "IntervalTransformAnalysisRequest.h"
#include "GazeIntersectionShapes.h"

// Tests whether the gaze ray emitted by id_a intersects a shape that is fixed in
// world space (position/rotation/scale given at construction) rather than tracked
// via a second object id. Produces time intervals during which the ray hits.
class IntervalGazeStaticIntersectionAnalysisRequest : public IntervalTransformAnalysisRequest {
private:
    int id_a;
    // Gaze axis in id_a's local frame: 0=-z, 1=z, 2=-y, 3=y, 4=-x, 5=x
    int axis;

    GazeIntersectionShapeType shape_type;

    // Sphere / Capsule radius, already scaled (see constructor)
    float radius;
    // Cuboid half-extents in the shape's local frame, already scaled
    glm::vec3 half_extents;
    // Capsule half-length of the cylindrical section, already scaled
    float capsule_half_height;
    // Capsule axis in the shape's local frame: 0=X, 1=Y, 2=Z
    int capsule_axis;
    float capsule_offset_along_axis;

    // Fixed world-space pose of the shape.
    glm::vec3 position;
    glm::quat rotation;

    // Free-form label included in get_description, to help identify this shape
    // in a recording that has no associated object id for it.
    std::string label;

    TransformData current_a;
    TransformData last_a;

    glm::vec3 compute_ray_direction() const;

public:
    // half_extents/radius/capsule_half_height/capsule_offset_along_axis are given in the
    // shape's unscaled local units; `scale` is applied once at construction time.
    // Cuboid half-extents are scaled per-axis (exact). Sphere/capsule radius and the
    // capsule's cross-section are scaled using `scale.x` — non-uniform scaling of round
    // shapes isn't geometrically well-defined, so use a uniform scale for those shapes.
    // The capsule's half-height is scaled along its own local axis component of `scale`.
    IntervalGazeStaticIntersectionAnalysisRequest(int a, int axis,
        GazeIntersectionShapeType shape_type,
        float radius,
        glm::vec3 half_extents,
        float capsule_half_height,
        int capsule_axis,
        float capsule_offset_along_axis,
        glm::vec3 position,
        glm::quat rotation,
        glm::vec3 scale,
        std::string label = ""
        );

    void process_request(std::shared_ptr<TransformData> t_data,
                         std::shared_ptr<SoundData> s_data) override;

    std::string get_description(MetaInformation& meta_info) const override;

    void update_parameters(MetaInformation& original_meta_file,
                           MetaInformation& new_meta_file) override;

    std::shared_ptr<IntervalAnalysisRequest> clone() const override;

    void clear_recent_data() override;

    TransformAnalysisType get_type() const override;

    int get_id_a() const;
    void set_id_a(int new_id);
};

#endif //RECORDINGPLUGIN_INTERVALGAZESTATICINTERSECTIONANALYSISREQUEST_H
