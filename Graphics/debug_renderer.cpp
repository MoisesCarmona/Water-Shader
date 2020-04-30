#pragma once
#include "debug_renderer.h"
#include <array>
namespace
{

	// Anonymous namespace
		// Declarations in an anonymous namespace are global BUT only have internal linkage.
		// In other words, these variables are global but are only visible in this source file.

		// Maximum number of debug lines at one time (i.e: Capacity)
	constexpr size_t MAX_LINE_VERTS = 64200;//8192; 32006

	// CPU-side buffer of debug-line verts
	// Copied to the GPU and reset every frame.
	size_t line_vert_count = 0;
	std::array<colored_vertex, MAX_LINE_VERTS> line_verts;
};
namespace debug_renderer
{
	void add_line(XMFLOAT3 point_a, XMFLOAT3 point_b, XMFLOAT4 color_a, XMFLOAT4 color_b)
	{
		// Add points to debug_verts, increments debug_vert_count

		line_verts[line_vert_count].color = color_a;
		line_verts[line_vert_count].pos = point_a;

		line_verts[line_vert_count + 1].color = color_b;
		line_verts[line_vert_count + 1].pos = point_b;

		line_vert_count += 2;
	}

	void clear_lines()
	{
		// Resets debug_vert_count

		line_vert_count = 0;

	}

	const colored_vertex* get_line_verts()
	{
		// Does just what it says in the name

		return line_verts.data();
	}

	size_t get_line_vert_count()
	{
		// Does just what it says in the name
		return line_vert_count;
	}

	size_t get_line_vert_capacity()
	{
		// Does just what it says in the name
		return MAX_LINE_VERTS;
	}
	XMFLOAT3 XMVECTOR_TO_XMFLOAT3(XMVECTOR v)
	{
		return{ v.m128_f32[0],v.m128_f32[1],v.m128_f32[2] };

	};

	XMVECTOR XMMATRIX_XMVectorbyROW(XMMATRIX& InMatrix, int row)
	{
		return { InMatrix.r[row] };

	};
	XMFLOAT3 XMMATRIX_XMVECTOR_With_Offset(XMMATRIX& InMatrix, int row, XMVECTOR& origin)
	{
		XMVECTOR v = InMatrix.r[row]/5 + origin;

		return { XMVECTOR_TO_XMFLOAT3(v) };
	};
	void draw_matrix(XMMATRIX& outMatrix)
	{

		XMVECTOR origin = XMMATRIX_XMVectorbyROW(outMatrix, 3);
	//	add_line(XMVECTOR_TO_XMFLOAT3(origin), XMVECTOR_TO_XMFLOAT3(origin), { 1,1,1,1 });
		//float3{ outMatrix.r[3].m128_f32[0],outMatrix.r[3].m128_f32[1],outMatrix.r[3].m128_f32[2] };
		add_line(XMVECTOR_TO_XMFLOAT3(origin), XMMATRIX_XMVECTOR_With_Offset(outMatrix, 0, origin), { 1,0,0,1 }); //x axis
		add_line(XMVECTOR_TO_XMFLOAT3(origin), XMMATRIX_XMVECTOR_With_Offset(outMatrix, 1, origin), { 0,1,0,1 }); //y axis
		add_line(XMVECTOR_TO_XMFLOAT3(origin), XMMATRIX_XMVECTOR_With_Offset(outMatrix, 2, origin), { 0,0,1,1 }); //z axis

	};
	void draw_to_parent(XMMATRIX& outMatrix, XMMATRIX& parentMatrix)
	{
		XMVECTOR child = XMMATRIX_XMVectorbyROW(outMatrix, 3);
		XMVECTOR parent = XMMATRIX_XMVectorbyROW(parentMatrix, 3);


		add_line(XMVECTOR_TO_XMFLOAT3(child), XMVECTOR_TO_XMFLOAT3(parent), { 1,1,1,1 });


	};


}
