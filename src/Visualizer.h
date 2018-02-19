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
#include "Effects.h"

const float LIGHT_SPHERE_SIZE = 0.2f;

using namespace ci;
using namespace ci::app;
using namespace photonic;

// This class is mainly intended to keep the main app more readable.
class Visualizer {
public:
    Visualizer();
    void setup(std::vector<LightRef> &lights);
    void mouseDown(MouseEvent event);
    void mouseUp(MouseEvent event);
    void mouseDrag(MouseEvent event);
    void mouseMove(MouseEvent event);
    void draw(std::vector<LightRef> lights);
    
    void enableEditingMode();
    void disableEditingMode();
    
    void resize();
    LightRef pickLight(std::vector<LightRef> lights);
    void highLightLight(LightRef light);
    void highLightLight(LightRef light, Color color);
    void drawEffects(std::vector<EffectRef> effect);
    
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
    
    void drawLight(LightRef light);
    bool mEditingMode;

};
#endif /* Visualizer_hpp */
