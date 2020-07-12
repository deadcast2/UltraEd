#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <regex>
#include <STB/stb_image.h>
#include <STB/stb_image_resize.h>
#include <STB/stb_image_write.h>
#include "build.h"
#include "util.h"
#include "debug.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "Settings.h"
#include "shlwapi.h"
#include "PubSub.h"

namespace UltraEd
{
    bool Build::WriteSpecFile(const vector<Actor *> &actors)
    {
        string specSegments, specIncludes;
        const char *specHeader = "#include <nusys.h>\n\n"
            "beginseg"
            "\n\tname \"code\""
            "\n\tflags BOOT OBJECT"
            "\n\tentry nuBoot"
            "\n\taddress NU_SPEC_BOOT_ADDR"
            "\n\tstack NU_SPEC_BOOT_STACK"
            "\n\tinclude \"codesegment.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\rspboot.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\aspMain.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DEX2.fifo.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspL3DEX2.fifo.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DEX2.Rej.fifo.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DEX2.NoN.fifo.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DLX2.Rej.fifo.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspS2DEX2.fifo.o\""
            "\nendseg\n";
        const char *specIncludeStart = "\nbeginwave"
            "\n\tname \"main\""
            "\n\tinclude \"code\"";
        const char *specIncludeEnd = "\nendwave";

        vector<string> resourceCache;
        int loopCount = 0;
        for (const auto &actor : actors)
        {
            string newResName = Util::NewResourceName(loopCount++);

            if (actor->GetType() != ActorType::Model) continue;
            
            map<string, string> resources = actor->GetResources();

            if (find(resourceCache.begin(), resourceCache.end(), resources["vertexDataPath"]) 
                == resourceCache.end())
            {
                string id = Util::GuidToString(actor->GetId());
                id.insert(0, Util::RootPath().append("\\"));
                id.append(".rom.sos");

                string modelName(newResName);
                modelName.append("_M");

                specSegments.append("\nbeginseg\n\tname \"");
                specSegments.append(modelName);
                specSegments.append("\"\n\tflags RAW\n\tinclude \"");
                specSegments.append(id);
                specSegments.append("\"\nendseg\n");

                specIncludes.append("\n\tinclude \"");
                specIncludes.append(modelName);
                specIncludes.append("\"");

                resourceCache.push_back(resources["vertexDataPath"]);
            }

            if (resources.count("textureDataPath") && 
                find(resourceCache.begin(), resourceCache.end(), resources["textureDataPath"])
                == resourceCache.end())
            {
                // Load the set texture and resize to required dimensions.
                string path = resources["textureDataPath"];
                int width, height, channels;
                unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 3);
                if (data)
                {
                    // Force 32 x 32 texture for now.
                    if (stbir_resize_uint8(data, width, height, 0, data, 32, 32, 0, 3))
                    {
                        path.append(".rom.png");
                        stbi_write_png(path.c_str(), 32, 32, 3, data, 0);
                    }

                    stbi_image_free(data);
                }

                string textureName(newResName);
                textureName.append("_T");

                specSegments.append("\nbeginseg\n\tname \"");
                specSegments.append(textureName);
                specSegments.append("\"\n\tflags RAW\n\tinclude \"");
                specSegments.append(path);
                specSegments.append("\"\nendseg\n");

                specIncludes.append("\n\tinclude \"");
                specIncludes.append(textureName);
                specIncludes.append("\"");

                resourceCache.push_back(resources["textureDataPath"]);
            }
        }

        string specPath = GetPathFor("Engine\\spec");
        unique_ptr<FILE, decltype(fclose) *> file(fopen(specPath.c_str(), "w"), fclose);
        if (file == NULL) return false;

        string slashesNormalized(specHeader);
        slashesNormalized = regex_replace(slashesNormalized, regex("\\\\"), "\\\\");
        fwrite(slashesNormalized.c_str(), 1, slashesNormalized.size(), file.get());

        slashesNormalized = string(specSegments);
        slashesNormalized = regex_replace(slashesNormalized, regex("\\\\"), "\\\\");
        fwrite(slashesNormalized.c_str(), 1, slashesNormalized.size(), file.get());

        fwrite(specIncludeStart, 1, strlen(specIncludeStart), file.get());
        fwrite(specIncludes.c_str(), 1, specIncludes.size(), file.get());
        fwrite(specIncludeEnd, 1, strlen(specIncludeEnd), file.get());
        return true;
    }

    bool Build::WriteDefinitionsFile()
    {
        char buffer[128];
        string mode = Settings::GetVideoMode() == VideoMode::NTSC ? "OS_VI_NTSC_LAN1" : "OS_VI_PAL_LAN1";
        sprintf(buffer, "#define _UER_VIDEO_MODE %s\n", mode.c_str());

        string path = GetPathFor("Engine\\definitions.h");
        unique_ptr<FILE, decltype(fclose) *> file(fopen(path.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(buffer, 1, strlen(buffer), file.get());
        return true;
    }

    bool Build::WriteSegmentsFile(const vector<Actor *> &actors, map<string, string> *resourceCache)
    {
        string romSegments;
        int loopCount = 0;
        for (const auto &actor : actors)
        {
            string newResName = Util::NewResourceName(loopCount++);

            if (actor->GetType() != ActorType::Model) continue;

            map<string, string> resources = actor->GetResources();
            
            if (resourceCache->find(resources["vertexDataPath"]) == resourceCache->end())
            {
                string modelName(newResName);
                modelName.append("_M");

                romSegments.append("extern u8 _");
                romSegments.append(modelName);
                romSegments.append("SegmentRomStart[];\n");
                romSegments.append("extern u8 _");
                romSegments.append(modelName);
                romSegments.append("SegmentRomEnd[];\n");

                (*resourceCache)[resources["vertexDataPath"]] = newResName;
            }

            if (resources.count("textureDataPath") && 
                resourceCache->find(resources["textureDataPath"]) == resourceCache->end())
            {
                string textureName(newResName);
                textureName.append("_T");

                romSegments.append("extern u8 _");
                romSegments.append(textureName);
                romSegments.append("SegmentRomStart[];\n");
                romSegments.append("extern u8 _");
                romSegments.append(textureName);
                romSegments.append("SegmentRomEnd[];\n");

                (*resourceCache)[resources["textureDataPath"]] = newResName;
            }
        }

        string segmentsPath = GetPathFor("Engine\\segments.h");
        unique_ptr<FILE, decltype(fclose) *> file(fopen(segmentsPath.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(romSegments.c_str(), 1, romSegments.size(), file.get());
        return true;
    }

    bool Build::WriteSceneFile(Scene *scene)
    {
        char buffer[128];
        COLORREF bgColor = scene->GetBackgroundColor();
        sprintf(buffer, "int _UER_SceneBackgroundColor[3] = { %i, %i, %i };\n", GetRValue(bgColor),
            GetGValue(bgColor), GetBValue(bgColor));

        string scenePath = GetPathFor("Engine\\scene.h");
        unique_ptr<FILE, decltype(fclose) *> file(fopen(scenePath.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(buffer, 1, strlen(buffer), file.get());
        return true;
    }

    bool Build::WriteActorsFile(const vector<Actor *> &actors, const map<string, string> &resourceCache)
    {
        int actorCount = -1;
        char countBuffer[10];
        string actorInits, modelDraws;

        _itoa(actors.size(), countBuffer, 10);
        string actorsArrayDef("const int _UER_ActorCount = ");
        actorsArrayDef.append(countBuffer).append(";\nactor *_UER_Actors[")
            .append(countBuffer).append("];\n").append("actor *_UER_ActiveCamera = NULL;\n");

        for (const auto &actor : actors)
        {
            string resourceName = Util::NewResourceName(++actorCount);

            _itoa(actorCount, countBuffer, 10);
            actorInits.append("\n\t_UER_Actors[").append(countBuffer).append("] = ");

            D3DXVECTOR3 colliderCenter = actor->HasCollider() ? actor->GetCollider()->GetCenter() : D3DXVECTOR3(0, 0, 0);
            FLOAT colliderRadius = actor->HasCollider() && actor->GetCollider()->GetType() == ColliderType::Sphere ?
                dynamic_cast<SphereCollider *>(actor->GetCollider())->GetRadius() : 0.0f;
            D3DXVECTOR3 colliderExtents = actor->HasCollider() && actor->GetCollider()->GetType() == ColliderType::Box ?
                dynamic_cast<BoxCollider *>(actor->GetCollider())->GetExtents() : D3DXVECTOR3(0, 0, 0);

            if (actor->GetType() == ActorType::Model)
            {
                const auto resources = actor->GetResources();

                if (resourceCache.find(resources.at("vertexDataPath")) != resourceCache.end())
                    resourceName = resourceCache.at(resources.at("vertexDataPath"));

                string modelName(resourceName);
                modelName.append("_M");

                if (resources.count("textureDataPath"))
                    actorInits.append("(actor*)load_model_with_texture(_");
                else
                    actorInits.append("(actor*)load_model(_");

                actorInits.append(modelName).append("SegmentRomStart, _").append(modelName).append("SegmentRomEnd");

                if (resources.count("textureDataPath"))
                {
                    if (resourceCache.find(resources.at("textureDataPath")) != resourceCache.end())
                        resourceName = resourceCache.at(resources.at("textureDataPath"));

                    string textureName(resourceName);
                    textureName.append("_T");

                    actorInits.append(", _").append(textureName).append("SegmentRomStart, _").append(textureName).append("SegmentRomEnd");
                }

                // Add transform data.
                char vectorBuffer[256];
                D3DXVECTOR3 position = actor->GetPosition(), scale = actor->GetScale(), axis;
                float angle;
                actor->GetAxisAngle(&axis, &angle);
                sprintf(vectorBuffer, ", %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %s",
                    position.x, position.y, position.z,
                    axis.x, axis.y, axis.z, angle * (180.0 / D3DX_PI),
                    scale.x, scale.y, scale.z,
                    colliderCenter.x, colliderCenter.y, colliderCenter.z, colliderRadius,
                    colliderExtents.x, colliderExtents.y, colliderExtents.z, 
                    actor->HasCollider() ? actor->GetCollider()->GetName() : "None");
                actorInits.append(vectorBuffer).append(");\n");

                // Write out mesh data.
                vector<Vertex> vertices = actor->GetVertices();
                string id = Util::GuidToString(actor->GetId());
                id.insert(0, Util::RootPath().append("\\")).append(".rom.sos");
                FILE *file = fopen(id.c_str(), "w");
                if (file == NULL) return false;
                fprintf(file, "%lu\n", vertices.size());
                for (size_t i = 0; i < vertices.size(); i++)
                {
                    Vertex vert = vertices[i];
                    D3DXCOLOR color(vert.color);
                    fprintf(file, "%f %f %f %f %f %f %f %f %f\n", vert.position.x, vert.position.y, vert.position.z,
                        color.r, color.g, color.b, color.a, vert.tu, vert.tv);
                }
                fclose(file);

                modelDraws.append("\n\tmodel_draw(_UER_Actors[").append(countBuffer).append("], display_list);\n");
            }
            else if (actor->GetType() == ActorType::Camera)
            {
                actorInits.append("(actor*)create_camera(");
                char vectorBuffer[256];
                D3DXVECTOR3 position = actor->GetPosition();
                D3DXVECTOR3 axis;
                float angle;
                actor->GetAxisAngle(&axis, &angle);
                sprintf(vectorBuffer, "%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %s",
                    position.x, position.y, position.z,
                    axis.x, axis.y, axis.z, angle * (180.0 / D3DX_PI),
                    colliderCenter.x, colliderCenter.y, colliderCenter.z, colliderRadius,
                    colliderExtents.x, colliderExtents.y, colliderExtents.z,
                    actor->HasCollider() ? actor->GetCollider()->GetName() : "None");
                actorInits.append(vectorBuffer).append(");\n");
            }
        }

        string actorInitsPath = GetPathFor("Engine\\actors.h");
        unique_ptr<FILE, decltype(fclose) *> file(fopen(actorInitsPath.c_str(), "w"), fclose);
        if (file == NULL) return false;

        fwrite(actorsArrayDef.c_str(), 1, actorsArrayDef.size(), file.get());

        const char *actorInitStart = "\nvoid _UER_Load() {";
        fwrite(actorInitStart, 1, strlen(actorInitStart), file.get());
        fwrite(actorInits.c_str(), 1, actorInits.size(), file.get());
        fwrite("}", 1, 1, file.get());

        const char *drawStart = "\n\nvoid _UER_Draw(Gfx **display_list) {";
        fwrite(drawStart, 1, strlen(drawStart), file.get());
        fwrite(modelDraws.c_str(), 1, modelDraws.size(), file.get());
        fwrite("}", 1, 1, file.get());
        return true;
    }

    bool Build::WriteCollisionFile(const vector<Actor *> &actors)
    {
        string collideSetStart("void _UER_Collide() {");
        string collisions;
        int collisionCount = 0;
        char countBuffer[10];

        for (const auto &actor : actors)
        {
            int subLoop = collisionCount++;

            if (!actor->GetCollider()) continue;

            for (auto subActor = next(actors.begin(), collisionCount); subActor != actors.end(); ++subActor)
            {
                subLoop++;

                if (!(*subActor)->GetCollider()) continue;

                string actorScript = actor->GetScript();
                string subActorScript = (*subActor)->GetScript();

                // Both actors must define a collide method.
                if (actorScript.find("collide(") == string::npos || subActorScript.find("collide(") == string::npos)
                    continue;

                _itoa(collisionCount - 1, countBuffer, 10);
                collisions.append("\n\tif(check_collision(_UER_Actors[").append(countBuffer);
                _itoa(subLoop, countBuffer, 10);
                collisions.append("], _UER_Actors[").append(countBuffer).append("]))\n\t{\n");

                collisions.append("\t\t").append(Util::NewResourceName(subLoop)).append("collide(");
                _itoa(collisionCount - 1, countBuffer, 10);
                collisions.append("_UER_Actors[").append(countBuffer).append("]);\n");

                collisions.append("\t\t").append(Util::NewResourceName(collisionCount - 1)).append("collide(");
                _itoa(subLoop, countBuffer, 10);
                collisions.append("_UER_Actors[").append(countBuffer).append("]);\n");

                collisions.append("\t}\n");
            }
        }

        string collisionPath = GetPathFor("Engine\\collisions.h");
        unique_ptr<FILE, decltype(fclose) *> file(fopen(collisionPath.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(collideSetStart.c_str(), 1, collideSetStart.size(), file.get());
        fwrite(collisions.c_str(), 1, collisions.size(), file.get());
        fwrite("}", 1, 1, file.get());
        return true;
    }

    bool Build::WriteScriptsFile(const vector<Actor *> &actors)
    {
        string scriptStartStart("void _UER_Start() {");
        string scriptUpdateStart("\n\nvoid _UER_Update() {");
        string inputStart("\n\nvoid _UER_Input(NUContData gamepads[4]) {");

        string scripts;
        char countBuffer[10];
        int actorCount = -1;

        for (const auto &actor : actors)
        {
            string actorRef;
            actorRef.append("_UER_Actors[");

            string newResName = Util::NewResourceName(++actorCount);
            string script = actor->GetScript();
            auto result = unique_ptr<char>(Util::ReplaceString(script.c_str(), "$", newResName.c_str()));

            _itoa(actorCount, countBuffer, 10);
            actorRef.append(countBuffer).append("]->");
            result = unique_ptr<char>(Util::ReplaceString(result.get(), "self->", actorRef.c_str()));
            scripts.append(result.get()).append("\n\n");

            if (scripts.find(string(newResName).append("start(")) != string::npos)
            {
                scriptStartStart.append("\n\t").append(newResName).append("start();\n");
            }

            if (scripts.find(string(newResName).append("update(")) != string::npos)
            {
                scriptUpdateStart.append("\n\t").append(newResName).append("update();\n");
            }

            if (scripts.find(string(newResName).append("input(")) != string::npos)
            {
                inputStart.append("\n\t").append(newResName).append("input(gamepads);\n");
            }
        }

        string scriptsPath = GetPathFor("Engine\\scripts.h");
        unique_ptr<FILE, decltype(fclose) *> file(fopen(scriptsPath.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(scripts.c_str(), 1, scripts.size(), file.get());
        fwrite(scriptStartStart.c_str(), 1, scriptStartStart.size(), file.get());
        fwrite("}", 1, 1, file.get());
        fwrite(scriptUpdateStart.c_str(), 1, scriptUpdateStart.size(), file.get());
        fwrite("}", 1, 1, file.get());
        fwrite(inputStart.c_str(), 1, inputStart.size(), file.get());
        fwrite("}", 1, 1, file.get());
        return true;
    }

    bool Build::WriteMappingsFile(const vector<Actor *> &actors)
    {
        string mappingsStart("void _UER_Mappings() {");
        int loopCount = 0;
        char countBuffer[10];

        for (const auto &actor : actors)
        {
            _itoa(loopCount++, countBuffer, 10);
            mappingsStart.append("\n\tinsert(\"").append(actor->GetName()).append("\", ")
                .append(countBuffer).append(");\n");
        }

        string mappingsPath = GetPathFor("Engine\\mappings.h");
        unique_ptr<FILE, decltype(fclose) *> file(fopen(mappingsPath.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(mappingsStart.c_str(), 1, mappingsStart.size(), file.get());
        fwrite("}", 1, 1, file.get());
        return true;
    }

    bool Build::Start(Scene *scene)
    {
        auto actors = scene->GetActors();

        // Share texture and model data to reduce ROM size. Resource use is tracked during
        // segment generation and the actor script generator uses that info. 
        map<string, string> resourceCache;
        WriteSegmentsFile(actors, &resourceCache);
        WriteActorsFile(actors, resourceCache);
        
        WriteSpecFile(actors);
        WriteDefinitionsFile();
        WriteCollisionFile(actors);
        WriteScriptsFile(actors);
        WriteMappingsFile(actors);
        WriteSceneFile(scene);
        
        return Compile(scene->GetWndHandle());
    }

    bool Build::Run()
    {
        // Get the path to where the program is running.
        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            DWORD exitCode;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            // Format the path to execute the ROM build.
            string currDir = GetPathFor("Player");

            // Start the build with no window.
            CreateProcess(NULL, const_cast<LPSTR>("cmd /c cen64.exe pifdata.bin ..\\Engine\\main.n64"), NULL, NULL, FALSE,
                CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);

            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return exitCode == 0;
        }

        return false;
    }

    bool Build::Load(const HWND &hWnd)
    {
        // Get the path to where the program is running.
        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            HANDLE stdOutRead = NULL;
            HANDLE stdOutWrite = NULL;

            SECURITY_ATTRIBUTES securityAttrs;
            securityAttrs.nLength = sizeof(SECURITY_ATTRIBUTES);
            securityAttrs.bInheritHandle = TRUE;
            securityAttrs.lpSecurityDescriptor = NULL;

            if (!CreatePipe(&stdOutRead, &stdOutWrite, &securityAttrs, 0))
                MessageBox(NULL, "Could not create pipe", "Pipe Error", NULL);

            if (!SetHandleInformation(stdOutRead, HANDLE_FLAG_INHERIT, 0))
                MessageBox(NULL, "Could set handle information for pipe", "Pipe Error", NULL);

            DWORD exitCode;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            si.hStdError = stdOutWrite;
            si.hStdOutput = stdOutWrite;
            si.dwFlags |= STARTF_USESTDHANDLES;
            ZeroMemory(&pi, sizeof(pi));

            // Format the path to execute the ROM build.
            string currDir = GetPathFor("Player\\USB");

            // Start the USB loader with no window.
            string command = Settings::GetBuildCart() == BuildCart::_64drive ? 
                "cmd /c 64drive_usb.exe -l ..\\..\\Engine\\main.n64 -c 6102" : 
                "cmd /c usb64.exe -rom=..\\..\\Engine\\main.n64 -start";
            CreateProcess(NULL, const_cast<LPSTR>(command.c_str()), NULL, NULL, TRUE, 
                CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);

            DWORD dwRead;
            CHAR chBuf[4096];
            CloseHandle(stdOutWrite);

            // Send the cart loading results to the output window.
            while (ReadFile(stdOutRead, chBuf, 4096, &dwRead, NULL))
            {
                if (dwRead == 0) break;
                auto buffer = make_unique<char[]>(dwRead + 1); // Add 1 to prevent garbage.
                if (buffer)
                {
                    memcpy(buffer.get(), chBuf, dwRead);
                    //SendMessage(hWnd, WM_COMMAND, TAB_BUILD_OUTPUT, reinterpret_cast<LPARAM>(buffer.release()));
                }
            }

            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            CloseHandle(stdOutRead);

            return exitCode == 0;
        }

        return false;
    }

    bool Build::Compile(const HWND &hWnd)
    {
        // Set the root env variable for the N64 build tools.
        SetEnvironmentVariable("ROOT", "..\\Engine\\n64sdk\\ultra");

        // Get the path to where the program is running.
        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            HANDLE stdOutRead = NULL;
            HANDLE stdOutWrite = NULL;

            SECURITY_ATTRIBUTES securityAttrs;
            securityAttrs.nLength = sizeof(SECURITY_ATTRIBUTES);
            securityAttrs.bInheritHandle = TRUE;
            securityAttrs.lpSecurityDescriptor = NULL;

            if (!CreatePipe(&stdOutRead, &stdOutWrite, &securityAttrs, 0))
                MessageBox(NULL, "Could not create pipe", "Pipe Error", NULL);

            if (!SetHandleInformation(stdOutRead, HANDLE_FLAG_INHERIT, 0))
                MessageBox(NULL, "Could set handle information for pipe", "Pipe Error", NULL);

            DWORD exitCode;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            si.hStdError = stdOutWrite;
            si.hStdOutput = stdOutWrite;
            si.dwFlags |= STARTF_USESTDHANDLES;
            ZeroMemory(&pi, sizeof(pi));

            // Format the path to execute the ROM build.
            string currDir = GetPathFor("Engine");

            // Start the build with no window.
            CreateProcess(NULL, const_cast<LPSTR>("cmd /c build.bat"), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);

            DWORD dwRead;
            CHAR chBuf[4096];
            CloseHandle(stdOutWrite);

            // Send the build results to the output window.
            PubSub::Publish("BuildOutputClear");
            while (ReadFile(stdOutRead, chBuf, 4096, &dwRead, NULL))
            {
                if (dwRead == 0) break;
                auto buffer = make_unique<char[]>(dwRead + 1); // Add 1 to prevent garbage.
                if (buffer)
                {
                    memcpy(buffer.get(), chBuf, dwRead);
                    PubSub::Publish("BuildOutputAppend", buffer.release());
                }
            }

            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            CloseHandle(stdOutRead);

            return exitCode == 0;
        }

        return false;
    }

    string Build::GetPathFor(const string &name)
    {
        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string path(buffer);

#ifdef _DEBUG
            return path.append("\\..\\..\\..\\").append(name);
#else
            return path.append("\\..\\").append(name);
#endif
        }
        return string();
    }
}
