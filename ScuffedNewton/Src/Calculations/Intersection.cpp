#include "../pch.h"
#include <algorithm>
#include <iterator>

#include "Intersection.h"

#include "../Shapes/Shape.h"

namespace Scuffed {

	float Intersection::RayWithAabb(const glm::vec3& rayStart, const glm::vec3& rayVec, const glm::vec3& aabbPos, const glm::vec3& aabbHalfSize, glm::vec3* intersectionAxis) {
		float returnValue = -1.0f;
		glm::vec3 normalizedRay = glm::normalize(rayVec);
		bool noHit = false; //Boolean for early exits from the for-loop
		float tMin = -std::numeric_limits<float>::infinity(); //tMin initialized at negative infinity
		float tMax = std::numeric_limits<float>::infinity(); //tMax initialized at positive infinity
		glm::vec3 minAxis(0.0f);
		glm::vec3 p = aabbPos - rayStart; //Vector to center off AABB
		for (int i = 0; i < 3; i++) {
			float tempH = aabbHalfSize[i]; //Temporary variable to store the current half axis
			glm::vec3 tempAxis(0.0f);
			tempAxis[i] = tempH;

			float e = p[i];
			float f = normalizedRay[i];
			if (f != 0.0f) { //Ray is not parallel to slab
				float tempF = 1 / f; //temporary variable to avoid calculating division with the (possibly) very small value f multiple times since it is an expensive calculation

				//Finds the entering and exiting points of the current slab.
				float t1 = (e + tempH) * tempF;
				float t2 = (e - tempH) * tempF;
				if (t1 > t2) { //Swaps values if t1 is bigger than t2
					float tempT = t2;
					t2 = t1;
					t1 = tempT;
					tempAxis = -tempAxis;
				}
				if (t1 > tMin) {//Replaces tMin if t1 is bigger
					tMin = t1;
					minAxis = tempAxis;
				}
				if (t2 < tMax) {//Replaces tMax if t2 is smaller
					tMax = t2;
				}
				if (tMin > tMax || tMax < 0) { //tMin > tMax - A slab was exited before all slabs had been entered. tMax < 0 - the AABB is "behind" the ray start
					//Exit test, no hit.
					i = 3;
					noHit = true;
				}
			}
			else if (-e - tempH > 0 || -e + tempH < 0) { //Ray is parallel to slab and it does not go through OBB
				//Exit test, no hit.
				i = 3;
				noHit = true;
			}
		}

		if (noHit == false) { //The AABB was hit by the ray
			if (tMin > 0) { //tMin is bigger than 0 so it is the first intersection
				returnValue = tMin;
			}
			else if (tMax > 0) { //tMin is smaller than 0 so try with tMax.
				returnValue = 0.0f; //Ray started in AABB, instant hit. Distance is 0
			}
		}

		if (intersectionAxis) {
			*intersectionAxis = glm::normalize(minAxis);
		}

		return returnValue;
	}

	float Intersection::RayWithTriangle(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& triPos1, const glm::vec3& triPos2, const glm::vec3& triPos3) {
		const glm::vec3 edge1 = triPos2 - triPos1;
		const glm::vec3 edge2 = triPos3 - triPos1;
		const glm::vec3 planeNormal = glm::normalize(glm::cross(edge1, edge2));

		const float originToPlaneDistance = glm::dot(triPos1, planeNormal);
		const float rayToPlaneDistance = RayWithPlane(rayStart, rayDir, planeNormal, originToPlaneDistance);

		if (rayToPlaneDistance == -1.0f) {
			return -1.0f;
		}

		// Determine barycentric coordinates u, v, w
		float u, v, w;
		const glm::vec3 p = rayStart + rayDir * rayToPlaneDistance;
		Barycentric(p, triPos1, triPos2, triPos3, u, v, w);

		// Check if point on triangle plane is within triangle
		if (OnTriangle(u, v, w)) {
			return rayToPlaneDistance;
		}
		return -1.0f;
	}

	float Intersection::RayWithPlane(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& planeNormal, const float planeDistance) {
		const float dirDotNormal = glm::dot(rayDir, planeNormal);

		bool isParallelWithPlane = std::fabsf(dirDotNormal) < 0.001f;
		if (isParallelWithPlane) {
			return -1.0f;
		}

		const glm::vec3 pointOnPlane = planeNormal * planeDistance;
		const glm::vec3 startToPlane = pointOnPlane - rayStart;
		const float shortestDistanceToPlane = glm::dot(startToPlane, planeNormal);
		const float distanceToPlane = shortestDistanceToPlane / dirDotNormal;

		return distanceToPlane;
	}

	float Intersection::RayWithPaddedAabb(const glm::vec3& rayStart, const glm::vec3& rayVec, const glm::vec3& aabbPos, const glm::vec3& aabbHalfSize, float padding, glm::vec3* intersectionAxis) {
		float returnValue = -1.0f;

		if (padding != 0.0f) {
			//Add padding
			returnValue = RayWithAabb(rayStart, rayVec, aabbPos, aabbHalfSize + glm::vec3(padding), intersectionAxis);
		}
		else {
			returnValue = RayWithAabb(rayStart, rayVec, aabbPos, aabbHalfSize, intersectionAxis);
		}

		return returnValue;
	}

	float Intersection::RayWithPaddedTriangle(const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& triPos1, const glm::vec3& triPos2, const glm::vec3& triPos3, float padding, const bool checkBackfaces) {
		float returnValue = -1.0f;

		glm::vec3 triangleNormal = glm::normalize(glm::cross(glm::vec3(triPos1 - triPos2), glm::vec3(triPos1 - triPos3)));

		if (glm::dot(triPos1 - rayStart, triangleNormal) < 0.0f || checkBackfaces) {
			//Only check if triangle is facing ray start
			if (padding != 0.0f) {
				glm::vec3 oldV[3];
				oldV[0] = triPos1;
				oldV[1] = triPos2;
				oldV[2] = triPos3;

				glm::vec3 newV[3];

				glm::vec3 normalPadding = triangleNormal * padding;

				glm::vec3 middle = ((oldV[0] + oldV[1] + oldV[2]) / 3.0f) + normalPadding;

				for (int i = 0; i < 3; i++) {
					newV[i] = oldV[i] + normalPadding;

					float oldRayDist = glm::dot(rayDir, oldV[i] - rayStart);
					float newRayDist = glm::dot(rayDir, newV[i] - rayStart);

					glm::vec3 oldProjectionOnRayDir = rayStart + rayDir * oldRayDist;
					glm::vec3 newProjectionOnRayDir = rayStart + rayDir * newRayDist;
					float oldNormalDot = glm::dot(oldProjectionOnRayDir - oldV[i], triangleNormal);
					float newNormalDot = glm::dot(newProjectionOnRayDir - newV[i], triangleNormal);

					if ((std::signbit(oldNormalDot) != std::signbit(newNormalDot) && glm::dot(middle - newV[i], rayDir) > 0.0f) || std::signbit(oldRayDist) != std::signbit(newRayDist)) {
						glm::vec3 toRayStart = rayStart - oldV[i];
						float length = glm::min(glm::length(toRayStart), padding) - 0.001f;

						newV[i] = oldV[i] + glm::normalize(toRayStart) * length;
						//newV[i] = oldV[i];
					}
				}

				returnValue = RayWithTriangle(rayStart, rayDir, newV[0], newV[1], newV[2]);
			}
			else {
				returnValue = RayWithTriangle(rayStart, rayDir, triPos1, triPos2, triPos3);
			}
		}

		return returnValue;
	}

	bool Intersection::FrustumPlaneWithAabb(const glm::vec3& planeNormal, const float planeDistance, const glm::vec3* aabbCorners) {
		// Find point on positive side of plane
		for (short i = 0; i < 8; i++) {
			if ((glm::dot(aabbCorners[i], planeNormal) + planeDistance) < 0.0f) {
				return true;
			}
		}
		return false;
	}

	//bool Intersection::FrustumWithAabb(const Frustum& frustum, const glm::vec3* aabbCorners) {
	//	for (int i = 0; i < 6; i++) {
	//		if (!FrustumPlaneWithAabb(glm::vec3(frustum.planes[i].x, frustum.planes[i].y, frustum.planes[i].z), frustum.planes[i].w, aabbCorners)) {
	//			//Aabb is on the wrong side of a plane - it is outside the frustum.
	//			return false;
	//		}
	//	}
	//	return true;
	//}

	glm::vec3 Intersection::PointProjectedOnPlane(const glm::vec3& point, const glm::vec3& planeNormal, const float planeDistance) {
		const glm::vec3 pointOnPlane = planeNormal * planeDistance;
		const glm::vec3 planeToPoint = point - pointOnPlane;
		const float shortestDistanceToPlane = glm::dot(planeToPoint, planeNormal);
		return point - planeNormal * shortestDistanceToPlane;
	}

	void Intersection::Barycentric(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, float& u, float& v, float& w) {
		const glm::vec3 v0 = b - a;
		const glm::vec3 v1 = c - a;
		const glm::vec3 v2 = p - a;

		// Determine barycentric coordinates
		const float d00 = glm::dot(v0, v0);
		const float d01 = glm::dot(v0, v1);
		const float d11 = glm::dot(v1, v1);
		const float d20 = glm::dot(v2, v0);
		const float d21 = glm::dot(v2, v1);
		const float denom = d00 * d11 - d01 * d01;
		const float divDenom = 1.0f / denom;
		u = (d11 * d20 - d01 * d21) * divDenom;
		v = (d00 * d21 - d01 * d20) * divDenom;
		w = 1.0f - u - v;
	}

	bool Intersection::OnTriangle(const float u, const float v, const float w) {
		return ((0.0f <= v && v <= 1.0f) && (0.0f <= w && w <= 1.0f) && (0.0f <= u && u <= 1.0f));
	}

	float Intersection::dot(const glm::vec3& v1, const glm::vec3& v2) {
#ifdef _DEBUG
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
#else
		return glm::dot(v1, v2);
#endif
	}

	float Intersection::projectionOverlapTest(const glm::vec3& testVec, const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2) {
		float min1 = INFINITY, min2 = INFINITY;
		float max1 = -INFINITY, max2 = -INFINITY;

		float tempDot;

		for (const auto& vert : vertices1) {
			tempDot = dot(vert, testVec);

			if (tempDot < min1) {
				min1 = tempDot;
			}
			if (tempDot > max1) {
				max1 = tempDot;
			}
		}

		for (const auto& vert : vertices2) {
			tempDot = dot(vert, testVec);

			if (tempDot < min2) {
				min2 = tempDot;
			}
			if (tempDot > max2) {
				max2 = tempDot;
			}
		}

		if (max2 >= min1 && max1 >= min2) {
			return glm::min(max2 - min1, max1 - min2);
		}
		return -1.f;
	}

	bool Intersection::SAT(Shape* shape1, Shape* shape2) {
		const std::vector<glm::vec3>& s1Norms = shape1->getNormals();
		for (const auto& it : s1Norms) {
			float intersection = projectionOverlapTest(it, shape1->getVertices(), shape2->getVertices());
			if (intersection < 0.f) {
				return false;
			}
		}

		const std::vector<glm::vec3>& s2Norms = shape2->getNormals();
		for (const auto& it : s2Norms) {
			float intersection = projectionOverlapTest(it, shape1->getVertices(), shape2->getVertices());
			if (intersection < 0.f) {
				return false;
			}
		}

		const std::vector<glm::vec3>& s1Edges = shape1->getEdges();
		const std::vector<glm::vec3>& s2Edges = shape2->getEdges();

		glm::vec3 testVec;

		// Calculate cross vectors
		for (const auto& e1 : s1Edges) {
			for (const auto& e2 : s2Edges) {
				if (e1 != e2 && e1 != -e2) {
					testVec = glm::normalize(glm::cross(e1, e2));
					float intersection = projectionOverlapTest(testVec, shape1->getVertices(), shape2->getVertices());
					if (intersection < 0.f) {
						return false;
					}
				}
			}
		}

		return true;
	}

	bool Intersection::SAT(Shape* shape1, Shape* shape2, glm::vec3* intersectionAxis, float* intersectionDepth) {
		*intersectionDepth = INFINITY;

		const std::vector<glm::vec3>& s1Norms = shape1->getNormals();
		for (const auto& it : s1Norms) {
			float intersection = projectionOverlapTest(it, shape1->getVertices(), shape2->getVertices());
			if (intersection < 0.f) {
				return false;
			}
			else {
				// Save smallest 
				if (intersection < *intersectionDepth) {
					*intersectionDepth = intersection;
					*intersectionAxis = it;
				}
			}
		}

		const std::vector<glm::vec3>& s2Norms = shape2->getNormals();
		for (const auto& it : s2Norms) {
			float intersection = projectionOverlapTest(it, shape1->getVertices(), shape2->getVertices());
			if (intersection < 0.f) {
				return false;
			}
			else {
				// Save smallest 
				if (intersection < *intersectionDepth) {
					*intersectionDepth = intersection;
					*intersectionAxis = it;
				}
			}
		}

		const std::vector<glm::vec3>& s1Edges = shape1->getEdges();
		const std::vector<glm::vec3>& s2Edges = shape2->getEdges();

		glm::vec3 testVec;

		// Calculate cross vectors
		for (const auto& e1 : s1Edges) {
			for (const auto& e2 : s2Edges) {
				if (e1 != e2 && e1 != -e2) {
					testVec = glm::normalize(glm::cross(e1, e2));
					float intersection = projectionOverlapTest(testVec, shape1->getVertices(), shape2->getVertices());
					if (intersection < 0.f) {
						return false;
					}
					else {
						// Save smallest 
						if (intersection < *intersectionDepth) {
							*intersectionDepth = intersection;
							*intersectionAxis = testVec;
						}
					}
				}
			}
		}

		return true;
	}

	bool Intersection::continousOverlapTest(const glm::vec3& testVec, const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2, const glm::vec3& relativeVel, float& timeFirst, float& timeLast, const float& timeMax) {
		float min1 = INFINITY, min2 = INFINITY;
		float max1 = -INFINITY, max2 = -INFINITY;

		float tempDot;

		for (const auto& vert : vertices1) {
			tempDot = dot(vert, testVec);

			if (tempDot < min1) {
				min1 = tempDot;
			}
			if (tempDot > max1) {
				max1 = tempDot;
			}
		}

		for (const auto& vert : vertices2) {
			tempDot = dot(vert, testVec);

			if (tempDot < min2) {
				min2 = tempDot;
			}
			if (tempDot > max2) {
				max2 = tempDot;
			}
		}

		//Following found here: https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf

		float T;
		float speed = dot(testVec, relativeVel);

		if (max2 < min1) { // Interval (2) initially on ‘left’ of interval (1)
			if (speed <= 0) { return false; } // Intervals moving apart

			T = (min1 - max2) / speed;
			if (T > timeFirst) { timeFirst = T; }
			if (timeFirst > timeMax) { return false; } // Early exit

			T = (max1 - min2) / speed;
			if (T < timeLast) { timeLast = T; }
			if (timeFirst > timeLast) { return false; } // Early exit
		}
		else  if (max1 < min2) { // Interval (2) initially on ‘right’ of interval (1)
			if (speed >= 0) { return false; } // Intervals moving apart

			T = (max1 - min2) / speed;
			if (T > timeFirst) { timeFirst = T; }
			if (timeFirst > timeMax) { return false; } // Early exit

			T = (min1 - max2) / speed;
			if (T < timeLast) { timeLast = T; }
			if (timeFirst > timeLast) { return false; } // Early exit
		}
		else { // Interval (1) and interval (2) overlap
			if (speed > 0) {
				T = (max1 - min2) / speed;
				if (T < timeLast) { timeLast = T; }
				if (timeFirst > timeLast) { return false; } // Early exit
			}
			else if (speed < 0) {
				T = (min1 - max2) / speed;
				if (T < timeLast) { timeLast = T; }
				if (timeFirst > timeLast) { return false; } // Early exit
			}
		}

		return true;
	}

	float Intersection::continousSAT(Shape* shape1, Shape* shape2, const glm::vec3& vel1, const glm::vec3& vel2, const float& dt) {
		// Treat shape1 as stationary and shape2 as moving
		glm::vec3 relativeVel = vel2 - vel1;

		float timeFirst = 0;
		float timeLast = INFINITY;

		const std::vector<glm::vec3>& s1Norms = shape1->getNormals();
		for (const auto& it : s1Norms) {
			if (!continousOverlapTest(it, shape1->getVertices(), shape2->getVertices(), relativeVel, timeFirst, timeLast, dt)) {
				return -1.0f;
			}
		}

		const std::vector<glm::vec3>& s2Norms = shape2->getNormals();
		for (const auto& it : s2Norms) {
			if (!continousOverlapTest(it, shape1->getVertices(), shape2->getVertices(), relativeVel, timeFirst, timeLast, dt)) {
				return -1.0f;
			}
		}

		const std::vector<glm::vec3>& s1Edges = shape1->getEdges();
		const std::vector<glm::vec3>& s2Edges = shape2->getEdges();

		glm::vec3 testVec;

		// Calculate cross vectors
		for (const auto& e1 : s1Edges) {
			for (const auto& e2 : s2Edges) {
				if (e1 != e2 && e1 != -e2) {
					testVec = glm::normalize(glm::cross(e1, e2));
					if (!continousOverlapTest(testVec, shape1->getVertices(), shape2->getVertices(), relativeVel, timeFirst, timeLast, dt)) {
						return -1.0f;
					}
				}
			}
		}

		return timeFirst;
	}
}