#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/TriMesh.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"
#include "Light.h"
#include "ConfigManager.h"
#include "Visualizer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

class PhotonicDirectorApp : public App {
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void mouseUp( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void mouseMove( MouseEvent event ) override;
    void update() override;
    void draw() override;
    void keyDown( KeyEvent event) override;
    void resize() override;
    
    void save();
    void load();
    
    void pickLight();
    
    void editLight(Light* light);
    
    void addLight();
    void addLightDone();
    
    ~PhotonicDirectorApp();
    
protected:
    ConfigManager config;
    
    vector<Light*> mLights;
    
    // Light picking.
    Light* pickedLight;
    
    // Visualizer.
    Visualizer mVisualizer;
    
    // Create parameters
    params::InterfaceGlRef mGeneralControls;
    params::InterfaceGlRef mLightControls;
    bool mDrawGui;
};

void PhotonicDirectorApp::setup()
{
    pickedLight = nullptr;
    
    // Setup some initial lights.
    mLights.push_back(new Light(vec3(2.0f, 2.0f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.6f));
    mLights.push_back(new Light(vec3(1.0f, 1.0, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.99f));
    mLights.push_back(new Light(vec3(3.0f, 2.0f, -1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.9f));
    
    // Setup visualizer.
    mVisualizer.setup(mLights);
    
    // Initialize params.
    mDrawGui = true;
    mGeneralControls = params::InterfaceGl::create("Controls", vec2(200,200));
    mGeneralControls->setPosition(vec2(10,10));
    mGeneralControls->addButton("Add light", [&](){addLight();});
    mGeneralControls->addSeparator();
    mGeneralControls->addButton("Save", std::bind(&PhotonicDirectorApp::save, this));
    mGeneralControls->addButton("Load", std::bind(&PhotonicDirectorApp::load, this));
    mLightControls = params::InterfaceGl::create("Light controls", vec2(200,400));
    mLightControls->setPosition(vec2(220, 10));
    // Hide the light controls.
    mLightControls->hide();
}

void PhotonicDirectorApp::editLight(Light* light) {
    mLightControls->clear();
    mLightControls->addParam("Intensity", &(light->intensity), "min=0.0 max=1.0 step=0.01");
    // Create functions for setting the position.
    function<void (vec3)> setter = bind(&Light::setPosition, light, placeholders::_1);
    function<vec3 ()> getter = bind(&Light::getPosition, light);
    mLightControls->addParam("Position", setter, getter);
    mLightControls->addParam("Color", &(light->color));
    mLightControls->addButton("Done", [&](){addLightDone();});
    mLightControls->show();
}

void PhotonicDirectorApp::addLight() {
    Light* newLight = new Light(vec3(1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.5f);
    mLights.push_back(newLight);
    editLight(newLight);
}

void PhotonicDirectorApp::addLightDone() {
    mLightControls->clear();
    mLightControls->hide();
}

void PhotonicDirectorApp::mouseDown( MouseEvent event )
{
    mVisualizer.mouseDown(event);
    if (pickedLight) {
        editLight(pickedLight);
    }
}

void PhotonicDirectorApp::mouseUp(cinder::app::MouseEvent event)
{
    mVisualizer.mouseUp(event);
}

void PhotonicDirectorApp::mouseDrag(cinder::app::MouseEvent event)
{
    mVisualizer.mouseDrag(event);
}

void PhotonicDirectorApp::mouseMove(cinder::app::MouseEvent event)
{
    mVisualizer.mouseMove(event);
}

void PhotonicDirectorApp::keyDown( KeyEvent event)
{
    if (event.getChar() == KeyEvent::KEY_ESCAPE)
        exit(0);
    if (event.getChar() == 'g')
        mDrawGui = !mDrawGui;
}

void PhotonicDirectorApp::save()
{
    std::vector<string> extensions {"xml"};
    fs::path savePath = getSaveFilePath(fs::path(), extensions);
    if (! savePath.empty()) {
        config.writeLights(savePath, mLights);
    }
}

void PhotonicDirectorApp::load()
{
    std::vector<string> extensions {"xml"};
    fs::path loadPath = getOpenFilePath(fs::path(), extensions);
    if (! loadPath.empty()) {
        config.readLights(loadPath, mLights);
    }
}

void PhotonicDirectorApp::update()
{
    pickLight();
}

void PhotonicDirectorApp::resize()
{
    mVisualizer.resize();
}

void PhotonicDirectorApp::pickLight()
{
    pickedLight = mVisualizer.pickLight(mLights);
}

void PhotonicDirectorApp::draw()
{
    
    gl::clear( Color( 0, 0, 0 ) );
    
    mVisualizer.draw(mLights);
    
    if (pickedLight) {
        mVisualizer.highLightLight(pickedLight);
    }
    
    if (mDrawGui) {
        mGeneralControls->draw();
        mLightControls->draw();
    }
    
}

PhotonicDirectorApp::~PhotonicDirectorApp()
{
    for (Light* light : mLights) {
        delete light;
    }
}

CINDER_APP( PhotonicDirectorApp, RendererGl( RendererGl::Options().msaa(8)), [](cinder::app::AppBase::Settings *settings){
    settings->setTitle("Photonic Director");
    settings->setWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
} );

