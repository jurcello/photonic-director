//
//  Visualizer.hpp
//  OpenGlTest
//
//  Created by Jur de Vries on 17/10/2017.
//

#ifndef Visualizer_h
#define Visualizer_h

#include <stdio.h>
#include "cinder/gl/gl.h"
#include "cinder/app/AppBase.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "Light.h"

const float LIGHT_SPHERE_SIZE = 0.2f;

using namespace ci;
using namespace ci::app;

// This class is mainly intended to keep the main app more readable.
class Visualizer {
public:
    void setup(std::vector<Light*> lights);
    void mouseDown(MouseEvent event);
    void mouseUp(MouseEvent event);
    void mouseDrag(MouseEvent event);
    void mouseMove(MouseEvent event);
    void draw(std::vector<Light*> lights);
    
    void resize();
    Light* pickLight(std::vector<Light*> lights);
    void highLightLight(Light* light);
    void highLightLight(Light* light, Color color);
    
protected:
    gl::BatchRef mCube, mLight;
    gl::TextureRef mTextureWood;
    gl::GlslProgRef mObjectShader, mLightShader;
    gl::UboRef mLightsUbo;
    
    gl::BatchRef        mWirePlane;
    gl::BatchRef        mWireCube;
    
    TriMeshRef mSphere;
    AxisAlignedBox mSphereBounds;
    
    CameraPersp mCam;
    CameraUi mCamUI;
    vec2 mMousePos;
    
    void drawLight(Light* light);
    
    
};
#endif /* Visualizer_hpp */
