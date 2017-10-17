#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class PhotonicDirectorApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void PhotonicDirectorApp::setup()
{
}

void PhotonicDirectorApp::mouseDown( MouseEvent event )
{
}

void PhotonicDirectorApp::update()
{
}

void PhotonicDirectorApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( PhotonicDirectorApp, RendererGl )
