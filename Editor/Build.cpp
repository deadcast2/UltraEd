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
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "Settings.h"
#include "shlwapi.h"
#include "PubSub.h"

namespace UltraEd
{
    bool Build::WriteSpecFile(const std::vector<Actor *> &actors)
    {
        std::string specSegments, specIncludes;
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

        std::vector<std::string> resourceCache;
        int loopCount = 0;
        for (const auto &actor : actors)
        {
            std::string newResName = Util::NewResourceName(loopCount++);

            if (actor->GetType() != ActorType::Model) continue;

            std::map<std::string, std::string> resources = actor->GetResources();

            if (find(resourceCache.begin(), resourceCache.end(), resources["vertexDataPath"])
                == resourceCache.end())
            {
                std::string id = Util::GuidToString(actor->GetId());
                id.insert(0, Util::RootPath().append("\\"));
                id.append(".rom.sos");

                std::string modelName(newResName);
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
                std::string path = resources["textureDataPath"];
                int width, height, channels;
                unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 3);
                if (data)
                {
                    auto dimensions = static_cast<Model *>(actor)->TextureDimensions();
                    if (stbir_resize_uint8(data, width, height, 0, data, dimensions[0], dimensions[1], 0, 3))
                    {
                        path.append(".rom.png");
                        stbi_write_png(path.c_str(), dimensions[0], dimensions[1], 3, data, 0);
                    }

                    stbi_image_free(data);
                }

                std::string textureName(newResName);
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

        std::string specPath = GetPathFor("Engine\\spec");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(specPath.c_str(), "w"), fclose);
        if (file == NULL) return false;

        std::string slashesNormalized(specHeader);
        slashesNormalized = std::regex_replace(slashesNormalized, std::regex("\\\\"), "\\\\");
        fwrite(slashesNormalized.c_str(), 1, slashesNormalized.size(), file.get());

        slashesNormalized = std::string(specSegments);
        slashesNormalized = std::regex_replace(slashesNormalized, std::regex("\\\\"), "\\\\");
        fwrite(slashesNormalized.c_str(), 1, slashesNormalized.size(), file.get());

        fwrite(specIncludeStart, 1, strlen(specIncludeStart), file.get());
        fwrite(specIncludes.c_str(), 1, specIncludes.size(), file.get());
        fwrite(specIncludeEnd, 1, strlen(specIncludeEnd), file.get());
        return true;
    }

    bool Build::WriteDefinitionsFile()
    {
        char buffer[128];
        std::string mode = Settings::GetVideoMode() == VideoMode::NTSC ? "OS_VI_NTSC_LAN1" : "OS_VI_PAL_LAN1";
        sprintf(buffer, "#define _UER_VIDEO_MODE %s\n", mode.c_str());

        std::string path = GetPathFor("Engine\\definitions.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(path.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(buffer, 1, strlen(buffer), file.get());
        return true;
    }

    bool Build::WriteSegmentsFile(const std::vector<Actor *> &actors, std::map<std::string, std::string> *resourceCache)
    {
        std::string romSegments;
        int loopCount = 0;
        for (const auto &actor : actors)
        {
            std::string newResName = Util::NewResourceName(loopCount++);

            if (actor->GetType() != ActorType::Model) continue;

            std::map<std::string, std::string> resources = actor->GetResources();

            if (resourceCache->find(resources["vertexDataPath"]) == resourceCache->end())
            {
                std::string modelName(newResName);
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
                std::string textureName(newResName);
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

        std::string segmentsPath = GetPathFor("Engine\\segments.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(segmentsPath.c_str(), "w"), fclose);
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

        std::string scenePath = GetPathFor("Engine\\scene.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(scenePath.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(buffer, 1, strlen(buffer), file.get());
        return true;
    }

    bool Build::WriteActorsFile(const std::vector<Actor *> &actors, const std::map<std::string, std::string> &resourceCache)
    {
        int actorCount = -1;
        std::string totalActors = std::to_string(actors.size());
        std::string actorInits, modelDraws;

        std::string actorsArrayDef("const int _UER_ActorCount = ");
        actorsArrayDef.append(totalActors).append(";\nactor *_UER_Actors[")
            .append(totalActors).append("];\n").append("actor *_UER_ActiveCamera = NULL;\n");

        for (const auto &actor : actors)
        {
            std::string resourceName = Util::NewResourceName(++actorCount);
            actorInits.append("\n\t_UER_Actors[").append(std::to_string(actorCount)).append("] = ");

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

                std::string modelName(resourceName);
                modelName.append("_M");

                if (resources.count("textureDataPath"))
                    actorInits.append("(actor*)loadTexturedModel(_");
                else
                    actorInits.append("(actor*)loadModel(_");

                actorInits.append(modelName).append("SegmentRomStart, _").append(modelName).append("SegmentRomEnd");

                if (resources.count("textureDataPath"))
                {
                    if (resourceCache.find(resources.at("textureDataPath")) != resourceCache.end())
                        resourceName = resourceCache.at(resources.at("textureDataPath"));

                    std::string textureName(resourceName);
                    textureName.append("_T");

                    auto dimensions = static_cast<Model *>(actor)->TextureDimensions();
                    actorInits.append(", _").append(textureName).append("SegmentRomStart, _")
                        .append(textureName).append("SegmentRomEnd, ").append(std::to_string(dimensions[0])).append(", ")
                        .append(std::to_string(dimensions[1]));
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
                std::vector<Vertex> vertices = actor->GetVertices();
                std::string id = Util::GuidToString(actor->GetId());
                id.insert(0, Util::RootPath().append("\\")).append(".rom.sos");
                FILE *file = fopen(id.c_str(), "w");
                if (file == NULL) return false;
                fprintf(file, "%i\n", static_cast<int>(vertices.size()));
                for (size_t i = 0; i < vertices.size(); i++)
                {
                    Vertex vert = vertices[i];
                    D3DXCOLOR color(vert.color);
                    fprintf(file, "%f %f %f %f %f %f %f %f %f\n", vert.position.x, vert.position.y, vert.position.z,
                        color.r, color.g, color.b, color.a, vert.tu, vert.tv);
                }
                fclose(file);

                modelDraws.append("\n\tmodelDraw(_UER_Actors[").append(std::to_string(actorCount)).append("], display_list);\n");
            }
            else if (actor->GetType() == ActorType::Camera)
            {
                actorInits.append("(actor*)createCamera(");
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

        std::string actorInitsPath = GetPathFor("Engine\\actors.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(actorInitsPath.c_str(), "w"), fclose);
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

    bool Build::WriteCollisionFile(const std::vector<Actor *> &actors)
    {
        std::string collideSetStart("void _UER_Collide() {");
        std::string collisions;
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

                std::string actorScript = actor->GetScript();
                std::string subActorScript = (*subActor)->GetScript();

                // Both actors must define a collide method.
                if (actorScript.find("collide(") == std::string::npos || subActorScript.find("collide(") == std::string::npos)
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

        std::string collisionPath = GetPathFor("Engine\\collisions.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(collisionPath.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(collideSetStart.c_str(), 1, collideSetStart.size(), file.get());
        fwrite(collisions.c_str(), 1, collisions.size(), file.get());
        fwrite("}", 1, 1, file.get());
        return true;
    }

    bool Build::WriteScriptsFile(const std::vector<Actor *> &actors)
    {
        std::string scriptStartStart("void _UER_Start() {");
        std::string scriptUpdateStart("\n\nvoid _UER_Update() {");
        std::string inputStart("\n\nvoid _UER_Input(NUContData gamepads[4]) {");

        std::string scripts;
        char countBuffer[10];
        int actorCount = -1;

        for (const auto &actor : actors)
        {
            std::string actorRef;
            actorRef.append("_UER_Actors[");

            std::string newResName = Util::NewResourceName(++actorCount);
            std::string script = actor->GetScript();
            auto result = std::unique_ptr<char>(Util::ReplaceString(script.c_str(), "$", newResName.c_str()));

            _itoa(actorCount, countBuffer, 10);
            actorRef.append(countBuffer).append("]->");
            result = std::unique_ptr<char>(Util::ReplaceString(result.get(), "self->", actorRef.c_str()));
            scripts.append(result.get()).append("\n\n");

            if (scripts.find(std::string(newResName).append("start(")) != std::string::npos)
            {
                scriptStartStart.append("\n\t").append(newResName).append("start();\n");
            }

            if (scripts.find(std::string(newResName).append("update(")) != std::string::npos)
            {
                scriptUpdateStart.append("\n\t").append(newResName).append("update();\n");
            }

            if (scripts.find(std::string(newResName).append("input(")) != std::string::npos)
            {
                inputStart.append("\n\t").append(newResName).append("input(gamepads);\n");
            }
        }

        std::string scriptsPath = GetPathFor("Engine\\scripts.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(scriptsPath.c_str(), "w"), fclose);
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

    bool Build::WriteMappingsFile(const std::vector<Actor *> &actors)
    {
        std::string mappingsStart("void _UER_Mappings() {");
        int loopCount = 0;
        char countBuffer[10];

        for (const auto &actor : actors)
        {
            _itoa(loopCount++, countBuffer, 10);
            mappingsStart.append("\n\tinsert(\"").append(actor->GetName()).append("\", ")
                .append(countBuffer).append(");\n");
        }

        std::string mappingsPath = GetPathFor("Engine\\mappings.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(mappingsPath.c_str(), "w"), fclose);
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
        std::map<std::string, std::string> resourceCache;
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
            std::string currDir = GetPathFor("Player");

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
            std::string currDir = GetPathFor("Player\\USB");

            // Start the USB loader with no window.
            std::string command = Settings::GetBuildCart() == BuildCart::_64drive ?
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
                auto buffer = std::make_unique<char[]>(dwRead + 1); // Add 1 to prevent garbage.
                if (buffer)
                {
                    memcpy(buffer.get(), chBuf, dwRead);
                    auto output = std::string("Build Output: ").append(buffer.get());
                    PubSub::Publish("AppendToConsole", &output);
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
            std::string currDir = GetPathFor("Engine");

            // Start the build with no window.
            CreateProcess(NULL, const_cast<LPSTR>("cmd /c build.bat"), NULL, NULL, TRUE, CREATE_NO_WINDOW, 
                NULL, currDir.c_str(), &si, &pi);

            DWORD dwRead;
            CHAR chBuf[4096];
            CloseHandle(stdOutWrite);

            // Send the build results to the console window.
            while (ReadFile(stdOutRead, chBuf, 4096, &dwRead, NULL))
            {
                if (dwRead == 0) break;
                auto buffer = std::make_unique<char[]>(dwRead + 1); // Add 1 to prevent garbage.
                if (buffer)
                {
                    memcpy(buffer.get(), chBuf, dwRead);
                    auto output = std::string("Build Output: ").append(buffer.get());
                    PubSub::Publish("AppendToConsole", &output);
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

    std::string Build::GetPathFor(const std::string &name)
    {
        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            std::string path(buffer);

        #ifdef _DEBUG
            return path.append("\\..\\..\\..\\").append(name);
        #else
            return path.append("\\..\\").append(name);
        #endif
        }
        return std::string();
    }
}
