#include "geometry-utilities.h"


std::vector<glm::vec3> computeTangentBitangent(glm::vec3 p1, glm::vec2 p1UV, glm::vec3 p2, glm::vec2 p2UV, glm::vec3 p3, glm::vec2 p3UV)
{
    /**
     * Approach: For a triangle formed from the points p1, p2, and p3, we can compute the tangent vector
     * T and bi-tangent vector B by recognizing that the edge vectors from p1 to p2 (p2 - p1) and p1 to p3
     * (p3 - p1) can be formed by a linear combination of the tangent and bitangent vectors. 
     * The +U direction in 2D space corresponds to the tangent vector, and the +V direction in 2D space corresponds
     * to the bitangent vector. 
     * 
     * Edge 1 has UV vectors euv1 = (u1, v1) and edge 2 has UV vectors euv2 = (u2, v2). If we apply this to the
     * equivalent 3D vectors, we get that Edge 1 = u1 * T + v1 * B and edge 2 = u2 * T + v2 * B.
     * 
     * We compute tangent vectors per triangle
     */

    // Compute the uv differences in uv space
    glm::vec2 uv1 = p2UV - p1UV, uv2 = p3UV - p1UV;

    glm::vec3 edge1 = p2 - p1, edge2 = p3 - p1;

    // compute the inverse of the uv difference matrix

    float mDet = (1.f / (uv1.x * uv2.y - uv1.y * uv2.x));

    glm::mat2 mAdjoint = {
        {uv2.y, -uv1.y},
        {-uv2.x, uv1.x}
    };

    glm::mat2 mInv = mDet * mAdjoint;

    glm::vec3 tangent = {mInv[0][0] * edge1 + mInv[0][1] * edge2};
    tangent = glm::normalize(tangent);
    glm::vec3 bitangent = {mInv[1][0] * edge1 + mInv[1][1] * edge2};
    bitangent = glm::normalize(bitangent);

    return std::vector<glm::vec3> {tangent, bitangent};
}