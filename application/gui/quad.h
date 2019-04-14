#pragma once

#include "arrayMesh.h"
#include "../../engine/math/vec2.h"

class Quad {
private:
	unsigned int vao;
	unsigned int vbo;

public:
	void resize(Vec2 position, Vec2 dimension) {
		float vertices[6][4] = {
			{ position.x,				position.y				, 0, 1},
			{ position.x,				position.y - dimension.y, 0, 0},
			{ position.x + dimension.x, position.y - dimension.y, 1, 0},

			{ position.x,				position.y				, 0, 1},
			{ position.x + dimension.x,	position.y - dimension.y, 1, 0},
			{ position.x + dimension.x, position.y				, 1, 1}
		};

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	Quad() {
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		resize(Vec2(-1, 1), Vec2(2));
	}

	void render() {
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
};