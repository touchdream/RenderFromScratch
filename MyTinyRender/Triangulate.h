#pragma once
#include "geometry.h"
#include "tgaimage.h"
#include <algorithm>

class Triangulate
{
public:
	Triangulate()
	{
		mTriangles.resize(3, Vec2i(0, 0));
	}
	Triangulate(Vec2i a, Vec2i b, Vec2i c)
	{
// 		if (a.y > b.y) std::swap(a, b);
// 		if (a.y > c.y) std::swap(a, c);
// 		if (b.y > c.y) std::swap(b, c);
		mTriangles.resize(3);
		mTriangles[0] = a;
		mTriangles[1] = b;
		mTriangles[2] = c;
	}
	Vec2i& GetA() { return mTriangles[0]; }
	Vec2i& GetB() { return mTriangles[1]; }
	Vec2i& GetC() { return mTriangles[2]; }

	Vec3f GetBerycentric(Vec2i p)
	{
		Vec3i u = cross(Vec3i(mTriangles[1].x - mTriangles[0].x, mTriangles[2].x - mTriangles[0].x, mTriangles[0].x - p.x), Vec3i(mTriangles[1].y - mTriangles[0].y, mTriangles[2].y - mTriangles[0].y, mTriangles[0].y - p.y));
		if (u.z == 0) {
			return Vec3f(-1, 1, 1);
		}
		return Vec3f(1.0f - (u.x + u.y) / (float)(u.z), u.x/(float)u.z, u.y/(float)u.z);
	}

	void ResetVertex(Vec2i a, Vec2i b, Vec2i c)
	{
		if (mTriangles.size() != 3)
		{
			mTriangles.resize(3);			
		}
		mTriangles[0] = a;
		mTriangles[1] = b;
		mTriangles[2] = c;
	}
	void DoTriangulate(TGAImage& image, TGAColor color)
	{
		Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
		Vec2i bboxmax(0, 0);
		for (int i = 0; i < 3; i++)
		{
			bboxmin.x = std::max<int>(0, std::min<int>(bboxmin.x, mTriangles[i].x));
			bboxmin.y = std::max<int>(0, std::min<int>(bboxmin.y, mTriangles[i].y));

			bboxmax.x = std::min<int>(image.get_width() -1, std::max<int>(bboxmax.x, mTriangles[i].x));
			bboxmax.y = std::min<int>(image.get_height() - 1, std::max<int>(bboxmax.y, mTriangles[i].y));
		}
		Vec2i p;
		for (p.x = bboxmin.x; p.x <= bboxmax.x; p.x++)
		{
			for (p.y = bboxmin.y; p.y <= bboxmax.y; p.y++)
			{
				Vec3f baryPos = GetBerycentric(p);
				if (baryPos.x < 0 || baryPos.y < 0 || baryPos.z < 0) { continue; }
				image.set(p.x, p.y, color);
			}
		}
	}
	void DoTriangulateZTest(TGAImage& image, TGAColor color, float* zBuffer, float z1, float z2, float z3)
	{
		Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
		Vec2i bboxmax(0, 0);
		for (int i = 0; i < 3; i++)
		{
			bboxmin.x = std::max<int>(0, std::min<int>(bboxmin.x, mTriangles[i].x));
			bboxmin.y = std::max<int>(0, std::min<int>(bboxmin.y, mTriangles[i].y));

			bboxmax.x = std::min<int>(image.get_width() - 1, std::max<int>(bboxmax.x, mTriangles[i].x));
			bboxmax.y = std::min<int>(image.get_height() - 1, std::max<int>(bboxmax.y, mTriangles[i].y));
		}
		Vec2i p;
		float z = 0;
		for (p.x = bboxmin.x; p.x <= bboxmax.x; p.x++)
		{
			for (p.y = bboxmin.y; p.y <= bboxmax.y; p.y++)
			{
				Vec3f baryPos = GetBerycentric(p);
				if (baryPos.x < 0 || baryPos.y < 0 || baryPos.z < 0) { continue; }
				z = baryPos.x*z1 + baryPos.y*z2 + baryPos.z*z3;
				if (z > zBuffer[p.x*image.get_height() + p.y])
				{
					zBuffer[p.x*image.get_height() + p.y] = z;
					image.set(p.x, p.y, color);
				}
				
			}
		}
	}
	void FillTriangleWithDiffuse(TGAImage& image, TGAColor* diffuse, float* zBuffer, float z1, float z2, float z3)
	{
		Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
		Vec2i bboxmax(0, 0);
		for (int i = 0; i < 3; i++)
		{
			bboxmin.x = std::max<int>(0, std::min<int>(bboxmin.x, mTriangles[i].x));
			bboxmin.y = std::max<int>(0, std::min<int>(bboxmin.y, mTriangles[i].y));

			bboxmax.x = std::min<int>(image.get_width() - 1, std::max<int>(bboxmax.x, mTriangles[i].x));
			bboxmax.y = std::min<int>(image.get_height() - 1, std::max<int>(bboxmax.y, mTriangles[i].y));
		}
		Vec2i p;
		float z = 0;
		TGAColor color;
		for (p.x = bboxmin.x; p.x <= bboxmax.x; p.x++)
		{
			for (p.y = bboxmin.y; p.y <= bboxmax.y; p.y++)
			{
				Vec3f baryPos = GetBerycentric(p);
				if (baryPos.x < 0 || baryPos.y < 0 || baryPos.z < 0) { continue; }
				z = baryPos.x*z1 + baryPos.y*z2 + baryPos.z*z3;
				if (z > zBuffer[p.x*image.get_height() + p.y])
				{
					color[0] = baryPos.x*diffuse[0][0] + baryPos.y * diffuse[1][0] + baryPos.z * diffuse[2][0];
					//color[0] *= 255.0f;
					color[1] = baryPos.x*diffuse[0][1] + baryPos.y * diffuse[1][1] + baryPos.z * diffuse[2][1];
					//color[1] *= 255.0f;
					color[2] = baryPos.x*diffuse[0][2] + baryPos.y * diffuse[1][2] + baryPos.z * diffuse[2][2];
					//color[2] *= 255.0f;
					color[3] = baryPos.x*diffuse[0][3] + baryPos.y * diffuse[1][3] + baryPos.z * diffuse[2][3];
					zBuffer[p.x*image.get_height() + p.y] = z;
					image.set(p.x, p.y, color);
				}

			}
		}
	}
private:
	std::vector<Vec2i> mTriangles;
};