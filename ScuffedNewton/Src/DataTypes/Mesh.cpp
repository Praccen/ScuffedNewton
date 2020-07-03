#include "../pch.h"

#include "Mesh.h"
#include "../Shapes/Box.h"
#include "../Calculations/Intersection.h"

namespace Scuffed {

	Mesh::Mesh() {
		m_data = nullptr;
		m_size = 0;
		m_vertexSize = 1;
		m_positionOffset = 0;
		m_positionSize = 0;

		m_indices = nullptr;
		m_nrOfIndices = 0;

		m_softLimitTriangles = 10;
		m_minimumNodeHalfSize = 1.0f;
	}

	Mesh::~Mesh() {
		clean(&m_baseNode);
	}

	void Mesh::loadData(void* data, size_t size, size_t vertexSize, size_t positionOffset, size_t positionSize) {
		m_data = data;
		m_size = size;
		m_vertexSize = vertexSize;
		m_positionOffset = positionOffset;
		m_positionSize = positionSize;

		clean(&m_baseNode);
		setUpOctree();
	}

	void Mesh::loadIndices(int* indices, int nrOfIndices) {
		m_indices = indices;
		m_nrOfIndices = nrOfIndices;

		clean(&m_baseNode);
		setUpOctree();
	}

	glm::vec3 Mesh::getVertexPosition(int vertexIndex) {
		glm::vec3 position(0.0);
		if ((size_t)vertexIndex < m_size / m_vertexSize) {
			void* pos = static_cast<void*>(static_cast<char*>(m_data) + (m_vertexSize * vertexIndex) + m_positionOffset);

			if (m_positionSize == sizeof(float)) {
				position.x = *(float*)pos;
				pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
				position.y = *(float*)pos;
				pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
				position.z = *(float*)pos;
			}
			else if (m_positionSize == sizeof(double)) {
				position.x = (float) * (double*)pos;
				pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
				position.y = (float) * (double*)pos;
				pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
				position.z = (float) * (double*)pos;
			}
		}

		return position;
	}

	int Mesh::getVertexIndex(int indexIndex) {
		if (indexIndex < m_nrOfIndices) {
			return m_indices[indexIndex];
		}
		return -1;
	}

	int Mesh::getNumberOfVertices() {
		return m_size / m_vertexSize;
	}

	int Mesh::getNumberOfIndices() {
		return m_nrOfIndices;
	}

	void Mesh::getTrianglesForCollisionTesting(std::vector<int> &triangles, Shape* shape) {
		triangles = m_baseNode.triangles;
		//collisionTrianglesRec(triangles, shape, &m_baseNode);
	}

	void Mesh::getTrianglesForContinousCollisionTesting(std::vector<int>& triangles, Shape* shape, glm::vec3& shapeVel, glm::vec3& meshVel, const float maxTime) {
		triangles = m_baseNode.triangles;
		//continousCollisionTrianglesRec(triangles, shape, shapeVel, meshVel, &m_baseNode, maxTime);
	}

	void Mesh::setUpOctree() {
		glm::vec3 minVals(INFINITY);
		glm::vec3 maxVals(-INFINITY);

		std::vector<int> trianglesToAdd;

		if (m_nrOfIndices > 0) { // Has indices
			for (int i = 0; i < m_nrOfIndices; i++) {
				glm::vec3 position = getVertexPosition(m_indices[i]);
				for (int j = 0; j < 3; j++) {
					if (position[j] < minVals[j]) {
						minVals[j] = position[j];
					}
					if (position[j] > maxVals[j]) {
						maxVals[j] = position[j];
					}
				}
				if (i % 3 == 0) {
					trianglesToAdd.emplace_back();
					trianglesToAdd.back() = i;
				}
			}
		}
		else if (getNumberOfVertices() > 0) {
			int numVertices = getNumberOfVertices();
			for (int i = 0; i < numVertices; i++) {
				glm::vec3 position = getVertexPosition(i);
				for (int j = 0; j < 3; j++) {
					if (position[j] < minVals[j]) {
						minVals[j] = position[j];
					}
					if (position[j] > maxVals[j]) {
						maxVals[j] = position[j];
					}
				}
				if (i % 3 == 0) {
					trianglesToAdd.emplace_back();
					trianglesToAdd.back() = i;
				}
			}
		}

		m_baseNode.halfSize = (maxVals - minVals) / 2.0f;
		m_baseNode.nodeBB = SN_NEW Box(m_baseNode.halfSize, minVals + m_baseNode.halfSize);

		m_minimumNodeHalfSize = m_baseNode.halfSize.x / 30.0f; 

		addTrianglesToOctree(trianglesToAdd);
	}

	bool Mesh::addTriangleRec(int triangle, OctNode* currentNode) {
		bool entityAdded = false;

		glm::vec3 isInsideVec = findCornerOutside(triangle, currentNode);
		if (glm::length(isInsideVec) < 1.0f) {
			//The current node does contain the whole triangle. Keep going deeper or add to this node if no smaller nodes are allowed
			if (currentNode->childNodes.size() > 0) { //Not leaf node
				//Recursively call children
				for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
					entityAdded = addTriangleRec(triangle, &currentNode->childNodes[i]);

					if (entityAdded) {
						//Triangle was added to child node. Break loop. No need to try the rest of the children
						i = (unsigned int)currentNode->childNodes.size();
					}
				}

				if (!entityAdded) { //Triangle did not fit in any child node
					//Add triangle to this node
					currentNode->triangles.push_back(triangle);
					currentNode->nrOfTriangles++;
					entityAdded = true;
				}
			}
			else { //Is leaf node
				if (currentNode->nrOfTriangles < m_softLimitTriangles || currentNode->halfSize.x / 2.0f < m_minimumNodeHalfSize) { //Soft limit not reached or smaller nodes are not allowed
					//Add triangle to this node
					currentNode->triangles.push_back(triangle);
					currentNode->nrOfTriangles++;
					entityAdded = true;
				}
				else {
					//Create more children
					for (int i = 0; i < 2; i++) {
						for (int j = 0; j < 2; j++) {
							for (int k = 0; k < 2; k++) {
								Box* currentNodeBB = currentNode->nodeBB;
								OctNode tempChildNode;
								tempChildNode.nodeBB = SN_NEW Box(currentNode->halfSize / 2.0f, currentNodeBB->getMiddle() - currentNode->halfSize / 2.0f + glm::vec3(currentNode->halfSize.x * i, currentNode->halfSize.y * j, currentNode->halfSize.z * k));
								tempChildNode.halfSize = currentNode->halfSize / 2.0f;
								tempChildNode.nrOfTriangles = 0;
								tempChildNode.parentNode = currentNode;
								currentNode->childNodes.push_back(tempChildNode);

								//Try to put triangles that was in this leaf node in the new child nodes.
								for (int l = 0; l < currentNode->nrOfTriangles; l++) {
									if (addTriangleRec(currentNode->triangles[l], &currentNode->childNodes.back())) {
										//Triangle was successfully added to child. Remove it from this node.
										currentNode->triangles.erase(currentNode->triangles.begin() + l);
										currentNode->nrOfTriangles--;
										l--;
									}
								}
							}
						}
					}

					//Try to add the triangle to newly created child nodes. It gets placed in current node within recursion if the children can not contain it.
					entityAdded = addTriangleRec(triangle, currentNode);
				}
			}
		}
		else if (!currentNode->parentNode) { //This is base node
			//Expand base node
			expandBaseNode(isInsideVec);
			//Try to add triangle to new base node
			addTriangleRec(triangle, &m_baseNode);
		}

		return entityAdded;
	}

	void Mesh::addTrianglesToOctree(std::vector<int> trianglesToAdd) {
		for (size_t i = 0; i < trianglesToAdd.size(); i++) {
			//addTriangleRec(trianglesToAdd[i], &m_baseNode);
			m_baseNode.triangles.emplace_back(trianglesToAdd[i]);
			m_baseNode.nrOfTriangles++;
		}
	}

	void Mesh::expandBaseNode(glm::vec3 direction) {
		//Direction to expand in
		int x, y, z;
		x = direction.x >= 0.0f;
		y = direction.y >= 0.0f;
		z = direction.z >= 0.0f;

		OctNode newBaseNode;
		Box* baseNodeBB = m_baseNode.nodeBB;
		newBaseNode.nodeBB = SN_NEW Box(m_baseNode.halfSize * 2.0f, baseNodeBB->getMiddle() - m_baseNode.halfSize + glm::vec3(x * m_baseNode.halfSize.x * 2.0f, y * m_baseNode.halfSize.y * 2.0f, z * m_baseNode.halfSize.z * 2.0f));
		newBaseNode.halfSize = m_baseNode.halfSize * 2.0f;

		newBaseNode.nrOfTriangles = 0;
		newBaseNode.parentNode = nullptr;

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				for (int k = 0; k < 2; k++) {
					OctNode tempChildNode;
					if (i != x && j != y && k != z) {
						tempChildNode = m_baseNode;
					}
					else {
						tempChildNode.nodeBB = SN_NEW Box(m_baseNode.halfSize, newBaseNode.nodeBB->getMiddle() - m_baseNode.halfSize + glm::vec3(m_baseNode.halfSize.x * 2.0f * i, m_baseNode.halfSize.y * 2.0f * j, m_baseNode.halfSize.z * 2.0f * k));
						tempChildNode.halfSize = m_baseNode.halfSize;
						tempChildNode.nrOfTriangles = 0;
					}
					tempChildNode.parentNode = &newBaseNode;
					newBaseNode.childNodes.push_back(tempChildNode);
				}
			}
		}

		m_baseNode = newBaseNode;
	}

	glm::vec3 Mesh::findCornerOutside(int triangle, OctNode* testNode) {
		//Find if any corner of a triangle is outside of node. Returns a vector towards the outside corner if one is found. Otherwise a 0.0f vec is returned.
		glm::vec3 directionVec(0.0f, 0.0f, 0.0f);

		glm::vec3 testNodeHalfSize = testNode->halfSize;

		for (int i = 0; i < 3; i++) {
			glm::vec3 distanceVec(0.f);
			if (m_nrOfIndices > 0) { // Has indices
				distanceVec = getVertexPosition(m_indices[triangle + i]) - testNode->nodeBB->getMiddle();
			}
			else {
				distanceVec = getVertexPosition(triangle + i) - testNode->nodeBB->getMiddle();
			}

			if (distanceVec.x < -testNodeHalfSize.x || distanceVec.x > testNodeHalfSize.x ||
				distanceVec.y < -testNodeHalfSize.y || distanceVec.y > testNodeHalfSize.y ||
				distanceVec.z < -testNodeHalfSize.z || distanceVec.z > testNodeHalfSize.z) {
				directionVec = distanceVec;
				i = 3;
			}
		}

		return directionVec;
	}

	void Mesh::clean(OctNode* node) {
		for (size_t i = 0; i < node->childNodes.size(); i++) {
			clean(&node->childNodes[i]);
		}
		node->childNodes.clear();
		if (node->nodeBB) {
			delete node->nodeBB;
			node->nodeBB = nullptr;
		}
	}

	void Mesh::collisionTrianglesRec(std::vector<int>& triangles, Shape* shape, OctNode* node) {
		if (Intersection::SAT(node->nodeBB, shape)) {
			triangles.insert(triangles.end(), node->triangles.begin(), node->triangles.end());

			int nrOfChildNodes = node->childNodes.size();
			for (int i = 0; i < nrOfChildNodes; i++) {
				collisionTrianglesRec(triangles, shape, &node->childNodes[i]);
			}
		}
	}

	void Mesh::continousCollisionTrianglesRec(std::vector<int>& triangles, Shape* shape, glm::vec3& shapeVel, glm::vec3& meshVel, OctNode* node, const float maxTime) {
		if (Intersection::continousSAT(shape, node->nodeBB, shapeVel, meshVel, maxTime) >= 0.f) {
			triangles.insert(triangles.end(), node->triangles.begin(), node->triangles.end());

			int nrOfChildNodes = node->childNodes.size();
			for (int i = 0; i < nrOfChildNodes; i++) {
				continousCollisionTrianglesRec(triangles, shape, shapeVel, meshVel, &node->childNodes[i], maxTime);
			}
		}
	}

}