#pragma once
#include "Shape.h"
#include "Material.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "Renderer.h"

class ENGINEDLL_API Circle : public Shape {
public:
	Circle(Renderer* pRenderer, Material material, unsigned int sidesAmount = 10);
	~Circle() {}
	void Draw() override;
	void SetVertices(void* data) override;
	void SetSidesAmount(unsigned int amount);
private:
	unsigned int m_sides;
	VertexArray m_va;
	VertexBuffer m_vb;
	Material m_material;
	Renderer* m_pRenderer;
};
