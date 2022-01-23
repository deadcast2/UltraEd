#ifndef _CONFIRMATION_H_
#define _CONFIRMATION_H_

#include <functional>

namespace UltraEd 
{
    struct ModalProperties
    {
        bool IsOpen;
        bool IsDirty;
        std::function<void()> Yes;
        std::function<void()> No;
    };
}

#endif
