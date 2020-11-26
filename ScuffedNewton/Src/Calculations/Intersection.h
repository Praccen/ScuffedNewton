#pragma once

#include <vector>


namespace Scuffed {

	class Shape;
	class Entity;

	class Intersection {
	public:
		struct CollisionTimeInfo {
			float time = -1.0f;
			Entity* entity1 = nullptr;
			Entity* entity2 = nullptr;
			std::vector<std::pair<int, int>> triangleIndices; // Will be empty if mesh collisions aren't used or -1 if not used one one of the meshes

			bool operator<(const CollisionTimeInfo& other) {
				return time < other.time;
			}
		};

	public:
		static float dot(const glm::vec3& v1, const glm::vec3& v2);

		static void getCollisionTime(Entity& e1, Entity& e2, const float timeMax, CollisionTimeInfo& outInfo);
		static bool isColliding(CollisionTimeInfo& info);
		static glm::vec3 getIntersectionAxis(CollisionTimeInfo& info); // Always returns the intersection axis pointing from shape2 towards shape1

		// ----SAT functions----
		static float projectionOverlapTest(const glm::vec3& testVec, const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2, bool &invertAxis);
		static bool SAT(Shape* shape1, Shape* shape2); // TODO: Gives wrong result, investigate why
		static std::vector<glm::vec3> getManifold(const glm::vec3& testVec, const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2);
		static bool SAT(Shape* shape1, Shape* shape2, glm::vec3* intersectionAxis, float* intersectionDepth); // Always returns the intersection axis pointing from shape2 towards shape1
		static bool SAT(Shape* shape1, Shape* shape2, std::vector<glm::vec3>& manifold);

		static bool continousOverlapTest(const glm::vec3& testVec, const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2, const glm::vec3& relativeVel, float& timeFirst, float& timeLast, const float timeMax);
		static float continousSAT(Shape* shape1, Shape* shape2, const glm::vec3& vel1, const glm::vec3& vel2, const float timeMax);


		static glm::vec3 getIntersectionAxis(Shape* shape1, Shape* shape2); // Always returns the intersection axis pointing from shape2 towards shape1

		// ---------------------

		static float RayWithAabb(const glm::vec3& rayStart, const glm::vec3& rayVec, const glm::vec3& aabbPos, const glm::vec3& aabbHalfSize, glm::vec3* intersectionAxis = nullptr);
		static float RayWithTriangle(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& triPos1, const glm::vec3& triPos2, const glm::vec3& triPos3);
		static float RayWithPlane(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& planeNormal, const float planeDistance);

	private:
		//Private constructor so an instance can't be created
		Intersection() {};
		~Intersection() {};

		static void getMeshBoxCollisionTime(Entity& meshE, Entity& boxE, const float timeMax, CollisionTimeInfo& outInfo);
		static void getMeshMeshCollisionTime(Entity& e1, Entity& e2, const float timeMax, CollisionTimeInfo& outInfo);

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