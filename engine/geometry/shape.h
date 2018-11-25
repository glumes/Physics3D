#pragma once

#include "../../util/math/vec3.h"

struct Triangle {
	int firstIndex, secondIndex, thirdIndex;
};

class Shape {
private:

	Vec3 * vertices;
	const int vertexCount;
	Triangle * triangles;
	const int triangleCount;
	int * const copyCount;

public:
	Shape(int vertexCount, int triangleCount);

	Shape(Vec3* vertexes, int vertexCount, Triangle* triangles, int triangleCount);

	~Shape();

	Shape(const Shape& s);

	
	void setVertex(int i, Vec3 vertex) { vertices[i] = vertex; };
	void setTriangle(int i, int v1, int v2, int v3) { triangles[i] = { v1, v2, v3 }; };

};
