#pragma once

#include <string>

#include "../engine/math/vec2.h"
#include "../engine/math/vec4.h"

#include "component.h"

class Button;
class Texture;

typedef void (*ButtonAction) (Button*);

class Button : public Component {
private:
	void renderPressed(Vec4 blendColor);
	void renderHovering(Vec4 blendColor);
	void renderIdle(Vec4 blendColor);
public:
	ButtonAction action = [] (Button*) {};

	Texture* hoverTexture = nullptr;
	Texture* idleTexture = nullptr;
	Texture* pressTexture = nullptr;

	std::string text;
	Vec4 fontColor;
	double fontSize;

	bool textured = false;
	bool hovering = false;
	bool pressed = false;

	Vec4 hoverColor;
	Vec4 idleColor;
	Vec4 pressColor;

	Vec4 borderColor;
	double borderWidth;

	void setColor(Vec4 color);

	Button(double x, double y, double width, double height, bool textured);
	Button(std::string text, double x, double y, bool textured);

	void render() override;
	Vec2 resize() override;

	void press(Vec2 point) override;
	void release(Vec2 point) override;
	void enter() override;
	void exit() override;
};

