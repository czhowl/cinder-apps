#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class meshshaderApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	static void prepare(App::Settings *settings);

  private:
	void						createMesh();
	gl::GlslProgRef				mShader;
	CameraPersp					mCamera;
	CameraUi					mCamUi;
	gl::VboMeshRef				mVboMesh;
	gl::BatchRef				mBatch;
};

void meshshaderApp::setup()
{
}

void meshshaderApp::mouseDown( MouseEvent event )
{
}

void meshshaderApp::update()
{
}

void meshshaderApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

void meshshaderApp::createMesh()
{
	// create vertex, normal and texcoord buffers
	const int  RES_X = 3;
	const int  RES_Z = 3;
	const vec3 size = vec3(20.0f, 1.0f, 20.0f);

	std::vector<vec3> positions(RES_X * RES_Z);
	std::vector<vec3> normals(RES_X * RES_Z);
	std::vector<vec2> texcoords(RES_X * RES_Z);
	std::vector<ColorA> colors(RES_X * RES_Z);
	std::vector<ColorA> backcolors(RES_X * RES_Z);

	int i = 0;
	for (int x = 0; x < RES_X; ++x) {
		for (int z = 0; z < RES_Z; ++z) {
			const float u = float(x) / RES_X;
			const float v = float(z) / RES_Z;
			positions[i] = size * vec3(u - 0.5f, 0.0f, v - 0.5f);
			//normals[i] = vec3(0, 1, 0);
			normals[i] = vec3(0, 0, -1);
			texcoords[i] = vec2(u, v);
			float alpha = float(i) / RES_X / RES_Z;
			alpha = 1.0f;
			colors[i] = ColorA(1.0f, u, v, alpha);
			backcolors[i] = ColorA(v, u, 1.0f, alpha);
			i++;
		}
	}

	// create index buffer
	vector<uint16_t> indices;
	indices.reserve(6 * (RES_X - 1) * (RES_Z - 1));

	for (int x = 0; x < RES_X - 1; ++x) {
		for (int z = 0; z < RES_Z - 1; ++z) {
			uint16_t i = x * RES_Z + z;

			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(i + RES_Z);
			indices.push_back(i + RES_Z);
			indices.push_back(i + 1);
			indices.push_back(i + RES_Z + 1);
		}
	}

	// construct vertex buffer object
	gl::VboMesh::Layout layout;
	layout.usage(GL_DYNAMIC_DRAW).attrib(geom::POSITION, 3);
	layout.attrib(geom::COLOR, 4);
	layout.attrib(geom::CUSTOM_0, 4);
	layout.attrib(geom::NORMAL, 3);
	layout.attrib(geom::TEX_COORD_0, 2);

	mVboMesh = gl::VboMesh::create(positions.size(), GL_TRIANGLES, { layout }, indices.size());
	mVboMesh->bufferAttrib(geom::POSITION, positions.size() * sizeof(vec3), positions.data());
	mVboMesh->bufferAttrib(geom::COLOR, colors.size() * sizeof(ColorA), colors.data());
	mVboMesh->bufferAttrib(geom::CUSTOM_0, backcolors.size() * sizeof(ColorA), backcolors.data());
	mVboMesh->bufferAttrib(geom::NORMAL, normals.size() * sizeof(vec3), normals.data());
	mVboMesh->bufferAttrib(geom::TEX_COORD_0, texcoords.size() * sizeof(vec2), texcoords.data());
	mVboMesh->bufferIndices(indices.size() * sizeof(uint16_t), indices.data());

	// create a batch for better performance
	mBatch = gl::Batch::create(mVboMesh, mShader, { { geom::Attrib::CUSTOM_0, "backColor" } });
}

void meshshaderApp::prepare(App::Settings *settings)
{
	settings->setWindowSize(1920, 1080);
	//    settings->setFullScreen();
}

CINDER_APP( meshshaderApp, RendererGl(RendererGl::Options().msaa(16)), &meshshaderApp::prepare)
