#include "panel.h"

#include "gui.h"

#include "../shaderProgram.h"

#include "layout.h"
#include "../util/log.h"
#include "../engine/math/mathUtil.h"

Panel::Panel(double x, double y) : Container(x, y) {
	this->background = GUI::COLOR::BACK;
	this->padding = GUI::padding;
	this->margin = GUI::margin;
};

Panel::Panel(double x, double y, double width, double height) : Container(x, y, width, height) {
	this->background = GUI::COLOR::BACK;
	this->padding = GUI::padding;
	this->margin = GUI::margin;
};

Vec2 Panel::resize() {
	Vec2 positionOffset = Vec2(padding, -padding);
	Vec2 dimensionOffset = Vec2(2 * padding);

	position += positionOffset;
	dimension -= dimensionOffset;

	dimension = layout->resize(this);

	position -= positionOffset;
	dimension += dimensionOffset;

	return dimension;
}

void Panel::render() {
	if (visible) {

		Vec4 blendColor = (disabled) ? GUI::COLOR::DISABLED : GUI::COLOR::WHITE;

		resize();

		Shaders::quadShader.updateColor(GUI::COLOR::blend(background, blendColor));
		GUI::quad->resize(position, dimension);
		GUI::quad->render();

		renderChildren();
	}
}
