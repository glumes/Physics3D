#pragma once

#include "../engine/layer/layer.h"

namespace P3D::Application {

class Screen;

class ModelLayer : public Engine::Layer {
private:
	struct Uniform {
		Mat4f modelMatrix = Mat4f::IDENTITY();
		Vec4f albedo = Vec4f::full(1.0f);
		float metalness = 1.0f;
		float roughness  = 1.0f;
		float ao = 1.0f;
	};

	std::vector<Uniform> uniforms;

public:
	inline ModelLayer() : Layer() {};
	inline ModelLayer(Screen* screen, char flags = None) : Layer("Model", screen, flags) {}

	virtual void onInit(Engine::Registry64& registry) override;
	virtual void onUpdate(Engine::Registry64& registry) override;
	virtual void onEvent(Engine::Registry64& registry, Engine::Event& event) override;
	virtual void onRender(Engine::Registry64& registry) override;
	virtual void onClose(Engine::Registry64& registry) override;
};

};