#include "../pch.h"
#include <algorithm>

#include "Intersection.h"

#include "Shape.h"

namespace Scuffed {

	bool Intersection::AabbWithAabb(const glm::vec3& aabb1Pos, const glm::vec3& aabb1HalfSize, const glm::vec3& aabb2Pos, const glm::vec3& aabb2HalfSize) {
		if (glm::abs(aabb1Pos.x - aabb2Pos.x) > (aabb1HalfSize.x + aabb2HalfSize.x)) {
			return false;
		}
		if (glm::abs(aabb1Pos.y - aabb2Pos.y) > (aabb1HalfSize.y + aabb2HalfSize.y)) {
			return false;
		}
		if (glm::abs(aabb1Pos.z - aabb2Pos.z) > (aabb1HalfSize.z + aabb2HalfSize.z)) {
			return false;
		}
		return true;
	}

	bool Intersection::AabbWithAabb(const glm::vec3& aabb1Pos, const glm::vec3& aabb1HalfSize, const glm::vec3& aabb2Pos, const glm::vec3& aabb2HalfSize, glm::vec3* intersectionAxis, float* intersectionDepth) {
		*intersectionDepth = 9999999.0f;

		float tempDepth;
		for (int i = 0; i < 3; i++) {
			tempDepth = (aabb1HalfSize[i] + aabb2HalfSize[i]) - glm::abs(aabb1Pos[i] - aabb2Pos[i]);
			if (tempDepth < 0.0f) {
				return false;
			}
			else if (tempDepth < *intersectionDepth) {
				*intersectionDepth = tempDepth;
				*intersectionAxis = glm::vec3(0.0f);
				(*intersectionAxis)[i] = aabb1Pos[i] - aabb2Pos[i];
				(*intersectionAxis)[i] /= abs((*intersectionAxis)[i]); //Normalize
			}
		}
		return true;
	}

	bool Intersection::AabbWithTriangle(const glm::vec3& aabbPos, const glm::vec3& aabbHalfSize, const glm::vec3& triPos1, const glm::vec3& triPos2, const glm::vec3& triPos3, const bool checkBackfaces) {
		//Calculate normal for triangle
		glm::vec3 triNormal = glm::normalize(glm::cross(glm::vec3(triPos1 - triPos2), glm::vec3(triPos1 - triPos3)));

		// Calculate triangle points relative to the AABB
		glm::vec3 newV1 = triPos1 - aabbPos;
		glm::vec3 newV2 = triPos2 - aabbPos;
		glm::vec3 newV3 = triPos3 - aabbPos;

		//Don't intersect with triangles facing away from the boundingBox
		if (glm::dot(newV1, triNormal) > 0.0f && !checkBackfaces) {
			return false;
		}

		// Calculate the plane that the triangle is on
		glm::vec3 triangleToWorldOrigo = glm::vec3(0.0f) - triPos1;
		float distance = -glm::dot(triangleToWorldOrigo, triNormal);

		// Early exit: Check if a sphere around aabb intersects triangle plane
		if (SphereWithPlane({ aabbPos, glm::length(aabbHalfSize) }, triNormal, distance)) {
			// Testing AABB with triangle using separating axis theorem(SAT)
			glm::vec3 e[3];
			e[0] = glm::vec3(1.f, 0.f, 0.f);
			e[1] = glm::vec3(0.f, 1.f, 0.f);
			e[2] = glm::vec3(0.f, 0.f, 1.f);

			glm::vec3 f[3];
			f[0] = newV2 - newV1;
			f[1] = newV3 - newV2;
			f[2] = newV1 - newV3;

			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					glm::vec3 a = glm::cross(e[i], f[j]);
					glm::vec3 p = glm::vec3(glm::dot(a, newV1), glm::dot(a, newV2), glm::dot(a, newV3));
					float r = aabbHalfSize.x * glm::abs(a.x) + aabbHalfSize.y * glm::abs(a.y) + aabbHalfSize.z * glm::abs(a.z);
					if (glm::min(p.x, glm::min(p.y, p.z)) > r || glm::max(p.x, glm::max(p.y, p.z)) < -r) {
						return false;
					}
				}
			}
		}
		else {
			return false;
		}

		return true;
	}

	bool Intersection::AabbWithTriangle(const glm::vec3& aabbPos, const glm::vec3& aabbHalfSize, const glm::vec3& triPos1, const glm::vec3& triPos2, const glm::vec3& triPos3, glm::vec3* intersectionAxis, float* intersectionDepth) {
		//This version sets the intersection axis for the smallest collision and the intersection depth along that axis.
		float depth = 9999999.0f;
		glm::vec3 tempIntersectionAxis;

		//Calculate normal for triangle
		glm::vec3 triNormal = glm::normalize(glm::cross(glm::vec3(triPos1 - triPos2), glm::vec3(triPos1 - triPos3)));

		// Calculate triangle points relative to the AABB
		glm::vec3 newV1 = triPos1 - aabbPos;
		glm::vec3 newV2 = triPos2 - aabbPos;
		glm::vec3 newV3 = triPos3 - aabbPos;

		//Don't intersect with triangles faceing away from the boundingBox
		if (glm::dot(newV1, triNormal) > 0.0f) {
			return false;
		}

		// Calculate the plane that the triangle is on
		glm::vec3 triangleToWorldOrigo = glm::vec3(0.0f) - triPos1;
		float distance = -glm::dot(triangleToWorldOrigo, triNormal);

		// Early exit: Check if a sphere around aabb intersects triangle plane
		if (SphereWithPlane({ aabbPos, glm::length(aabbHalfSize) }, triNormal, distance)) {
			// Testing AABB with triangle using separating axis theorem(SAT)
			glm::vec3 e[3];
			e[0] = glm::vec3(1.f, 0.f, 0.f);
			e[1] = glm::vec3(0.f, 1.f, 0.f);
			e[2] = glm::vec3(0.f, 0.f, 1.f);

			glm::vec3 f[3];
			f[0] = glm::normalize(newV2 - newV1);
			f[1] = glm::normalize(newV3 - newV2);
			f[2] = glm::normalize(newV1 - newV3);

			glm::vec3 testAxis;

			for (int i = 0; i < 3; i++) {
				testAxis = e[i];

				if (!SATTest(testAxis, newV1, newV2, newV3, aabbHalfSize, &tempIntersectionAxis, &depth)) {
					return false;
				}

				for (int j = 0; j < 3; j++) {
					testAxis = glm::normalize(glm::cross(e[i], f[j]));

					if (!SATTest(testAxis, newV1, newV2, newV3, aabbHalfSize, &tempIntersectionAxis, &depth)) {
						return false;
					}
				}
			}

			testAxis = triNormal;

			if (!SATTest(testAxis, newV1, newV2, newV3, aabbHalfSize, &tempIntersectionAxis, &depth)) {
				return false;
			}

		}
		else {
			return false;
		}


		//Return intersection axis and depth if not nullptr
		if (intersectionAxis) {
			glm::vec3 triMiddle = (triPos1 + triPos2 + triPos3) / 3.0f;

			if (glm::dot(aabbPos - triMiddle, tempIntersectionAxis) < 0.0f) {
				tempIntersectionAxis = -tempIntersectionAxis;
			}

			*intersectionAxis = tempIntersectionAxis;
		}

		if (intersectionDepth) {
			*intersectionDepth = depth;
		}

		return true;
	}

	bool Intersection::AabbWithPlane(const glm::vec3* aabbCorners, const glm::vec3& planeNormal, const float planeDistance) {
		const float distFromPlaneAlongNormal[] = {
			glm::dot(aabbCorners[0], planeNormal) - planeDistance,
			glm::dot(aabbCorners[1], planeNormal) - planeDistance,
			glm::dot(aabbCorners[2], planeNormal) - planeDistance,
			glm::dot(aabbCorners[3], planeNormal) - planeDistance,
			glm::dot(aabbCorners[4], planeNormal) - planeDistance,
			glm::dot(aabbCorners[5], planeNormal) - planeDistance,
			glm::dot(aabbCorners[6], planeNormal) - planeDistance,
			glm::dot(aabbCorners[7], planeNormal) - planeDistance
		};

		// Find smallest and biggest distance (opposing corners)
		float minDist = distFromPlaneAlongNormal[0];
		float maxDist = distFromPlaneAlongNormal[0];
		for (short i = 1; i < 8; i++) {
			minDist = std::fminf(minDist, distFromPlaneAlongNormal[i]);
			maxDist = std::fmaxf(maxDist, distFromPlaneAlongNormal[i]);
		}
		// True if they are on opposite sides of the plane
		return minDist * maxDist < 0.0f;
	}

	bool Intersection::AabbWithSphere(const glm::vec3* aabbCorners, const Sphere& sphere) {
		// Find the point on the aabb closest to the sphere
		float closestOnAabbX = std::fminf(std::fmaxf(sphere.position.x, aabbCorners[0].x), aabbCorners[1].x);
		float closestOnAabbY = std::fminf(std::fmaxf(sphere.position.y, aabbCorners[2].y), aabbCorners[0].y);	// corners[0] > corners[2]
		float closestOnAabbZ = std::fminf(std::fmaxf(sphere.position.z, aabbCorners[0].z), aabbCorners[4].z);

		// Distance from cylinder to closest point on rectangle
		float distX = closestOnAabbX - sphere.position.x;
		float distY = closestOnAabbY - sphere.position.y;
		float distZ = closestOnAabbZ - sphere.position.z;
		float distSquared = distX * distX + distY * distY + distZ * distZ;

		// True if the distance is smaller than the radius
		return (distSquared < sphere.radius * sphere.radius);
	}

	bool Intersection::SphereWithPlane(const Sphere& sphere, const glm::vec3& planeNormal, const float planeDistance) {
		const glm::vec3 pointOnPlane = planeNormal * planeDistance;
		const glm::vec3 centerToPlane = pointOnPlane - sphere.position;
		const float shortestDistanceToPlane = glm::dot(centerToPlane, planeNormal);
		return (std::fabsf(shortestDistanceToPlane) < sphere.radius);
	}

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

	bool Intersection::SATTest(const glm::vec3& testAxis, const glm::vec3& triPos1, const glm::vec3& triPos2, const glm::vec3& triPos3, const glm::vec3& aabbSize, glm::vec3* intersectionAxis, float* depth) {
		glm::vec3 p = glm::vec3(glm::dot(testAxis, triPos1), glm::dot(testAxis, triPos2), glm::dot(testAxis, triPos3));
		float r = aabbSize.x * glm::abs(testAxis.x) + aabbSize.y * glm::abs(testAxis.y) + aabbSize.z * glm::abs(testAxis.z);

		float tempDepth = glm::min(r - glm::min(p.x, glm::min(p.y, p.z)), glm::max(p.x, glm::max(p.y, p.z)) + r);

		if (tempDepth < 0.0f) {
			return false;
		}
		else {
			//Save depth along axis if lower than previous
			if (tempDepth < *depth) {
				*depth = tempDepth;
				*intersectionAxis = testAxis;
			}
		}
		return true;
	}

	float Intersection::projectionOverlapTest(glm::vec3& testVec, const std::vector<glm::vec3>& shape1, const std::vector<glm::vec3>& shape2) {
		testVec = glm::normalize(testVec);
		float min1 = INFINITY, min2 = INFINITY;
		float max1 = -INFINITY, max2 = -INFINITY;

		for (int i = 0; i < shape1.size(); i++) {
			float tempDot1 = glm::dot(shape1[i], testVec);

			if (tempDot1 < min1) {
				min1 = tempDot1;
			}
			if (tempDot1 > max1) {
				max1 = tempDot1;
			}
		}

		for (int i = 0; i < shape2.size(); i++) {
			float tempDot2 = glm::dot(shape2[i], testVec);

			if (tempDot2 < min2) {
				min2 = tempDot2;
			}
			if (tempDot2 > max2) {
				max2 = tempDot2;
			}
		}

		if (max2 > min1 && max1 > min2) {
			return glm::min(max2 - min1, max1 - min2);
		}
		return -1.f;
	}

	bool Intersection::SAT(Shape& shape1, Shape& shape2, glm::vec3& intersectionAxis, float& intersectionDepth) {
		std::vector<glm::vec3> axes;
		std::vector<glm::vec3> s1Norms = shape1.getNormals();
		std::vector<glm::vec3> s2Norms = shape2.getNormals();
		axes.insert(axes.end(), s1Norms.begin(), s1Norms.end());
		axes.insert(axes.end(), s2Norms.begin(), s2Norms.end());

		std::vector<glm::vec3> s1Edges = shape1.getEdges();
		std::vector<glm::vec3> s2Edges = shape2.getEdges();

		for (size_t i = 0; i < s1Edges.size(); i++) {
			for (size_t j = 0; j < s2Edges.size(); j++) {
				axes.emplace_back();
				axes.back() = glm::normalize(glm::cross(s1Edges[i], s2Edges[j]));
			}
		}

		for (size_t i = 0; i < axes.size(); i++) {
			float intersection = projectionOverlapTest(axes[i], shape1.getVertices(), shape2.getVertices());
			if (intersection < 0.f) {
				return false;
			}
			else {
				if (intersection < intersectionDepth) {
					intersectionDepth = intersection;
					intersectionAxis = axes[i];
				}
			}
		}

		return true;
	}

}