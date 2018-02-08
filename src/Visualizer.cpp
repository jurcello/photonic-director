//
//  Visualizer.cpp
//  OpenGlTest
//
//  Created by Jur de Vries on 17/10/2017.
//

#include "Visualizer.h"


Visualizer::Visualizer()
:mEditingMode(false)
{
}

void Visualizer::setup(std::vector<LightRef> &lights)
{
    mMousePos = vec2(0.f);
    // Setup the camera.
    mCam.setEyePoint(vec3(-7.f, 0.f, 0.f));
    mCam.lookAt(vec3(0.f, 0.f, 0.f));
    mCam.setPerspective(45.f, getWindowAspectRatio(), 0.1, 10000.);
    mCamUI.setCamera(&mCam);
    
    // Setup the wire grid and the wire box for selected light.
    auto colorShader = gl::getStockShader( gl::ShaderDef().color() );
    mWirePlane = gl::Batch::create( geom::WirePlane().size( vec2( 10 ) ).subdivisions( ivec2( 10 ) ), colorShader );
    mWireCube = gl::Batch::create( geom::WireCube(), colorShader );

    
    // Create the cube batch.
    mObjectShader = gl::GlslProg::create(loadAsset("object_shader.vertex"), loadAsset("object_shader.fragment"));
    mTextureWood = gl::Texture::create(loadImage(loadAsset("wood.jpeg")));
    
    geom::Cube myCube = geom::Cube();
    mCube = gl::Batch::create(myCube, mObjectShader);
    // TODO: this might be dangerous. Look how we can bind the texture to an unique number.
    mTextureWood->bind(10);
    mObjectShader->uniform("texture1", 10);

    // Create the light shader.
    mLightShader = gl::GlslProg::create(loadAsset("light_shader.vertex"), loadAsset("light_shader.fragment"));
    
    mSphere = TriMesh::create(geom::Sphere());
    mSphereBounds = mSphere->calcBoundingBox();
    mLight = gl::Batch::create(*mSphere, mLightShader);

    // Create the uniform buffer object for the lights.
    unsigned long dataSize = sizeof(Light) * lights.size();
    std::vector<LightBufferData> bufferLights;
    for (auto& light: lights) {
        bufferLights.push_back(LightBufferData(light));
    }
    mLightsUbo = gl::Ubo::create(dataSize, bufferLights.data(), GL_DYNAMIC_DRAW);
    mLightsUbo->bindBufferBase(0);
    mObjectShader->uniformBlock("Lights", 0);

}

void Visualizer::mouseDown(cinder::app::MouseEvent event)
{
    mCamUI.mouseDown(event);
}

void Visualizer::mouseUp(cinder::app::MouseEvent event)
{
    mCamUI.mouseUp(event);
}

void Visualizer::mouseDrag(cinder::app::MouseEvent event)
{
    mCamUI.mouseDrag(event);
    mMousePos = event.getPos();
}

void Visualizer::mouseMove(cinder::app::MouseEvent event)
{
    mMousePos = event.getPos();
}

void Visualizer::resize()
{
    mCam.setPerspective(45.f, getWindowAspectRatio(), 0.1, 10000.);
}

void Visualizer::draw(std::vector<LightRef> lights)
{
    gl::ScopedColor color(Color::gray(1.0f));
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::pushMatrices();
    gl::setMatrices(mCam);

    // First draw the wireplane.
    {
        gl::ScopedColor color(Color::gray(0.3f));
        mWirePlane->draw();
    }
    
    // Fill the uniform buffer object.
    unsigned long dataSize = sizeof(Light) * lights.size();
    std::vector<LightBufferData> bufferLights;
    for (auto& light: lights) {
        bufferLights.push_back(LightBufferData(light));
    }
    
    mLightsUbo->bufferData(dataSize, bufferLights.data(), GL_DYNAMIC_DRAW);
    
    // Draw the cube.
    mObjectShader->uniform("lightCount", (int) lights.size());
    mCube->draw();

    // Draw the lights.
    gl::enableBlending();
    gl::ScopedBlendAdditive a;
    gl::disableDepthWrite();
    for (auto light : lights)
    {
        drawLight(light);
    }
    gl::popMatrices();
}

void Visualizer::enableEditingMode()
{
    mEditingMode = true;
}

void Visualizer::disableEditingMode()
{
    mEditingMode = false;
}

LightRef Visualizer::pickLight(std::vector<LightRef> lights)
{
    float u = mMousePos.x / (float) getWindowWidth();
    float v = mMousePos.y / (float) getWindowHeight();
    Ray ray = mCam.generateRay(u, 1.0f - v, getWindowAspectRatio());
    LightRef pickedLight = nullptr;
    for (LightRef light : lights) {
        mat4 transform = mat4(1.0f);
        transform *= translate(vec3(light->position));
        transform *= scale(vec3(LIGHT_SPHERE_SIZE));
        AxisAlignedBox worldAlignedBox = mSphereBounds.transformed(transform);
        
        if (worldAlignedBox.intersects(ray)) {
            pickedLight = light;
            break;
        }
    }
    return pickedLight;
}

void Visualizer::highLightLight(LightRef light)
{
    highLightLight(light, Color(1.0f, 1.0f, 0.0f));
}

void Visualizer::highLightLight(LightRef light, Color color)
{
    // Create a pulsating box by altering the scale.
    double scaleRatio = 1.1 + 0.1 * sin(4.0 * (getElapsedSeconds() + color.b + 10 * color.g + 100 * color.r));
    gl::pushMatrices();
    gl::setMatrices(mCam);
    gl::translate(vec3(light->position));
    gl::scale(vec3(LIGHT_SPHERE_SIZE * scaleRatio));
    gl::ScopedColor scopedColor(Color(color.r, color.g, color.b));
    mWireCube->draw();
    gl::popMatrices();
}


void Visualizer::drawLight(LightRef light)
{
    gl::pushMatrices();
    gl::setMatrices(mCam);
    gl::translate(vec3(light->position));
    gl::scale(vec3(LIGHT_SPHERE_SIZE));
    ColorA &color = mEditingMode ? light->getLightType()->editColor : light->color;
    mLightShader->uniform("LightColor", color);
    // If in editingmode, draw a light at full intensity,
    float intensity = mEditingMode ? 1.f : light->intensity;
    mLightShader->uniform("LightIntensity", intensity);
    mLight->draw();
    gl::popMatrices();

}
