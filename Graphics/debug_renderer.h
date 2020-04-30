#pragma once
#include <DirectXMath.h>
using namespace DirectX;

struct colored_vertex
{

	XMFLOAT3 pos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };


	colored_vertex() = default;
	colored_vertex(const colored_vertex&) = default;

	inline colored_vertex(const XMFLOAT3& p, const XMFLOAT4& c) : pos{ p }, color{ c } {};
	inline colored_vertex(const XMFLOAT3& p, const XMFLOAT3& c) : pos{ p }, color{ c.x, c.y, c.z, 1.0f } {};
	inline colored_vertex(const XMFLOAT3& p, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : pos{ p }, color{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f } {};
};


namespace debug_renderer
{

	// Interface to the debug renderer

	void add_line(XMFLOAT3 point_a, XMFLOAT3 point_b, XMFLOAT4 color_a, XMFLOAT4 color_b);

	inline void add_line(XMFLOAT3 p, XMFLOAT3 q, XMFLOAT4 color) { add_line(p, q, color, color); };

	void clear_lines();

	const colored_vertex* get_line_verts();

	size_t get_line_vert_count();

	size_t get_line_vert_capacity();

	XMFLOAT3 XMVECTOR_TO_XMFLOAT3(XMVECTOR v);
	XMVECTOR XMMATRIX_XMVectorbyROW(XMMATRIX& InMatrix, int row);
	XMFLOAT3 XMMATRIX_XMVECTOR_With_Offset(XMMATRIX& InMatrix, int row, XMVECTOR& origin);
	void draw_matrix(XMMATRIX& outMatrix);
	void draw_to_parent(XMMATRIX& outMatrix, XMMATRIX& parentMatrix);


}
