//
//  Utils.h
//  PhotonicDirector
//
//  Created by Jur de Vries on 25/10/2017.
//

#ifndef Utils_h
#define Utils_h

#include <uuid/uuid.h>


// I know that this is not system indepedent, but this can easily be fixed once needed.
namespace photonic {
    inline std::string generate_uuid()
    {
        uuid_t uuid;
        uuid_generate_random(uuid);
        char s[37];
        uuid_unparse(uuid, s);
        return s;
    }    
}

#endif /* Utils_h */
