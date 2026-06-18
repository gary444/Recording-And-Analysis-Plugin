//
// Shared ray/shape intersection tests used by gaze-intersection analysis requests.
//

#ifndef RECORDINGPLUGIN_GAZEINTERSECTIONSHAPES_H
#define RECORDINGPLUGIN_GAZEINTERSECTIONSHAPES_H

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

enum GazeIntersectionShapeType {
    IntersectionSphere  = 0,
    IntersectionCuboid  = 1,
    IntersectionCapsule = 2
};

namespace GazeIntersectionShapes {

    // Returns true if the ray hits the given sphere (handles ray-origin-inside-sphere).
    bool ray_hits_sphere(const glm::vec3& ray_origin, const glm::vec3& ray_dir,
                          const glm::vec3& center, float r);

    bool intersects_sphere(const glm::vec3& ray_origin, const glm::vec3& ray_dir,
                            const glm::vec3& center, float radius);

    // Shape is an OBB given by its center, rotation and half-extents in its local frame.
    bool intersects_cuboid(const glm::vec3& ray_origin, const glm::vec3& ray_dir,
                            const glm::vec3& center, const glm::quat& rotation,
                            const glm::vec3& half_extents);

    // Capsule axis in the shape's local frame: 0=X, 1=Y, 2=Z.
    bool intersects_capsule(const glm::vec3& ray_origin, const glm::vec3& ray_dir,
                             const glm::vec3& center, const glm::quat& rotation,
                             int capsule_axis, float capsule_half_height,
                             float capsule_offset_along_axis, float radius);

}

#endif //RECORDINGPLUGIN_GAZEINTERSECTIONSHAPES_H
