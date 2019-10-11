#include "Triangulate.h"

void Triangulate::FillTriangleWithDiffuse(TGAImage& image, TGAColor* diffuse, float* zBuffer, const Vec3f world[3])
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
			z = baryPos.x*world[0].z + baryPos.y*world[1].z + baryPos.z*world[2].z;
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
