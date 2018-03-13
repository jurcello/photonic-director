//
//  Effects.hpp
//  PhotonicDirector
//
//  Created by Jur de Vries on 24/10/2017.
//

#ifndef Effects_h
#define Effects_h

#include <stdio.h>
#include "cinder/app/App.h"
#include <ctime>
#include "Light.h"

#define REGISTER_TYPE(klass) \
class klass##Factory : public EffectFactory { \
public: \
    klass##Factory() { \
        Effect::registerType(#klass, this); \
    } \
    virtual EffectRef create(std::string name) { \
        return EffectRef(new klass(name)); \
    } \
    virtual EffectRef create(std::string name, std::string uuid) { \
        return EffectRef(new klass(name, uuid)); \
    } \
}; \
static klass##Factory global_##klass##Factory;


namespace photonic {
    class InputChannel;
    typedef std::shared_ptr<InputChannel> InputChannelRef;
    
    class InputChannel {
    public:
        enum Type {
            kType_Dim1,
            kType_Dim2,
            kType_Dim3,
        };
        static InputChannelRef create(std::string name, std::string address, std::string uuid = "");
        void setAdrress(std::string address);
        void setName(const std::string name);
        void setValue(float value);
        void setValue(vec2 value);
        void setValue(vec3 value);
        float getValue();
        vec2 getVec2Value();
        vec3 getVec3Value();

        void setType(Type type);
        Type getType();

        std::string getUuid();
        std::string getAddress();
        std::string getName() const;
        
    protected:
        InputChannel(std::string name, std::string address, std::string uuid = "");
        float mValue;
        vec2 mVec2Value;
        vec3 mVec3Value;
        Type mType;
        std::string mUuid;
        std::string mAddress;
        std::string mName;
    };
    
    struct Parameter {
        enum Type {
            kType_Float,
            kType_Int,
            kType_Color,
            kType_Vector3,
            kType_Channel,
            kType_Channel_MinMax,
        };
        Parameter();
        Parameter(Type type, std::string description = "");
      
        Type type;
        std::string description;
        float floatValue;
        int intValue;
        ColorA colorValue;
        vec3 vec3Value;
        // Type channel and channel min max.
        InputChannelRef channelRef;
        float minIn, min, maxIn, max;

        void setValue(float value);
        void setValue(int value);
        void setValue(ColorA value);
        void setValue(vec3 value);
        void setValue(vec4 minMax);

        float getMappedChannelValue();
    };
    
    class Effect;
    typedef std::shared_ptr<Effect> EffectRef;
    
    // Virtual factory to create new effects.
    class EffectFactory {
    public:
        virtual EffectRef create(std::string name) = 0;
        virtual EffectRef create(std::string name, std::string uuid) = 0;
    };
    
    class Effect {
    public:
        enum Status {
            kStatus_On,
            kStatus_Off,
            kStatus_FadingOut,
            kStatus_FadingIn,
        };

        enum Stage {
            kStage_Main,
            kStage_After,
        };
        
        static void registerType(const std::string type, EffectFactory* factory);
        static EffectRef create(std::string type, std::string name);
        static EffectRef create(std::string type, std::string name, std::string uuid);
        static std::vector<std::string> getTypes();
        
        Effect(std::string name, std::string uuid = "");
        ~Effect();
        
        std::string getUuid();
        std::string getName();
        std::string getStatusName();
        Status getStatus();
        double getFadeValue();
        
        void setName(std::string name);
        void addLight(LightRef light);
        void removeLight(LightRef light);
        void toggleLight(LightRef light);
        bool hasLight(LightRef light);
        std::vector<LightRef> getLights();
        void setChannel(InputChannelRef channel);
        InputChannelRef getChannel();
        // Params section.
        Parameter* getParam(int index);
        std::map<int, Parameter*>& getParams();

        template<typename ENUM_TYPE, typename PARAM_INDEX, typename DATA_TYPE>
        void registerParam(ENUM_TYPE type, PARAM_INDEX index, DATA_TYPE initialValue, std::string description) {
            Parameter* newParam = new Parameter(type, description);
            newParam->setValue(initialValue);
            mParams[index] = newParam;
        };

        template<typename ENUM_TYPE, typename PARAM_INDEX>
        void registerParam(ENUM_TYPE type, PARAM_INDEX index, std::string description) {
            Parameter* newParam = new Parameter(type, description);
            newParam->channelRef = nullptr;
            mParams[index] = newParam;
        };

        
        virtual std::string getTypeClassName() = 0;
        virtual std::string getTypeName() = 0;
        virtual void drawEditGui();
        virtual void execute(double dt);
        virtual Stage getStage();
        bool hasOutput();
        virtual void init();
        virtual void visualize();

        // Public accessable variables. Part of the interface!
        float fadeTime;
        bool isTurnedOn;
        float weight;
        
    protected:
        std::string mUuid;
        std::string mName;
        std::vector<LightRef> mLights;
        InputChannelRef mChannel;
        Status mStatus;
        double mStatusChangeTime;
        double mFadeValue;
        std::map<int, Parameter*> mParams;

        ColorA interPolateColors(ColorA color1, ColorA color2, double intensity);
        
    private:
        static std::map<std::string, EffectFactory*> factories;
        static std::vector<std::string> types;
    };
    
    class SimpleVolumeEffect : public Effect {
    public:
        enum Inputs {
            kInput_BaseColor = 1,
            kInput_EffectColor = 2,
            kInput_Volume = 3,
        };
        SimpleVolumeEffect(std::string name, std::string uuid = "");

        virtual void execute(double dt);
        virtual std::string getTypeName();
        virtual std::string getTypeClassName();
    };
    
    class StaticValueEffect : public Effect {
    public:
        enum Inputs {
            kInput_Volume = 1,
            kInput_Color = 2,
        };
        
        StaticValueEffect(std::string name, std::string uuid = "");
        
        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;

    };
}

namespace ph = photonic;

#endif /* Effects_h */
