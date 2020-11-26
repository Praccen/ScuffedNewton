#include "../pch.h"
#include <algorithm>
#include <iterator>
#include <map>

#include "Intersection.h"

#include "../Shapes/Shapes.h"
#include "../Components/Components.h"
#include "../DataTypes/Entity.h"
#include "../DataTypes/Mesh.h"

namespace Scuffed {

	float Intersection::dot(const glm::vec3& v1, const glm::vec3& v2) {
#ifdef _DEBUG
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
#else
		return glm::dot(v1, v2);
#endif
	}

	void Intersection::getCollisionTime(Entity& e1, Entity& e2, const float timeMax, CollisionTimeInfo& outInfo) {
		// Entities will at least have physicalBodyComponents and boundingBoxComponents
		PhysicalBodyComponent* e1Phys = e1.getComponent<PhysicalBodyComponent>();
		PhysicalBodyComponent* e2Phys = e2.getComponent<PhysicalBodyComponent>();

		BoundingBoxComponent* e1BoundingBox = e1.getComponent<BoundingBoxComponent>();
		BoundingBoxComponent* e2BoundingBox = e2.getComponent<BoundingBoxComponent>();

		float boxesTime = continousSAT(e1BoundingBox->getBoundingBox(), e2BoundingBox->getBoundingBox(), e1Phys->velocity, e2Phys->velocity, timeMax);

		if (boxesTime < 0.0f) {
			outInfo.time = -1.0f;
			return;
		}

		// Check for meshes
		MeshComponent* e1Mesh = e1.getComponent<MeshComponent>();
		MeshComponent* e2Mesh = e2.getComponent<MeshComponent>();

		if (e1Mesh && e1Mesh->useMeshCollision && e2Mesh && e2Mesh->useMeshCollision) {
			// e1Mesh - e2Mesh collision
			getMeshMeshCollisionTime(e1, e2, timeMax, outInfo);
		}
		else if (e1Mesh && e1Mesh->useMeshCollision) {
			// e1Mesh - e2Box collision
			getMeshBoxCollisionTime(e1, e2, timeMax, outInfo);
		}
		else if (e2Mesh && e2Mesh->useMeshCollision) {
			// e1Box - e2Mesh collision
			getMeshBoxCollisionTime(e2, e1, timeMax, outInfo);
		}
		else {
			// e1Box - e2Box collision
			outInfo.time = boxesTime;
			outInfo.entity1 = &e1;
			outInfo.entity2 = &e2;
			outInfo.triangleIndices.clear();
		}
	}

	bool Intersection::isColliding(CollisionTimeInfo& info) {
		bool returnValue = false;
		TransformComponent* e1Transform = info.entity1->getComponent<TransformComponent>();
		TransformComponent* e2Transform = info.entity1->getComponent<TransformComponent>();

		glm::mat4 transformMatrix1(1.0f);
		if (e1Transform) {
			transformMatrix1 = e1Transform->getMatrixWithUpdate();
		}

		glm::mat4 transformMatrix2(1.0f);
		if (e2Transform) {
			transformMatrix2 = e2Transform->getMatrixWithUpdate();
		}

		if (info.triangleIndices.size() > 0 && info.triangleIndices[0].first >= 0 && info.triangleIndices[0].second >= 0) {
			// Triangle-triangle 
			MeshComponent* e1Mesh = info.entity1->getComponent<MeshComponent>();
			MeshComponent* e2Mesh = info.entity2->getComponent<MeshComponent>();

			Triangle triangle1(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
			Triangle triangle2(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
			triangle2.setBaseMatrix(transformMatrix2);
			triangle2.setMatrix(glm::inverse(transformMatrix1));

			if (e1Mesh->mesh->getNumberOfIndices() > 0) { // Has indices
				triangle1.setData(e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(info.triangleIndices[0].first)), e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(info.triangleIndices[0].first + 1)), e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(info.triangleIndices[0].first + 2)));
			}
			else if (e1Mesh->mesh->getNumberOfVertices() > 0) {
				triangle1.setData(e1Mesh->mesh->getVertexPosition(info.triangleIndices[0].first), e1Mesh->mesh->getVertexPosition(info.triangleIndices[0].first + 1), e1Mesh->mesh->getVertexPosition(info.triangleIndices[0].first + 2));
			}

			if (e2Mesh->mesh->getNumberOfIndices() > 0) { // Has indices
				triangle2.setData(e2Mesh->mesh->getVertexPosition(e2Mesh->mesh->getVertexIndex(info.triangleIndices[0].second)), e2Mesh->mesh->getVertexPosition(e2Mesh->mesh->getVertexIndex(info.triangleIndices[0].second + 1)), e2Mesh->mesh->getVertexPosition(e2Mesh->mesh->getVertexIndex(info.triangleIndices[0].second + 2)));
			}
			else if (e2Mesh->mesh->getNumberOfVertices() > 0) {
				triangle2.setData(e2Mesh->mesh->getVertexPosition(info.triangleIndices[0].second), e2Mesh->mesh->getVertexPosition(info.triangleIndices[0].second + 1), e2Mesh->mesh->getVertexPosition(info.triangleIndices[0].second + 2));
			}

			returnValue = continousSAT(&triangle1, &triangle2, glm::vec3(0.0f), glm::vec3(0.0f), 0.0f) == 0.0f;
		}
		else if (info.triangleIndices.size() > 0 && info.triangleIndices[0].first >= 0 && info.triangleIndices[0].second == -1) { // If one is -1 it will always be second (entity2)
			// Triangle-box
			MeshComponent* e1Mesh = info.entity1->getComponent<MeshComponent>();
			Triangle triangle1(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
			if (e1Mesh->mesh->getNumberOfIndices() > 0) { // Has indices
				triangle1.setData(e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(info.triangleIndices[0].first)), e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(info.triangleIndices[0].first + 1)), e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(info.triangleIndices[0].first + 2)));
			}
			else if (e1Mesh->mesh->getNumberOfVertices() > 0) {
				triangle1.setData(e1Mesh->mesh->getVertexPosition(info.triangleIndices[0].first), e1Mesh->mesh->getVertexPosition(info.triangleIndices[0].first + 1), e1Mesh->mesh->getVertexPosition(info.triangleIndices[0].first + 2));
			}
			Box* e2Box = info.entity2->getComponent<BoundingBoxComponent>()->getBoundingBox();
			e2Box->setMatrix(glm::inverse(transformMatrix1));
			returnValue = continousSAT(&triangle1, e2Box, glm::vec3(0.0f), glm::vec3(0.0f), 0.0f) == 0.0f;
			e2Box->setMatrix(glm::mat4(1.f));
		}
		else {
			// Box-box
			// TODO: Investigate why continousSAT is more stable than SAT
			returnValue = continousSAT(info.entity1->getComponent<BoundingBoxComponent>()->getBoundingBox(), info.entity2->getComponent<BoundingBoxComponent>()->getBoundingBox(), glm::vec3(0.0f), glm::vec3(0.0f), 0.0f) == 0.0f;
		}

		return returnValue;
	}

	glm::vec3 Intersection::getIntersectionAxis(CollisionTimeInfo& info) {
		glm::vec3 returnValue(0.f);
		TransformComponent* e1Transform = info.entity1->getComponent<TransformComponent>();
		TransformComponent* e2Transform = info.entity1->getComponent<TransformComponent>();

		glm::mat4 transformMatrix1(1.0f);
		if (e1Transform) {
			transformMatrix1 = e1Transform->getMatrixWithUpdate();
		}

		glm::mat4 transformMatrix2(1.0f);
		if (e2Transform) {
			transformMatrix2 = e2Transform->getMatrixWithUpdate();
		}

		if (info.triangleIndices.size() > 0 && info.triangleIndices[0].first >= 0 && info.triangleIndices[0].second >= 0) {
			// Triangle-triangle 
			MeshComponent* e1Mesh = info.entity1->getComponent<MeshComponent>();
			MeshComponent* e2Mesh = info.entity2->getComponent<MeshComponent>();

			Triangle triangle1(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
			Triangle triangle2(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
			triangle1.setBaseMatrix(transformMatrix1);
			triangle2.setBaseMatrix(transformMatrix2);


			if (e1Mesh->mesh->getNumberOfIndices() > 0) { // Has indices
				triangle1.setData(e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(info.triangleIndices[0].first)), e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(info.triangleIndices[0].first + 1)), e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(info.triangleIndices[0].first + 2)));
			}
			else if (e1Mesh->mesh->getNumberOfVertices() > 0) {
				triangle1.setData(e1Mesh->mesh->getVertexPosition(info.triangleIndices[0].first), e1Mesh->mesh->getVertexPosition(info.triangleIndices[0].first + 1), e1Mesh->mesh->getVertexPosition(info.triangleIndices[0].first + 2));
			}

			if (e2Mesh->mesh->getNumberOfIndices() > 0) { // Has indices
				triangle2.setData(e2Mesh->mesh->getVertexPosition(e2Mesh->mesh->getVertexIndex(info.triangleIndices[0].second)), e2Mesh->mesh->getVertexPosition(e2Mesh->mesh->getVertexIndex(info.triangleIndices[0].second + 1)), e2Mesh->mesh->getVertexPosition(e2Mesh->mesh->getVertexIndex(info.triangleIndices[0].second + 2)));
			}
			else if (e2Mesh->mesh->getNumberOfVertices() > 0) {
				triangle2.setData(e2Mesh->mesh->getVertexPosition(info.triangleIndices[0].second), e2Mesh->mesh->getVertexPosition(info.triangleIndices[0].second + 1), e2Mesh->mesh->getVertexPosition(info.triangleIndices[0].second + 2));
			}

			returnValue = getIntersectionAxis(&triangle1, &triangle2);
		}
		else if (info.triangleIndices.size() > 0 && info.triangleIndices[0].first >= 0 && info.triangleIndices[0].second == -1) { // If one is -1 it will always be second (entity2)
			// Triangle-box
			MeshComponent* e1Mesh = info.entity1->getComponent<MeshComponent>();
			Triangle triangle1(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
			triangle1.setBaseMatrix(transformMatrix1);
			if (e1Mesh->mesh->getNumberOfIndices() > 0) { // Has indices
				triangle1.setData(e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(info.triangleIndices[0].first)), e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(info.triangleIndices[0].first + 1)), e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(info.triangleIndices[0].first + 2)));
			}
			else if (e1Mesh->mesh->getNumberOfVertices() > 0) {
				triangle1.setData(e1Mesh->mesh->getVertexPosition(info.triangleIndices[0].first), e1Mesh->mesh->getVertexPosition(info.triangleIndices[0].first + 1), e1Mesh->mesh->getVertexPosition(info.triangleIndices[0].first + 2));
			}
			Box* e2Box = info.entity2->getComponent<BoundingBoxComponent>()->getBoundingBox();
			returnValue = getIntersectionAxis(&triangle1, e2Box);
		}
		else {
			// Box-box
			returnValue = getIntersectionAxis(info.entity1->getComponent<BoundingBoxComponent>()->getBoundingBox(), info.entity2->getComponent<BoundingBoxComponent>()->getBoundingBox());
		}

		return returnValue;
	}

	float Intersection::projectionOverlapTest(const glm::vec3& testVec, const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2, bool& invertAxis) {
		invertAxis = false;
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

		if (max2 - min1 < max1 - min2) {
			return max2 - min1;
		}
		else {
			invertAxis = true;
			return max1 - min2;
		}
	}

	bool Intersection::SAT(Shape* shape1, Shape* shape2) {
		bool invertAxis = false;
		const std::vector<glm::vec3>& s1Norms = shape1->getNormals();
		for (const auto& it : s1Norms) {
			float intersection = projectionOverlapTest(it, shape1->getVertices(), shape2->getVertices(), invertAxis);
			if (intersection < 0.f) {
				return false;
			}
		}

		const std::vector<glm::vec3>& s2Norms = shape2->getNormals();
		for (const auto& it : s2Norms) {
			float intersection = projectionOverlapTest(it, shape1->getVertices(), shape2->getVertices(), invertAxis);
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
					float intersection = projectionOverlapTest(testVec, shape1->getVertices(), shape2->getVertices(), invertAxis);
					if (intersection < 0.f) {
						return false;
					}
				}
			}
		}

		return true;
	}

	std::vector<glm::vec3> Intersection::getManifold(const glm::vec3& testVec, const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2) {
		float min1 = INFINITY, min2 = INFINITY;
		float max1 = -INFINITY, max2 = -INFINITY;

		std::vector<glm::vec3> min1Points, min2Points, max1Points, max2Points, manifold;

		float tempDot;

		for (const auto& vert : vertices1) {
			tempDot = dot(vert, testVec);

			if (tempDot < min1) {
				min1 = tempDot;
				min1Points.clear();
				min1Points.emplace_back(vert);
			}
			else if (tempDot == min1) {
				min1Points.emplace_back(vert);
			}
			if (tempDot > max1) {
				max1 = tempDot;
				max1Points.clear();
				max1Points.emplace_back(vert);
			}
			else if (tempDot == max1) {
				max1Points.emplace_back(vert);
			}
		}

		for (const auto& vert : vertices2) {
			tempDot = dot(vert, testVec);

			if (tempDot < min2) {
				min2 = tempDot;
				min2Points.clear();
				min2Points.emplace_back(vert);
			}
			else if (tempDot == min2) {
				min2Points.emplace_back(vert);
			}
			if (tempDot > max2) {
				max2 = tempDot;
				max2Points.clear();
				max2Points.emplace_back(vert);
			}
			else if (tempDot == max2) {
				max2Points.emplace_back(vert);
			}
		}

		std::vector<glm::vec3> points1, points2;
		if (max2 - min1 < max1 - min2) { // Use max2Points and min1Points
			points1 = min1Points;
			points2 = max2Points;
		}
		else { // Use max1Points and min2Points
			points1 = max1Points;
			points2 = min2Points;
		}


		if (points1.size() == 1) {
			// Point - something intersection. Use the point as contact set
			manifold.emplace_back(points1[0]);
		}
		else if (points2.size() == 1) {
			// Point - something intersection. Use the point as contact set
			manifold.emplace_back(points2[0]);
		}
		else if (points1.size() == 2) {
			if (points2.size() == 2) {
				// Line segment - line segment
				manifold = coPlanarLineSegmentsIntersection(points1[0], points1[1], points2[0], points2[1]);
			}
			else if (points2.size() == 3) {
				// Line - Triangle intersection
				manifold = coPlanarLineSegmentTriangleIntersection(points1[0], points1[1], points2[0], points2[1], points2[2]);
			}
			else if (points2.size() == 4) {
				// Line - Quad intersection
				manifold = coPlanarLineSegmentQuadIntersection(points1[0], points1[1], points2[0], points2[1], points2[2], points2[3]);
			}
		}
		else if (points2.size() == 2) {
			if (points1.size() == 3) {
				// Line - Triangle intersection
				manifold = coPlanarLineSegmentTriangleIntersection(points2[0], points2[1], points1[0], points1[1], points1[2]);
			}
			else if (points1.size() == 4) {
				// Line - Quad intersection
				manifold = coPlanarLineSegmentQuadIntersection(points2[0], points2[1], points1[0], points1[1], points1[2], points1[3]);
			}
		}
		else if (points1.size() == 3) {
			if (points2.size() == 3) {
				// Triangle - triangle intersection
				manifold = coPlanarTrianglesIntersection(points1[0], points1[1], points1[2], points2[0], points2[1], points2[2]);
			}
			else if (points2.size() == 4) {
				// Triangle - Quad intersection
				manifold = coPlanarTriangleQuadIntersection(points1[0], points1[1], points1[2], points2[0], points2[1], points2[2], points2[3]);
			}
		}
		else if (points2.size() == 3) {
			if (points1.size() == 4) {
				// Triangle - Quad intersection
				manifold = coPlanarTriangleQuadIntersection(points2[0], points2[1], points2[2], points1[0], points1[1], points1[2], points1[3]);
			}
		}
		else if (points1.size() == 4) {
			if (points2.size() == 4) {
				// Quad - Quad intersection
				manifold = coPlanarQuadsIntersection(points1[0], points1[1], points1[2], points1[3], points2[0], points2[1], points2[2], points2[3]);
			}
		}

		return manifold;
	}

	bool Intersection::SAT(Shape* shape1, Shape* shape2, std::vector<glm::vec3>& manifold) {
		bool invertAxis = false;

		float intersectionDepth = INFINITY;
		glm::vec3 intersectionAxis(0.f);

		const std::vector<glm::vec3>& s1Norms = shape1->getNormals();
		for (const auto& it : s1Norms) {
			float intersection = projectionOverlapTest(it, shape1->getVertices(), shape2->getVertices(), invertAxis);
			if (intersection < 0.f) {
				return false;
			}
			else {
				// Save smallest 
				if (intersection < intersectionDepth) {
					intersectionDepth = intersection;
					intersectionAxis = it;
					if (invertAxis) {
						intersectionAxis = -intersectionAxis;
					}
				}
			}
		}

		const std::vector<glm::vec3>& s2Norms = shape2->getNormals();
		for (const auto& it : s2Norms) {
			float intersection = projectionOverlapTest(it, shape1->getVertices(), shape2->getVertices(), invertAxis);
			if (intersection < 0.f) {
				return false;
			}
			else {
				// Save smallest 
				if (intersection < intersectionDepth) {
					intersectionDepth = intersection;
					intersectionAxis = it;
					if (invertAxis) {
						intersectionAxis = -intersectionAxis;
					}
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
					float intersection = projectionOverlapTest(testVec, shape1->getVertices(), shape2->getVertices(), invertAxis);
					if (intersection < 0.f) {
						return false;
					}
					else {
						// Save smallest 
						if (intersection < intersectionDepth) {
							intersectionDepth = intersection;
							intersectionAxis = testVec;
							if (invertAxis) {
								intersectionAxis = -intersectionAxis;
							}
						}
					}
				}
			}
		}

		manifold = getManifold(intersectionAxis, shape1->getVertices(), shape2->getVertices());

		return true;
	}

	bool Intersection::SAT(Shape* shape1, Shape* shape2, glm::vec3* intersectionAxis, float* intersectionDepth) {
		bool invertAxis = false;
		*intersectionDepth = INFINITY;

		const std::vector<glm::vec3>& s1Norms = shape1->getNormals();
		for (const auto& it : s1Norms) {
			float intersection = projectionOverlapTest(it, shape1->getVertices(), shape2->getVertices(), invertAxis);
			if (intersection < 0.f) {
				return false;
			}
			else {
				// Save smallest 
				if (intersection < *intersectionDepth) {
					*intersectionDepth = intersection;
					*intersectionAxis = it;
					if (invertAxis) {
						*intersectionAxis = -*intersectionAxis;
					}
				}
			}
		}

		const std::vector<glm::vec3>& s2Norms = shape2->getNormals();
		for (const auto& it : s2Norms) {
			float intersection = projectionOverlapTest(it, shape1->getVertices(), shape2->getVertices(), invertAxis);
			if (intersection < 0.f) {
				return false;
			}
			else {
				// Save smallest 
				if (intersection < *intersectionDepth) {
					*intersectionDepth = intersection;
					*intersectionAxis = it;
					if (invertAxis) {
						*intersectionAxis = -*intersectionAxis;
					}
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
					testVec = glm::cross(e1, e2);
					if (glm::length2(testVec) > 0.f) {
						testVec = glm::normalize(testVec);

						float intersection = projectionOverlapTest(testVec, shape1->getVertices(), shape2->getVertices(), invertAxis);
						if (intersection < 0.f) {
							return false;
						}
						else {
							// Save smallest 
							if (intersection < *intersectionDepth) {
								*intersectionDepth = intersection;
								*intersectionAxis = testVec;
								if (invertAxis) {
									*intersectionAxis = -*intersectionAxis;
								}
							}
						}
					}
				}
			}
		}

		return true;
	}

	bool Intersection::continousOverlapTest(const glm::vec3& testVec, const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2, const glm::vec3& relativeVel, float& timeFirst, float& timeLast, const float timeMax) {
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

		if (max2 <= min1) { // Interval (2) initially on ‘left’ of interval (1)
			if (speed <= 0.f) { return false; } // Intervals moving apart

			T = (min1 - max2) / speed;
			if (T > timeFirst) { timeFirst = T; }
			if (timeFirst > timeMax) { return false; } // Early exit

			T = (max1 - min2) / speed;
			if (T < timeLast) { timeLast = T; }
			if (timeFirst > timeLast) { return false; } // Early exit
		}
		else  if (max1 <= min2) { // Interval (2) initially on ‘right’ of interval (1)
			if (speed >= 0.f) { return false; } // Intervals moving apart

			T = (max1 - min2) / speed;
			if (T > timeFirst) { timeFirst = T; }
			if (timeFirst > timeMax) { return false; } // Early exit

			T = (min1 - max2) / speed;
			if (T < timeLast) { timeLast = T; }
			if (timeFirst > timeLast) { return false; } // Early exit
		}
		else { // Interval (1) and interval (2) overlap
			if (speed > 0.f) {
				T = (max1 - min2) / speed;
				if (T < timeLast) { timeLast = T; }
				if (timeFirst > timeLast) { return false; } // Early exit
			}
			else if (speed < 0.f) {
				T = (min1 - max2) / speed;
				if (T < timeLast) { timeLast = T; }
				if (timeFirst > timeLast) { return false; } // Early exit
			}
		}

		return true;
	}

	float Intersection::continousSAT(Shape* shape1, Shape* shape2, const glm::vec3& vel1, const glm::vec3& vel2, const float timeMax) {

		// Treat shape1 as stationary and shape2 as moving
		glm::vec3 relativeVel = vel2 - vel1;

		float timeFirst = 0.f;
		float timeLast = INFINITY;

		const std::vector<glm::vec3>& s1Norms = shape1->getNormals();
		for (const auto& it : s1Norms) {
			if (!continousOverlapTest(it, shape1->getVertices(), shape2->getVertices(), relativeVel, timeFirst, timeLast, timeMax)) {
				return -1.0f;
			}
		}

		const std::vector<glm::vec3>& s2Norms = shape2->getNormals();
		for (const auto& it : s2Norms) {
			if (!continousOverlapTest(it, shape1->getVertices(), shape2->getVertices(), relativeVel, timeFirst, timeLast, timeMax)) {
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
					if (!continousOverlapTest(testVec, shape1->getVertices(), shape2->getVertices(), relativeVel, timeFirst, timeLast, timeMax)) {
						return -1.0f;
					}
				}
			}
		}

		return timeFirst;
	}

	glm::vec3 Intersection::getIntersectionAxis(Shape* shape1, Shape* shape2) {
		// Always returns the intersection axis pointing from shape2 towards shape1
		bool invertAxis = false;
		glm::vec3 intersectionAxis(0.0f);
		float depth = INFINITY;

		const std::vector<glm::vec3>& s1Norms = shape1->getNormals();
		for (const auto& it : s1Norms) {
			float intersection = projectionOverlapTest(it, shape1->getVertices(), shape2->getVertices(), invertAxis);
			// Save smallest 
			if (intersection < depth) {
				intersectionAxis = it;
				depth = intersection;
				if (invertAxis) {
					intersectionAxis = -intersectionAxis;
				}
			}
		}

		const std::vector<glm::vec3>& s2Norms = shape2->getNormals();
		for (const auto& it : s2Norms) {
			float intersection = projectionOverlapTest(it, shape1->getVertices(), shape2->getVertices(), invertAxis);
			// Save smallest 
			if (intersection < depth) {
				intersectionAxis = it;
				depth = intersection;
				if (invertAxis) {
					intersectionAxis = -intersectionAxis;
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
					testVec = glm::cross(e1, e2);
					if (glm::length2(testVec) > 0.f) {
						testVec = glm::normalize(testVec);

						float intersection = projectionOverlapTest(testVec, shape1->getVertices(), shape2->getVertices(), invertAxis);
						// Save smallest 
						if (intersection < depth) {
							intersectionAxis = testVec;
							depth = intersection;
							if (invertAxis) {
								intersectionAxis = -intersectionAxis;
							}
						}
					}
				}
			}
		}

		return intersectionAxis;
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

	void Intersection::getMeshBoxCollisionTime(Entity& meshE, Entity& boxE, const float timeMax, CollisionTimeInfo& outInfo) {
		outInfo.entity1 = &meshE;
		outInfo.entity2 = &boxE;

		PhysicalBodyComponent* meshPhys = meshE.getComponent<PhysicalBodyComponent>();
		PhysicalBodyComponent* boxPhys = boxE.getComponent<PhysicalBodyComponent>();

		MeshComponent* mesh = meshE.getComponent<MeshComponent>();
		BoundingBoxComponent* box = boxE.getComponent<BoundingBoxComponent>();

		TransformComponent* transform = meshE.getComponent<TransformComponent>();

		glm::mat4 transformMatrix(1.0f);
		if (transform) {
			transformMatrix = transform->getMatrixWithUpdate();
		}

		box->getBoundingBox()->setMatrix(glm::inverse(transformMatrix));

		//Convert velocities to local space for mesh
		glm::vec3 zeroPoint = glm::inverse(transformMatrix) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec3 boxVel = glm::inverse(transformMatrix) * glm::vec4(boxPhys->velocity, 1.0f);
		boxVel = boxVel - zeroPoint;
		glm::vec3 meshVel = glm::inverse(transformMatrix) * glm::vec4(meshPhys->velocity, 1.0f);
		meshVel = meshVel - zeroPoint;

		// Get nodes to test continous collision against from narrow phase octree in mesh
		std::vector<Mesh::OctNode*> nodes;
		mesh->mesh->getCollidingNodesContinous(nodes, box->getBoundingBox(), boxVel, meshVel, timeMax);

		// Triangle to set mesh data to avoid creating new shapes for each triangle in mesh
		Triangle triangle(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));

		float time = INFINITY;

		for (size_t i = 0; i < nodes.size(); i++) {
			int numTriangles = nodes[i]->nrOfTriangles;
			bool hasIndices = mesh->mesh->getNumberOfIndices() > 0;
			bool hasVertices = mesh->mesh->getNumberOfVertices() > 0;

			for (int j = 0; j < numTriangles; j++) {
				if (hasIndices) { // Has indices
					triangle.setData(mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(nodes[i]->triangles[j])), mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(nodes[i]->triangles[j] + 1)), mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(nodes[i]->triangles[j] + 2)));
				}
				else if (hasVertices) {
					triangle.setData(mesh->mesh->getVertexPosition(nodes[i]->triangles[j]), mesh->mesh->getVertexPosition(nodes[i]->triangles[j] + 1), mesh->mesh->getVertexPosition(nodes[i]->triangles[j] + 2));
				}

				float newTime = Intersection::continousSAT(box->getBoundingBox(), &triangle, boxVel, meshVel, std::min(time, timeMax));

				if (newTime > 0.f && newTime < time) {
					time = newTime;
					outInfo.triangleIndices.clear();
					outInfo.triangleIndices.emplace_back(std::pair<int, int>(nodes[i]->triangles[j], -1));
				}
				else if (newTime == time) {
					outInfo.triangleIndices.emplace_back(std::pair<int, int>(nodes[i]->triangles[j], -1));
				}
			}
		}
		box->getBoundingBox()->setMatrix(glm::mat4(1.0f)); //Reset bounding box matrix to identity
		outInfo.time = time;
	}

	void Intersection::getMeshMeshCollisionTime(Entity& e1, Entity& e2, const float timeMax, CollisionTimeInfo& outInfo) {
		outInfo.entity1 = &e1;
		outInfo.entity2 = &e2;

		// Entities will at least have physicalBodyComponents and boundingBoxComponents
		PhysicalBodyComponent* e1Phys = e1.getComponent<PhysicalBodyComponent>();
		PhysicalBodyComponent* e2Phys = e2.getComponent<PhysicalBodyComponent>();

		BoundingBoxComponent* e1Box = e1.getComponent<BoundingBoxComponent>();
		BoundingBoxComponent* e2Box = e2.getComponent<BoundingBoxComponent>();

		// Check for meshes
		MeshComponent* e1Mesh = e1.getComponent<MeshComponent>();
		MeshComponent* e2Mesh = e2.getComponent<MeshComponent>();

		TransformComponent* e1Transform = e1.getComponent<TransformComponent>();
		TransformComponent* e2Transform = e2.getComponent<TransformComponent>();

		// ----Nodes for mesh1----
		glm::mat4 transformMatrix1(1.0f);
		if (e1Transform) {
			transformMatrix1 = e1Transform->getMatrixWithUpdate();
		}

		e2Box->getBoundingBox()->setMatrix(glm::inverse(transformMatrix1));

		//Convert velocities to local space for mesh
		glm::vec3 zeroPoint = glm::inverse(transformMatrix1) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec3 newEntityVel = glm::inverse(transformMatrix1) * glm::vec4(e2Phys->velocity, 1.0f);
		newEntityVel = newEntityVel - zeroPoint;
		glm::vec3 otherEntityVel = glm::inverse(transformMatrix1) * glm::vec4(e1Phys->velocity, 1.0f);
		otherEntityVel = otherEntityVel - zeroPoint;

		// Get nodes to test continous collision against from narrow phase octree in mesh
		std::vector<Mesh::OctNode*> nodes1;
		e1Mesh->mesh->getCollidingNodesContinous(nodes1, e2Box->getBoundingBox(), newEntityVel, otherEntityVel, timeMax);

		e2Box->getBoundingBox()->setMatrix(glm::mat4(1.0f)); //Reset bounding box matrix to identity
		// -----------------------

		// ----Nodes for mesh2----
		glm::mat4 transformMatrix2 (1.0f);
		if (e2Transform) {
			transformMatrix2 = e2Transform->getMatrixWithUpdate();
		}

		e1Box->getBoundingBox()->setMatrix(glm::inverse(transformMatrix2));

		//Convert velocities to local space for mesh
		zeroPoint = glm::inverse(transformMatrix2) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		newEntityVel = glm::inverse(transformMatrix2) * glm::vec4(e1Phys->velocity, 1.0f);
		newEntityVel = newEntityVel - zeroPoint;
		otherEntityVel = glm::inverse(transformMatrix2) * glm::vec4(e2Phys->velocity, 1.0f);
		otherEntityVel = otherEntityVel - zeroPoint;

		// Get nodes to test continous collision against from narrow phase octree in mesh
		std::vector<Mesh::OctNode*> nodes2;
		e2Mesh->mesh->getCollidingNodesContinous(nodes2, e1Box->getBoundingBox(), newEntityVel, otherEntityVel, timeMax);

		e1Box->getBoundingBox()->setMatrix(glm::mat4(1.0f)); //Reset bounding box matrix to identity
		// -----------------------

		float time = INFINITY;

		// Triangle to set mesh data to avoid creating new shapes for each triangle in mesh
		Triangle triangle1(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
		Triangle triangle2(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
		triangle2.setBaseMatrix(transformMatrix2);
		triangle2.setMatrix(glm::inverse(transformMatrix1));

		bool hasIndices1 = e1Mesh ->mesh->getNumberOfIndices() > 0;
		bool hasVertices1 = e1Mesh->mesh->getNumberOfVertices() > 0;
		bool hasIndices2 = e2Mesh->mesh->getNumberOfIndices() > 0;
		bool hasVertices2 = e2Mesh->mesh->getNumberOfVertices() > 0;

		for (size_t i = 0; i < nodes1.size(); i++) {
			nodes1[i]->nodeBB->setBaseMatrix(transformMatrix1);
			nodes1[i]->nodeBB->setMatrix(glm::inverse(transformMatrix2)); // Local space for mesh2
			for (size_t j = 0; j < nodes2.size(); j++) {
				float tempTime = Intersection::continousSAT(nodes1[i]->nodeBB, nodes2[j]->nodeBB, newEntityVel, otherEntityVel, std::min(time, timeMax));
				if (tempTime >= 0.0f) {
					for (int k = 0; k < nodes1[i]->nrOfTriangles; k++) {
						if (hasIndices1) { // Has indices
							triangle1.setData(e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(nodes1[i]->triangles[k])), e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(nodes1[i]->triangles[k] + 1)), e1Mesh->mesh->getVertexPosition(e1Mesh->mesh->getVertexIndex(nodes1[i]->triangles[k] + 2)));
						}
						else if (hasVertices1) {
							triangle1.setData(e1Mesh->mesh->getVertexPosition(nodes1[i]->triangles[k]), e1Mesh->mesh->getVertexPosition(nodes1[i]->triangles[k] + 1), e1Mesh->mesh->getVertexPosition(nodes1[i]->triangles[k] + 2));
						}

						for (int l = 0; l < nodes2[j]->nrOfTriangles; l++) {
							if (hasIndices2) { // Has indices
								triangle2.setData(e2Mesh->mesh->getVertexPosition(e2Mesh->mesh->getVertexIndex(nodes2[j]->triangles[l])), e2Mesh->mesh->getVertexPosition(e2Mesh->mesh->getVertexIndex(nodes2[j]->triangles[l] + 1)), e2Mesh->mesh->getVertexPosition(e2Mesh->mesh->getVertexIndex(nodes2[j]->triangles[l] + 2)));
							}
							else if (hasVertices2) {
								triangle2.setData(e2Mesh->mesh->getVertexPosition(nodes2[j]->triangles[l]), e2Mesh->mesh->getVertexPosition(nodes2[j]->triangles[l] + 1), e2Mesh->mesh->getVertexPosition(nodes2[j]->triangles[l] + 2));
							}

							float newTime = Intersection::continousSAT(&triangle1, &triangle2, e1Phys->velocity, e2Phys->velocity, std::min(time, timeMax));

							if (newTime > 0.f && newTime < time) {
								time = newTime;
								outInfo.triangleIndices.clear();
								outInfo.triangleIndices.emplace_back(std::pair<int, int>(nodes1[i]->triangles[k], nodes2[j]->triangles[l]));
							}
							else if (newTime == time) {
								outInfo.triangleIndices.emplace_back(std::pair<int, int>(nodes1[i]->triangles[k], nodes2[j]->triangles[l]));
							}
						}
					}
				}
			}
			nodes1[i]->nodeBB->setBaseMatrix(glm::mat4(1.0f));
			nodes1[i]->nodeBB->setMatrix(glm::mat4(1.0f));
		}
	}

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

	std::vector<glm::vec3>  Intersection::coPlanarLineSegmentsIntersection(const glm::vec3& l1p1, const glm::vec3& l1p2, const glm::vec3& l2p1, const glm::vec3& l2p2) {
		std::vector<glm::vec3> manifold;

		// Line - Line intersection.
		glm::vec3 line = l2p2 - l2p1;
		float tempDot = dot(glm::normalize(line), glm::normalize(l1p2 - l1p1));
		if (tempDot == 1.0f || tempDot == -1.0f) {
			// Points are all on the same line. Find two points in the middle.
			std::map<float, glm::vec3> points;
			points[0.0f] = l2p1;
			points[glm::pow(glm::length(line), 2.0f)] = l2p2;
			if ((dot(l1p1 - l2p1, line) < 0.f && dot(l1p2 - l2p1, line) < 0.f) || (dot(l1p1 - l2p1, line) > glm::pow(glm::length(line), 2.f) && dot(l1p2 - l2p1, line) > glm::pow(glm::length(line), 2.f))) {
				// Lines are not intersecting
				return manifold;
			}

			points[dot(l1p1 - l2p1, line)] = l1p1;
			points[dot(l1p2 - l2p1, line)] = l1p2;

			int counter = 0;
			for (std::map<float, glm::vec3>::const_iterator it = points.begin(); it != points.end(); ++it) {
				counter++;

				if (counter == 1 || (points.size() == 4 && counter == 2)) {
					manifold.emplace_back(it->second);
				}
			}
		}
		else {
			// Lines are crossing. Find intersection
			glm::vec3 lineNorm = glm::normalize(line);
			glm::vec3 pointOffset1 = l1p1 - (l2p1 + dot(l1p1, lineNorm));
			glm::vec3 pointOffset2 = l1p2 - (l2p1 + dot(l1p2, lineNorm));
			if (dot(pointOffset1, pointOffset2) < 0.f) {
				// Offsets to the same side. Lines are not intersecting
				return manifold;
			}
			float distMinPoint0 = glm::length(pointOffset1);
			float distMinPoint1 = glm::length(pointOffset2);
			float progress = distMinPoint0 / (distMinPoint0 + distMinPoint1);
			if (glm::length((l1p1 + (l1p2 - l1p1) * progress) - l2p1) > glm::length(l2p2 - l2p1)) {
				// Lines are not intersecting
				return manifold;
			}

			manifold.emplace_back(l1p1 + (l1p2 - l1p1) * progress);
		}
		return manifold;
	}

	std::vector<glm::vec3> Intersection::coPlanarLineSegmentTriangleIntersection(const glm::vec3& lp1, const glm::vec3& lp2, const glm::vec3& tp1, const glm::vec3& tp2, const glm::vec3& tp3) {
		std::vector<glm::vec3> manifold;
		glm::vec3 aPoint = tp1 + glm::cross(tp2 - tp1, tp3 - tp1);
		if (RayWithTriangle(aPoint, lp1 - aPoint, tp1, tp2, tp3) >= 0.f) {
			// lp1 is inside triangle
			manifold.emplace_back(lp1);
		}

		if (RayWithTriangle(aPoint, lp2 - aPoint, tp1, tp2, tp3) >= 0.f) {
			// lp2 is inside triangle
			manifold.emplace_back(lp2);
		}

		if (manifold.size() < 2) {
			glm::vec3 points[3]{ tp1, tp2, tp3 };
			std::vector<glm::vec3> intersections;
			for (int i = 0; i < 3; i++) {
				intersections = coPlanarLineSegmentsIntersection(lp1, lp2, points[i], points[(i + 1) % 3]);
				if (intersections.size() == 1) {
					manifold.emplace_back(intersections[0]);
				}
				else if (intersections.size() == 2) {
					return intersections;
				}

			}
		}

		return manifold;
	}

	std::vector<glm::vec3> Intersection::coPlanarLineSegmentQuadIntersection(const glm::vec3& lp1, const glm::vec3& lp2, const glm::vec3& qp1, const glm::vec3& qp2, const glm::vec3& qp3, const glm::vec3& qp4) {
		std::vector<glm::vec3> manifold;

		//Sort the points of the quad to be able to find the edges
		glm::vec3 middle = (qp1 + qp2 + qp3 + qp4) / 4.0f;

		std::vector<glm::vec3> corners = { qp1, qp2, qp3, qp4 };

		for (int i = 0; i < 3; i++) {
			glm::vec3 normalVec = glm::normalize(corners[i + 1] - corners[0]);

			glm::vec3 dirVec1 = corners[(i + 1) % 3 + 1] - corners[0] + normalVec * dot(normalVec, corners[(i + 1) % 3 + 1] - corners[0]);
			glm::vec3 dirVec2 = corners[(i + 2) % 3 + 1] - corners[0] + normalVec * dot(normalVec, corners[(i + 2) % 3 + 1] - corners[0]);

			if (dot(dirVec1, dirVec2) < 0.f) { // Diagonal
				if (i != 1) {
					// Needs reorderning
					std::swap(corners[i + 1], corners[2]); // Fix order
				}
				i = 3;
			}
		}

		glm::vec3 aPoint = glm::cross(corners[1] - corners[0], corners[2] - corners[0]);
		if (RayWithTriangle(aPoint, lp1 - aPoint, corners[0], corners[1], corners[2]) >= 0.f || RayWithTriangle(aPoint, lp1 - aPoint, corners[2], corners[0], corners[3]) >= 0.f) {
			// Line point 1 is inside quad
			manifold.emplace_back(lp1);
		}

		if (RayWithTriangle(aPoint, lp2 - aPoint, corners[0], corners[1], corners[2]) >= 0.f || RayWithTriangle(aPoint, lp2 - aPoint, corners[2], corners[0], corners[3]) >= 0.f) {
			// Line point 2 is inside quad
			manifold.emplace_back(lp2);
		}

		if (manifold.size() < 2) {
			for (int i = 0; i < 4; i++) {
				std::vector<glm::vec3> intersection = coPlanarLineSegmentsIntersection(lp1, lp2, corners[i], corners[(i + 1) % 4]);
				if (intersection.size() == 1) {
					manifold.emplace_back(intersection[0]);
				}
				else if (intersection.size() == 2) {
					return intersection;
				}
			}
		}

		return manifold;
	}

	std::vector<glm::vec3> Intersection::coPlanarTrianglesIntersection(const glm::vec3& t1p1, const glm::vec3& t1p2, const glm::vec3& t1p3, const glm::vec3& t2p1, const glm::vec3& t2p2, const glm::vec3& t2p3) {
		std::vector<glm::vec3> manifold;

		glm::vec3 t1[3]{ t1p1, t1p2, t1p3 };
		glm::vec3 t2[3]{ t2p1, t2p2, t2p3 };

		for (int i = 0; i < 3; i++) {
			std::vector<glm::vec3> intersections = coPlanarLineSegmentTriangleIntersection(t1[i], t1[(i + 1) % 3], t2[0], t2[1], t2[2]);
			manifold.insert(manifold.end(), intersections.begin(), intersections.end());

			// Will produce duplicates
			intersections = coPlanarLineSegmentTriangleIntersection(t2[i], t2[(i + 1) % 3], t1[0], t1[1], t1[2]);
			manifold.insert(manifold.end(), intersections.begin(), intersections.end());
		}

		// Remove duplicates
		/*std::sort(manifold.begin(), manifold.end());
		manifold.erase(std::unique(manifold.begin(), manifold.end()), manifold.end());*/

		return manifold;
	}

	std::vector<glm::vec3> Intersection::coPlanarTriangleQuadIntersection(const glm::vec3& tp1, const glm::vec3& tp2, const glm::vec3& tp3, const glm::vec3& qp1, const glm::vec3& qp2, const glm::vec3& qp3, const glm::vec3& qp4) {
		std::vector<glm::vec3> manifold;

		//Sort the points of the quad to be able to find the edges
		glm::vec3 middle = (qp1 + qp2 + qp3 + qp4) / 4.0f;

		std::vector<glm::vec3> corners = { qp1, qp2, qp3, qp4 };

		for (int i = 0; i < 3; i++) {
			glm::vec3 normalVec = glm::normalize(corners[i + 1] - corners[0]);

			glm::vec3 dirVec1 = corners[(i + 1) % 3 + 1] - corners[0] + normalVec * dot(normalVec, corners[(i + 1) % 3 + 1] - corners[0]);
			glm::vec3 dirVec2 = corners[(i + 2) % 3 + 1] - corners[0] + normalVec * dot(normalVec, corners[(i + 2) % 3 + 1] - corners[0]);

			if (dot(dirVec1, dirVec2) < 0.f) { // Diagonal
				if (i != 1) {
					// Needs reorderning
					std::swap(corners[i + 1], corners[2]); // Fix order
				}
				i = 3;
			}
		}

		glm::vec3 t[3]{ tp1, tp2, tp3 };

		for (int i = 0; i < 4; i++) {
			// Edges of quad vs triangle
			std::vector<glm::vec3> intersections = coPlanarLineSegmentTriangleIntersection(corners[i], corners[(i + 1) % 4], t[0], t[1], t[2]);
			manifold.insert(manifold.end(), intersections.begin(), intersections.end());
		}


		for (int i = 0; i < 3; i++) {
			// Edges of triangle vs quad
			// Will produce duplicates
			std::vector<glm::vec3> intersections = coPlanarLineSegmentQuadIntersection(t[i], t[(i + 1) % 3], corners[0], corners[1], corners[2], corners[3]);
			manifold.insert(manifold.end(), intersections.begin(), intersections.end());
		}

		// Remove duplicates
		/*std::sort(manifold.begin(), manifold.end());
		manifold.erase(std::unique(manifold.begin(), manifold.end()), manifold.end());*/

		return manifold;
	}

	std::vector<glm::vec3> Intersection::coPlanarQuadsIntersection(const glm::vec3& q1p1, const glm::vec3& q1p2, const glm::vec3& q1p3, const glm::vec3& q1p4, const glm::vec3& q2p1, const glm::vec3& q2p2, const glm::vec3& q2p3, const glm::vec3& q2p4) {
		std::vector<glm::vec3> manifold;

		//Sort the points of quad 1 to be able to find the edges
		glm::vec3 middle = (q1p1 + q1p2 + q1p3 + q1p4) / 4.0f;

		std::vector<glm::vec3> corners1 = { q1p1, q1p2, q1p3, q1p4 };

		for (int i = 0; i < 3; i++) {
			glm::vec3 normalVec = glm::normalize(corners1[i + 1] - corners1[0]);

			glm::vec3 dirVec1 = corners1[(i + 1) % 3 + 1] - corners1[0] + normalVec * dot(normalVec, corners1[(i + 1) % 3 + 1] - corners1[0]);
			glm::vec3 dirVec2 = corners1[(i + 2) % 3 + 1] - corners1[0] + normalVec * dot(normalVec, corners1[(i + 2) % 3 + 1] - corners1[0]);

			if (dot(dirVec1, dirVec2) < 0.f) { // Diagonal
				if (i != 1) {
					// Needs reorderning
					std::swap(corners1[i + 1], corners1[2]); // Fix order
				}
				i = 3;
			}
		}

		//Sort the points of quad 1 to be able to find the edges
		middle = (q2p1 + q2p2 + q2p3 + q2p4) / 4.0f;

		std::vector<glm::vec3> corners2 = { q2p1, q2p2, q2p3, q2p4 };

		for (int i = 0; i < 3; i++) {
			glm::vec3 normalVec = glm::normalize(corners2[i + 1] - corners2[0]);

			glm::vec3 dirVec1 = corners2[(i + 1) % 3 + 1] - corners2[0] + normalVec * dot(normalVec, corners2[(i + 1) % 3 + 1] - corners2[0]);
			glm::vec3 dirVec2 = corners2[(i + 2) % 3 + 1] - corners2[0] + normalVec * dot(normalVec, corners2[(i + 2) % 3 + 1] - corners2[0]);

			if (dot(dirVec1, dirVec2) < 0.f) { // Diagonal
				if (i != 1) {
					// Needs reorderning
					std::swap(corners2[i + 1], corners2[2]); // Fix order
				}
				i = 3;
			}
		}

		for (int i = 0; i < 4; i++) {
			// Edges of quad 1 vs quad 2
			std::vector<glm::vec3> intersections = coPlanarLineSegmentQuadIntersection(corners1[i], corners1[(i + 1) % 4], corners2[0], corners2[1], corners2[2], corners2[3]);
			manifold.insert(manifold.end(), intersections.begin(), intersections.end());
		}


		for (int i = 0; i < 4; i++) {
			// Edges of quad 2 vs quad 1
			// Will produce duplicates
			std::vector<glm::vec3> intersections = coPlanarLineSegmentQuadIntersection(corners2[i], corners2[(i + 1) % 3], corners1[0], corners1[1], corners1[2], corners1[3]);
			manifold.insert(manifold.end(), intersections.begin(), intersections.end());
		}

		// Remove duplicates
		/*std::sort(manifold.begin(), manifold.end());
		manifold.erase(std::unique(manifold.begin(), manifold.end()), manifold.end());*/

		return manifold;
	}
}