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
#include "cinder/Xml.h"
#include "Light.h"
#include "Osc.h"
#include "MidiMessage.h"

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
        void setValue(double value);
        void setValue(float value);
        void setValue(int value);
        void setValue(vec2 value);
        void setValue(vec3 value);
        void setSmoothing(int smoothing);
        float getValue();
        int getIntValue();
        vec2 getVec2Value();
        vec3 getVec3Value();
        int getSmoothing();

        void setType(Type type);
        Type getType();

        std::string getUuid();
        std::string getAddress();
        std::string getName() const;
        
    protected:
        InputChannel(std::string name, std::string address, std::string uuid = "");
        float mValue;
        int mIntValue;
        vec2 mVec2Value;
        vec3 mVec3Value;
        Type mType;
        std::string mUuid;
        std::string mAddress;
        std::string mName;

        int mSmoothing;
        int mCurrentSmoothing;

        void updateCurrentSmooting();
    };
    
    struct Parameter {
        enum Type {
            kType_Float,
            kType_Int,
            kType_Color,
            kType_Vector3,
            kType_Channel,
            kType_Channel_MinMax,
            kType_OscTrigger,
            kType_Light,
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
        // Type light
        LightRef lightRef;

        // Osc Trigger related.
        std::string oscAdress;
        bool triggerValue;

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

    class EffectXmlSerializer;
    typedef std::shared_ptr<EffectXmlSerializer> EffectXmlSerializerRef;

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
        
        static void registerType(std::string type, EffectFactory* factory);
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
        std::vector<LightRef>::iterator removeLight(LightRef light);
        void toggleLight(LightRef light);
        bool hasLight(LightRef light);
        std::vector<LightRef> getLights();
        void setChannel(InputChannelRef channel);
        InputChannelRef getChannel();
        // Params section.
        Parameter* getParam(int index);
        std::map<int, Parameter*>& getParams();
        std::map<int, Parameter*>& getOrderedParamsForUI();

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
            newParam->lightRef = nullptr;
            mParams[index] = newParam;
        };

        
        virtual std::string getTypeClassName() = 0;
        virtual std::string getTypeName() = 0;
        virtual bool supportsLight(LightRef light);
        virtual void drawEditGui();
        virtual void execute(double dt);
        // TODO: implement using listener pattern.
        void listenToOsc(const osc::Message &message);
        void setOscSender(osc::SenderUdp *OscSender);
        virtual Stage getStage();
        virtual void listenToMidi(const smf::MidiMessage* message);
        bool hasOutput();
        virtual void init();
        virtual void visualize();
        virtual EffectXmlSerializerRef getXmlSerializer();

        // Public accessable variables. Part of the interface!
        float fadeInTime;
        float fadeOutTime;
        bool isTurnedOn;
        float weight;

        std::string oscAddressForOnOff;

        
    protected:
        std::string mUuid;
        std::string mName;
        std::vector<LightRef> mLights;
        InputChannelRef mChannel;
        Status mStatus;
        double mStatusChangeTime;
        double mFadeValue;
        std::map<int, Parameter*> mParams;
        std::map<int, Parameter*> mOrderedParams;
        osc::SenderUdp* mOscSender;

        ColorA interPolateColors(ColorA color1, ColorA color2, double intensity);
        
    private:
        static std::map<std::string, EffectFactory*> factories;
        static std::vector<std::string> types;
    };


    class EffectXmlSerializer;


    class EffectXmlSerializer {
    public:
        explicit EffectXmlSerializer(Effect* cEffect);

        virtual void writeEffect(XmlTree &xmlNode);
        virtual void readEffect(XmlTree &xmlNode, const std::vector<LightRef> &lights, std::vector<InputChannelRef> &channels);

    protected:
        Effect* mEffect;
    };


    class SimpleVolumeEffect : public Effect {
    public:
        enum Inputs {
            kInput_BaseColor = 1,
            kInput_EffectColor = 2,
            kInput_Volume = 3,
            kInput_DecaySpeed = 4,
        };
        SimpleVolumeEffect(std::string name, std::string uuid = "");

        virtual void execute(double dt);
        virtual std::string getTypeName();
        virtual std::string getTypeClassName();

    protected:
        void updateVolumes(float input, double dt);

        float mActualVolume;
        float mTargetVolume;
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
