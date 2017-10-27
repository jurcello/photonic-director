//
//  Visualizer.cpp
//  OpenGlTest
//
//  Created by Jur de Vries on 17/10/2017.
//

#include "Visualizer.h"

void Visualizer::setup(std::vector<Light*> lights)
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
    mTextureWood->bind(0);
    mObjectShader->uniform("texture1", 0);

    // Create the light shader.
    mLightShader = gl::GlslProg::create(loadAsset("light_shader.vertex"), loadAsset("light_shader.fragment"));
    
    mSphere = TriMesh::create(geom::Sphere());
    mSphereBounds = mSphere->calcBoundingBox();
    mLight = gl::Batch::create(*mSphere, mLightShader);

    // Create the uniform buffer object for the lights.
    unsigned long dataSize = sizeof(Light) * lights.size();
    std::vector<Light> bufferLights;
    for (Light* light: lights) {
        bufferLights.push_back(*light);
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

void Visualizer::draw(std::vector<Light *> lights)
{
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::setMatrices(mCam);

    // First draw the wireplane.
    {
        gl::ScopedColor color(Color::gray(0.3f));
        mWirePlane->draw();
    }
    
    // Fill the uniform buffer object.
    unsigned long dataSize = sizeof(Light) * lights.size();
    std::vector<LightBufferData> bufferLights;
    for (Light* light: lights) {
        bufferLights.push_back(LightBufferData(*light));
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
}

Light* Visualizer::pickLight(std::vector<Light*> lights)
{
    float u = mMousePos.x / (float) getWindowWidth();
    float v = mMousePos.y / (float) getWindowHeight();
    Ray ray = mCam.generateRay(u, 1.0f - v, getWindowAspectRatio());
    Light* pickedLight = nullptr;
    for (Light* light : lights) {
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

void Visualizer::highLightLight(Light *light)
{
    highLightLight(light, Color(1.0f, 1.0f, 0.0f));
}

void Visualizer::highLightLight(Light *light, Color color)
{
    gl::pushMatrices();
    gl::translate(vec3(light->position));
    gl::scale(vec3(LIGHT_SPHERE_SIZE));
    gl::ScopedColor scopedColor(Color(color.r, color.g, color.b));
    mWireCube->draw();
    gl::popMatrices();
}


void Visualizer::drawLight(Light *light)
{
    gl::pushMatrices();
    gl::translate(vec3(light->position));
    gl::scale(vec3(LIGHT_SPHERE_SIZE));
    mLightShader->uniform("LightColor", light->color);
    mLightShader->uniform("LightIntensity", light->intensity);
    mLight->draw();
    gl::popMatrices();

}
