#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "Triangulate.h"
#include "RenderUtil.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	bool steep = false;
	if (std::abs(x0-x1)<std::abs(y0-y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	int step = x1 > x0 ? 1 : -1;
	float ratio = 1 / (float)(x1 - x0);
	for (int x = x0;  x!=x1; x += step)
	{
		float t = (x - x0) * ratio;
		int y = y0  + (y1 - y0)* t;
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}		
	}
}
void line(Vec2i v0, Vec2i v1, TGAImage& image, TGAColor color)
{
	line(v0.x, v0.y, v1.x, v1.y, image, color);
}

void FillTriangle(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage& image, TGAColor color)
{
	
	if (v0.y > v1.y) std::swap(v0, v1);
	if (v0.y > v2.y) std::swap(v0, v2);
	if (v1.y > v2.y) std::swap(v1, v2);
	int totalH = v2.y - v0.y;
	for (int i = v0.y; i <= v2.y; i++)
	{
		bool bSecond = i > v1.y || v1.y == v0.y;
		Vec2i start = v0 + (v2 - v0) * ((float)(i-v0.y) / totalH);
		Vec2i end = bSecond ? (v1 + (v2-v1)*((float)(i-v1.y))/(v2.y - v1.y)) : (v0 + (v1-v0)*((float)(i-v0.y)/(v1.y - v0.y)));
		if (start.x > end.x) std::swap(start, end);
		for (int j = start.x; j <= end.x; j++)
		{
			image.set(j, i, color);
		}
	}
	 
	//line(v0, v1, image, green);
	//line(v1, v2, image, white);
	//line(v2, v0, image, red);
}

void DrawBasicLines()
{
	TGAImage image(100, 100, TGAImage::RGB);
	//image.set(52, 41, red);
	line(10, 4, 50, 30, image, white);
	line(50, 30, 10, 7, image, red);
	line(20, 13, 40, 80, image, red);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
}

void DrawMesh(const char* meshName, int width, int height)
{
	if (meshName == nullptr) {
		return;
	}
	Model headModel(meshName);
	TGAImage headImage(width, height, TGAImage::RGB);
	for (int i = 0; i < headModel.nfaces(); i++)
	{
		std::vector<int> face = headModel.face(i);
		for (int j = 0; j < 3; j++)
		{
			Vec3f v0 = headModel.vert(face[j]);
			Vec3f v1 = headModel.vert(face[(j + 1) % 3]);
			int x0 = (v0.x + 1)*width / 2.0f;
			int y0 = (v0.y + 1)*height / 2.0f;
			int x1 = (v1.x + 1)*width / 2.0f;
			int y1 = (v1.y + 1)*height / 2.0f;
			line(x0, y0, x1, y1, headImage, white);
		}
	}
	headImage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	std::string name(meshName);
	headImage.write_tga_file((name.substr(0, name.length() - 4) + ".tga").c_str());
}

void FillMesh(const char* meshName, int width, int height)
{
	if (meshName == nullptr) {
		return;
	}
	Model headModel(meshName);
	TGAImage headImage(width, height, TGAImage::RGB);
	Triangulate tri;
	
	Vec2i v[3];
	Vec3f world[3];
	Vec3f light_dir(0,0,-1);
	for (int i = 0; i < headModel.nfaces(); i++)
	{
		std::vector<int> face = headModel.face(i);		
		for (int j = 0; j < 3; j++)
		{
			world[j] = headModel.vert(face[j]);
			Vec3f& v0 = world[j];
			v[j].x = (v0.x + 1)*width / 2.0f;
			v[j].y = (v0.y + 1)*height / 2.0f;
		}
		tri.ResetVertex(v[0], v[1], v[2]);
		Vec3f n = cross(world[2] - world[0], world[1] - world[0]);
		n.normalize();
		float intensity = n*light_dir;
		if (intensity > 0)
		{
			tri.DoTriangulate(headImage, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
		
	}
	headImage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	std::string name(meshName);
	headImage.write_tga_file((name.substr(0, name.length() - 4) + ".tga").c_str());
}

void FillMeshZTest(const char* meshName, int width, int height)
{
	if (meshName == nullptr) {
		return;
	}
	Model headModel(meshName);
	TGAImage headImage(width, height, TGAImage::RGB);
	Triangulate tri;

	Vec2i v[3];
	Vec3f world[3];
	Vec3f light_dir(0, 0, -1);
	float* zBuffer = new float[width*height];
	for (int i = width * height; i--; zBuffer[i] = -std::numeric_limits<float>::max());
	for (int i = 0; i < headModel.nfaces(); i++)
	{
		std::vector<int> face = headModel.face(i);
		for (int j = 0; j < 3; j++)
		{
			world[j] = headModel.vert(face[j]);
			Vec3f& v0 = world[j];
			v[j].x = (v0.x + 1)*width / 2.0f;
			v[j].y = (v0.y + 1)*height / 2.0f;
		}
		tri.ResetVertex(v[0], v[1], v[2]);
		Vec3f n = cross(world[2] - world[0], world[1] - world[0]);
		n.normalize();
		float intensity = n * light_dir;
		if (intensity > 0) // do back culling
		{
			tri.DoTriangulateZTest(headImage, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255), zBuffer, world[0].z, world[1].z, world[2].z);
		}

	}
	headImage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	std::string name(meshName);
	headImage.write_tga_file((name.substr(0, name.length() - 4) + ".tga").c_str());
	delete[] zBuffer;
}

void FillMeshWithDiffuseZTest(const char* meshName, int width, int height)
{
	if (meshName == nullptr) {
		return;
	}
	Model headModel(meshName);
	TGAImage headImage(width, height, TGAImage::RGB);
	Triangulate tri;

	Vec2i v[3];
	Vec3f world[3];
	Vec3f light_dir(0, 0, -1);
	TGAColor uvColor[3];
	float* zBuffer = new float[width*height];
	for (int i = width * height; i--; zBuffer[i] = -std::numeric_limits<float>::max());
	for (int i = 0; i < headModel.nfaces(); i++)
	{
		std::vector<int> face = headModel.face(i);
		for (int j = 0; j < 3; j++)
		{
			world[j] = headModel.vert(face[j]);
			Vec3f& v0 = world[j];
			v[j].x = (v0.x + 1)*width / 2.0f;
			v[j].y = (v0.y + 1)*height / 2.0f;
			uvColor[j] = headModel.diffuse(headModel.uv(i, j));
		}
		tri.ResetVertex(v[0], v[1], v[2]);
		Vec3f n = cross(world[2] - world[0], world[1] - world[0]);
		n.normalize();
		float intensity = n * light_dir;
		if (intensity > 0) // do back culling
		{
			tri.FillTriangleWithDiffuse(headImage, uvColor, zBuffer, world);
		}

	}
	headImage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	std::string name(meshName);
	headImage.write_tga_file((name.substr(0, name.length() - 4) + ".tga").c_str());
	delete[] zBuffer;
}

void FillMeshWithProjection(const char* meshName, int width, int height)
{
	if (meshName == nullptr) {
		return;
	}
	Model headModel(meshName);
	TGAImage headImage(width, height, TGAImage::RGB);

	float* zBuffer = new float[width*height];
	for (int i = width * height; i--; zBuffer[i] = -std::numeric_limits<float>::max());

	Matrix projection = Matrix::identity();
	projection[3][2] = -1.0f / 3;
	Matrix vp = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4, 255);

	Triangulate tri;
	Vec2i v[3];
	Vec3f world[3];
	Vec3f light_dir(0, 0, -1);
	TGAColor uvColor[3];
	Vec4f sp[3];

	for (int i = 0; i < headModel.nfaces(); i++)
	{
		std::vector<int> face = headModel.face(i);
		for (int j = 0; j < 3; j++)
		{
			world[j] = headModel.vert(face[j]);
			Vec4f v0;
			v0[0] = world[j][0];
			v0[1] = world[j][1];
			v0[2] = world[j][2];
			v0[3] = 1.0f;

			sp[j] = vp * projection*v0;
			v[j].x = sp[j][0]/ sp[j][3];
			v[j].y = sp[j][1]/ sp[j][3];
			//v[j].z = v0[2] / v0[3];
			uvColor[j] = headModel.diffuse(headModel.uv(i, j));
		}
		tri.ResetVertex(v[0], v[1], v[2]);
		Vec3f n = cross(world[2] - world[0], world[1] - world[0]);
		n.normalize();
		float intensity = n * light_dir;
		if (intensity > 0) // do back culling
		{
			tri.FillTriangleWithDiffuse(headImage, uvColor, zBuffer, world);
		}

	}
	headImage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	std::string name(meshName);
	headImage.write_tga_file((name.substr(0, name.length() - 4) + ".tga").c_str());
	delete[] zBuffer;
}

int main(int argc, char** argv) {
	
// 	DrawBasicLines();
// 	DrawMesh("african_head.obj", 800, 800);	
// 	TGAImage image(400, 400, TGAImage::RGB);
// 	FillTriangle(Vec2i(10, 10), Vec2i(300, 270), Vec2i(220, 355), image, green);
// 	image.flip_vertically();
// 	image.write_tga_file("triangle.tga");
// 
// 	TGAImage newImage(400, 400, TGAImage::RGB);
// 	Triangulate tri(Vec2i(10, 10), Vec2i(300, 270), Vec2i(220, 355));
// 	tri.DoTriangulate(newImage, red);
// 	newImage.flip_vertically();
// 	newImage.write_tga_file("newTri.tga");
//	FillMeshZTest("african_head.obj", 800, 800);
//	FillMeshWithDiffuseZTest("african_head.obj", 800, 800);
	FillMeshWithProjection("african_head.obj", 800, 800);
	system("pause");
	return 0;
}