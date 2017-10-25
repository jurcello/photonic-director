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
#include <uuid/uuid.h>

class InputChannel;
typedef std::shared_ptr<InputChannel> InputChannelRef;

class InputChannel {
public:
    static InputChannelRef create(std::string name, std::string address, std::string uuid = "");
    void setAdrress(std::string address);
    void setName(const std::string name);
    void setValue(float value);
    float getValue();
    std::string getUuid();
    std::string getAddress();
    std::string getName() const;
    
protected:
    InputChannel(std::string name, std::string address, std::string uuid = "");
    float mValue;
    std::string mUuid;
    std::string mAddress;
    std::string mName;
};

// I know that this is not system indepedent, but this can easily be fixed once needed.
inline std::string generate_uuid()
{
    uuid_t uuid;
    uuid_generate_random(uuid);
    char s[37];
    uuid_unparse(uuid, s);
    return s;
}

#endif /* Effects_h */
