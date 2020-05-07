#pragma once

#include <vector>

#include "../DataTypes/Sphere.h"


namespace Scuffed {

	class Shape;

	class Intersection {
	public:
		static float dot(const glm::vec3& v1, const glm::vec3& v2);

		// ----SAT functions----
		static float projectionOverlapTest(const glm::vec3& testVec, const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2);
		static bool SAT(Shape* shape1, Shape* shape2);
		static std::vector<glm::vec3> getManifold(const glm::vec3& testVec, const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2);
		static bool SAT(Shape* shape1, Shape* shape2, glm::vec3* intersectionAxis, float* intersectionDepth);
		static bool SAT(Shape* shape1, Shape* shape2, std::vector<glm::vec3>& manifold);

		static bool continousOverlapTest(const glm::vec3& testVec, const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2, const glm::vec3& relativeVel, float& timeFirst, float& timeLast, const float timeMax);
		static float continousSAT(Shape* shape1, Shape* shape2, const glm::vec3& vel1, const glm::vec3& vel2, const float dt);
		// ---------------------

	private:
		//Private constructor so an instance can't be created
		Intersection() {};
		~Intersection() {};

		static float RayWithAabb(const glm::vec3& rayStart, const glm::vec3& rayVec, const glm::vec3& aabbPos, const glm::vec3& aabbHalfSize, glm::vec3* intersectionAxis = nullptr);
		static float RayWithTriangle(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& triPos1, const glm::vec3& triPos2, const glm::vec3& triPos3);
		static float RayWithPlane(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& planeNormal, const float planeDistance);
		static float RayWithPaddedAabb(const glm::vec3& rayStart, const glm::vec3& rayVec, const glm::vec3& aabbPos, const glm::vec3& aabbHalfSize, float padding, glm::vec3* intersectionAxis = nullptr);
		static float RayWithPaddedTriangle(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& triPos1, const glm::vec3& triPos2, const glm::vec3& triPos3, float padding, const bool checkBackfaces = false);

		static bool FrustumPlaneWithAabb(const glm::vec3& planeNormal, const float planeDistance, const glm::vec3* aabbCorners);
		//static bool FrustumWithAabb(const Frustum& frustum, const glm::vec3* aabbCorners);

		static glm::vec3 PointProjectedOnPlane(const glm::vec3& point, const glm::vec3& planeNormal, const float planeDistance);

		static void Barycentric(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, float& outU, float& outV, float& outW);
		static bool OnTriangle(const float u, const float v, const float w);
	
		static std::vector<glm::vec3> coPlanarLineSegmentsIntersection(const glm::vec3& l1p1, const glm::vec3& l1p2, const glm::vec3& l2p1, const glm::vec3& l2p2);
		static std::vector<glm::vec3> coPlanarLineSegmentTriangleIntersection(const glm::vec3& lp1, const glm::vec3& lp2, const glm::vec3& tp1, const glm::vec3& tp2, const glm::vec3& tp3);
		static std::vector<glm::vec3> coPlanarLineSegmentQuadIntersection(const glm::vec3& lp1, const glm::vec3& lp2, const glm::vec3& qp1, const glm::vec3& qp2, const glm::vec3& qp3, const glm::vec3& qp4);
		static std::vector<glm::vec3> coPlanarTrianglesIntersection(const glm::vec3& t1p1, const glm::vec3& t1p2, const glm::vec3& t1p3, const glm::vec3& t2p1, const glm::vec3& t2p2, const glm::vec3& t2p3);
		static std::vector<glm::vec3> coPlanarTriangleQuadIntersection(const glm::vec3& tp1, const glm::vec3& tp2, const glm::vec3& tp3, const glm::vec3& qp1, const glm::vec3& qp2, const glm::vec3& qp3, const glm::vec3& qp4);
		static std::vector<glm::vec3> coPlanarQuadsIntersection(const glm::vec3& q1p1, const glm::vec3& q1p2, const glm::vec3& q1p3, const glm::vec3& q1p4, const glm::vec3& q2p1, const glm::vec3& q2p2, const glm::vec3& q2p3, const glm::vec3& q2p4);
	};

}