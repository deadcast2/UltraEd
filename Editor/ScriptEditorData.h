#ifndef _SCRIPTEDITORDATA_H_
#define _SCRIPTEDITORDATA_H_

#include <string>
#include <memory>
#include "Actor.h"
#include "Vendor/ImGui/Plugins/TextEditor.h"

namespace UltraEd
{
    typedef struct _ScriptEditorData {
        std::string name;
        Actor *actor;
        std::shared_ptr<TextEditor> textEditor;

        bool IsDirty() const {
            return this->textEditor->GetText() != this->actor->GetScript();
        }
    } ScriptEditorData;
}

#endif
