#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace VectorMath
{
    void ScreenPosToWorldRay(
        int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
        int screenWidth, int screenHeight,  // Window size, in pixels
        const glm::mat4 &ProjViewMatrix,    // Projection / View matrix
        glm::vec3& out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
        glm::vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
    );

    bool TestRayOBBIntersection(
        glm::vec3 ray_origin,        // Ray origin, in world space
        glm::vec3 ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
        glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
        glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
        glm::mat4 ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
        float& intersection_distance // Output : distance between ray_origin and the intersection with the OBB
    );

    bool TestRayOBBIntersection2D(
        glm::vec3 ray_origin,        // Ray origin, in world space
        glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
        glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
        glm::mat4 ModelMatrix        // Transformation applied to the mesh (which will thus be also applied to its bounding box)
    );

    bool TestVolumeOBBIntersection(
        int mouseX1, int mouseY1,    // Mouse position, in pixels, from bottom-left corner of the window
        int mouseX2, int mouseY2,    // Mouse position, in pixels, from bottom-left corner of the window
        int screenWidth, int screenHeight,  // Window size, in pixels
        glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
        glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
        glm::mat4 ProjViewMatrix,    // Projection / View matrix
        glm::mat4 ModelMatrix        // Transformation applied to the mesh (which will thus be also applied to its bounding box)
    );

    glm::vec2 GetScreenCoord(
        int screenWidth, int screenHeight,  // Window size, in pixels
        glm::vec3 position,          // X,Y,Z coords of the position when not transformed at all.
        glm::mat4 ProjViewMatrix,    // Projection / View matrix
        glm::mat4 ModelMatrix        // Transformation applied to the position
    );

    bool GetPlaneIntersect(
        glm::vec3 p,                 // Point origin  (x0, y0, z0)
        glm::vec3 v,                 // Ray direction (x,  y,  z)
        glm::vec3 point,             // Point on the plane
        glm::vec3 normal,            // Normal to the plane
        glm::vec3& intersect         // Output: intersect point
        );

    glm::quat rotationBetweenVectors(const glm::vec3 &start, const glm::vec3 &dest);
    glm::mat4 rotMatrixFromXAxisToVector(const glm::vec3 &vector);
    glm::mat4 rotationMatrixFromXAxisToVector(const glm::vec3 &vector);
    glm::mat4 rotationMatrixFromXAxisToVector2(const glm::vec3 &a, const glm::vec3 &b);
    glm::mat4 rotationMatrixBetweenVectors(const glm::vec3 &a, const glm::vec3 &b);

}
