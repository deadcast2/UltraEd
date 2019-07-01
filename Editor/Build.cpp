#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <regex>
#include "vendor/stb_image.h"
#include "vendor/stb_image_resize.h"
#include "vendor/stb_image_write.h"
#include "build.h"
#include "util.h"
#include "debug.h"

namespace UltraEd
{
    bool CBuild::WriteSpecFile(vector<CActor*> actors)
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

        int loopCount = 0;
        for (auto actor : actors)
        {          
            string newResName = CUtil::NewResourceName(loopCount++);

            if (actor->GetType() != ActorType::Model) continue;

            string id = CUtil::GuidToString(actor->GetId());
            id.insert(0, CUtil::RootPath().append("\\"));
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

            map<string, string> resources = actor->GetResources();
            if (resources.count("textureDataPath"))
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
            }
        }

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string specPath(buffer);
            specPath.append("\\..\\..\\Engine\\spec");
            FILE *file = fopen(specPath.c_str(), "w");
            if (file == NULL) return false;

            string slashesNormalized(specHeader);
            slashesNormalized = regex_replace(slashesNormalized, regex("\\\\"), "\\\\");
            fwrite(slashesNormalized.c_str(), 1, slashesNormalized.size(), file);

            slashesNormalized = string(specSegments);
            slashesNormalized = regex_replace(slashesNormalized, regex("\\\\"), "\\\\");
            fwrite(slashesNormalized.c_str(), 1, slashesNormalized.size(), file);

            fwrite(specIncludeStart, 1, strlen(specIncludeStart), file);
            fwrite(specIncludes.c_str(), 1, specIncludes.size(), file);
            fwrite(specIncludeEnd, 1, strlen(specIncludeEnd), file);
            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::WriteSegmentsFile(vector<CActor*> actors)
    {
        string romSegments;
        int loopCount = 0;
        for (auto actor : actors)
        {           
            string newResName = CUtil::NewResourceName(loopCount++);

            if (actor->GetType() != ActorType::Model) continue;

            string modelName(newResName);
            modelName.append("_M");

            romSegments.append("extern u8 _");
            romSegments.append(modelName);
            romSegments.append("SegmentRomStart[];\n");
            romSegments.append("extern u8 _");
            romSegments.append(modelName);
            romSegments.append("SegmentRomEnd[];\n");

            map<string, string> resources = actor->GetResources();
            if (resources.count("textureDataPath"))
            {
                string textureName(newResName);
                textureName.append("_T");

                romSegments.append("extern u8 _");
                romSegments.append(textureName);
                romSegments.append("SegmentRomStart[];\n");
                romSegments.append("extern u8 _");
                romSegments.append(textureName);
                romSegments.append("SegmentRomEnd[];\n");
            }
        }

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string segmentsPath(buffer);
            segmentsPath.append("\\..\\..\\Engine\\segments.h");
            FILE *file = fopen(segmentsPath.c_str(), "w");
            if (file == NULL) return false;
            fwrite(romSegments.c_str(), 1, romSegments.size(), file);
            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::WriteActorsFile(vector<CActor*> actors)
    {
        int actorCount = -1;
        char countBuffer[10];
        string actorInits, modelDraws;

        _itoa(actors.size(), countBuffer, 10);
        string actorsArrayDef("const int _UER_ActorCount = ");
        actorsArrayDef.append(countBuffer).append(";\nstruct actor *_UER_Actors[")
            .append(countBuffer).append("];\n");

        for (auto actor : actors)
        {
            string resourceName = CUtil::NewResourceName(++actorCount);

            string modelName(resourceName);
            modelName.append("_M");

            string textureName(resourceName);
            textureName.append("_T");

            _itoa(actorCount, countBuffer, 10);
            actorInits.append("\n\t_UER_Actors[").append(countBuffer).append("] = ");

            if (actor->GetType() == ActorType::Model)
            {
                auto resources = actor->GetResources();

                if (resources.count("textureDataPath"))
                {
                    actorInits.append("(struct actor*)load_model_with_texture(_");
                }
                else
                {
                    actorInits.append("(struct actor*)load_model(_");
                }

                actorInits.append(modelName).append("SegmentRomStart, _").append(modelName).append("SegmentRomEnd");

                if (resources.count("textureDataPath"))
                {
                    actorInits.append(", _").append(textureName).append("SegmentRomStart, _").append(textureName).append("SegmentRomEnd");
                }

                // Add transform data.
                char vectorBuffer[128];
                D3DXVECTOR3 position = actor->GetPosition(), scale = actor->GetScale(), axis;
                float angle;
                actor->GetAxisAngle(&axis, &angle);
                sprintf(vectorBuffer, ", %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf",
                    position.x, position.y, position.z,
                    axis.x, axis.y, axis.z, angle * (180 / D3DX_PI),
                    scale.x, scale.y, scale.z);
                actorInits.append(vectorBuffer).append(");\n");

                // Write out mesh data.
                vector<Vertex> vertices = actor->GetVertices();
                string id = CUtil::GuidToString(actor->GetId());
                id.insert(0, CUtil::RootPath().append("\\")).append(".rom.sos");
                FILE *file = fopen(id.c_str(), "w");
                if (file == NULL) return false;
                fprintf(file, "%i\n", vertices.size());
                for (unsigned int i = 0; i < vertices.size(); i++)
                {
                    Vertex vert = vertices[i];
                    fprintf(file, "%f %f %f %f %f\n", vert.position.x, vert.position.y, vert.position.z,
                        vert.tu, vert.tv);
                }
                fclose(file);

                modelDraws.append("\n\tmodel_draw(_UER_Actors[").append(countBuffer).append("], display_list);\n");
            }
            else if (actor->GetType() == ActorType::Camera)
            {
                actorInits.append("(struct actor*)create_camera(");
                char vectorBuffer[128];
                D3DXVECTOR3 position = actor->GetPosition();
                D3DXVECTOR3 axis;
                float angle;
                actor->GetAxisAngle(&axis, &angle);
                sprintf(vectorBuffer, "%lf, %lf, %lf, %lf, %lf, %lf, %lf", position.x, position.y, position.z,
                    axis.x, axis.y, axis.z, angle * (180 / D3DX_PI));
                actorInits.append(vectorBuffer).append(");\n");
            }
        }

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string actorInitsPath(buffer);
            actorInitsPath.append("\\..\\..\\Engine\\actors.h");
            FILE *file = fopen(actorInitsPath.c_str(), "w");
            if (file == NULL) return false;

            fwrite(actorsArrayDef.c_str(), 1, actorsArrayDef.size(), file);

            const char *actorInitStart = "\nvoid _UER_Load() {";
            fwrite(actorInitStart, 1, strlen(actorInitStart), file);
            fwrite(actorInits.c_str(), 1, actorInits.size(), file);
            fwrite("}", 1, 1, file);

            const char *drawStart = "\n\nvoid _UER_Draw(Gfx **display_list) {";
            fwrite(drawStart, 1, strlen(drawStart), file);
            fwrite(modelDraws.c_str(), 1, modelDraws.size(), file);
            fwrite("}", 1, 1, file);

            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::WriteCollisionFile(vector<CActor*> actors)
    {
        string collideSetStart("void _UER_Collide() {");
        string collisions;
        int collisionCount = 0;
        char countBuffer[10];

        for (auto actor : actors)
        {
            int subLoop = collisionCount++;

            if (!actor->GetCollider()) continue;

            for (auto subActor = next(actors.begin(), collisionCount); subActor != actors.end(); ++subActor)
            {
                subLoop++;

                if (!(*subActor)->GetCollider()) continue;

                _itoa(collisionCount - 1, countBuffer, 10);
                collisions.append("\n\tif(check_collision(_UER_Actors[").append(countBuffer);
                _itoa(subLoop, countBuffer, 10);
                collisions.append("], _UER_Actors[").append(countBuffer).append("]))\n\t{\n");

                collisions.append("\t\t").append(CUtil::NewResourceName(subLoop)).append("collide(");
                _itoa(collisionCount - 1, countBuffer, 10);
                collisions.append("_UER_Actors[").append(countBuffer).append("]);\n");

                collisions.append("\t\t").append(CUtil::NewResourceName(collisionCount - 1)).append("collide(");
                _itoa(subLoop, countBuffer, 10);
                collisions.append("_UER_Actors[").append(countBuffer).append("]);\n");

                collisions.append("\t}\n");
            }
        }

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string collisionPath(buffer);
            collisionPath.append("\\..\\..\\Engine\\collisions.h");
            FILE *file = fopen(collisionPath.c_str(), "w");
            if (file == NULL) return false;
            fwrite(collideSetStart.c_str(), 1, collideSetStart.size(), file);
            fwrite(collisions.c_str(), 1, collisions.size(), file);
            fwrite("}", 1, 1, file);
            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::WriteScriptsFile(vector<CActor*> actors)
    {
        string scriptStartStart("void _UER_Start() {");
        string scriptUpdateStart("\n\nvoid _UER_Update() {");
        string inputStart("\n\nvoid _UER_Input(NUContData gamepads[4]) {");

        string scripts;
        char countBuffer[10];
        int actorCount = -1;

        for (auto actor : actors)
        {
            string actorRef;
            actorRef.append("_UER_Actors[");

            string newResName = CUtil::NewResourceName(++actorCount);
            string script = actor->GetScript();
            char *result = CUtil::ReplaceString(script.c_str(), "@", newResName.c_str());

            _itoa(actorCount, countBuffer, 10);
            actorRef.append(countBuffer).append("]->");
            result = CUtil::ReplaceString(result, "self->", actorRef.c_str());
            scripts.append(result).append("\n\n");

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

            free(result);
        }

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string scriptsPath(buffer);
            scriptsPath.append("\\..\\..\\Engine\\scripts.h");
            FILE *file = fopen(scriptsPath.c_str(), "w");
            if (file == NULL) return false;
            fwrite(scripts.c_str(), 1, scripts.size(), file);
            fwrite(scriptStartStart.c_str(), 1, scriptStartStart.size(), file);
            fwrite("}", 1, 1, file);
            fwrite(scriptUpdateStart.c_str(), 1, scriptUpdateStart.size(), file);
            fwrite("}", 1, 1, file);
            fwrite(inputStart.c_str(), 1, inputStart.size(), file);
            fwrite("}", 1, 1, file);
            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::WriteMappingsFile(vector<CActor*> actors)
    {
        string mappingsStart("void _UER_Mappings() {");
        int loopCount = 0;
        char countBuffer[10];

        for (auto actor : actors)
        {
            _itoa(loopCount++, countBuffer, 10);
            mappingsStart.append("\n\tinsert(\"").append(actor->GetName()).append("\", ").append(countBuffer).append(");\n");
        }

        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            string mappingsPath(buffer);
            mappingsPath.append("\\..\\..\\Engine\\mappings.h");
            FILE *file = fopen(mappingsPath.c_str(), "w");
            if (file == NULL) return false;
            fwrite(mappingsStart.c_str(), 1, mappingsStart.size(), file);
            fwrite("}", 1, 1, file);
            fclose(file);
            return true;
        }
        return false;
    }

    bool CBuild::Start(vector<CActor*> actors)
    {
        WriteSpecFile(actors);
        WriteSegmentsFile(actors);
        WriteActorsFile(actors);
        WriteCollisionFile(actors);
        WriteScriptsFile(actors);
        WriteMappingsFile(actors);
        return Compile();
    }

    bool CBuild::Run()
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
            string currDir(buffer);
            currDir.append("\\..\\..\\Player");

            // Start the build with no window.
            CreateProcess(NULL, "cmd /c cen64.exe pifdata.bin ..\\Engine\\main.n64", NULL, NULL, FALSE,
                CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);

            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return exitCode == 0;
        }

        return false;
    }

    bool CBuild::Load()
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
            string currDir(buffer);
            currDir.append("\\..\\..\\Player\\USB");

            // Start the USB loader with no window.
            CreateProcess(NULL, "cmd /c 64drive_usb.exe -l ..\\..\\Engine\\main.n64 -c 6102", NULL, NULL, FALSE,
                CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);

            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return exitCode == 0;
        }

        return false;
    }

    bool CBuild::Compile()
    {
        // Set the root env variable for the N64 build tools.
        SetEnvironmentVariable("ROOT", "..\\Engine\\n64sdk\\ultra");

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
            string currDir(buffer);
            currDir.append("\\..\\..\\Engine");

            // Start the build with no window.
            CreateProcess(NULL, "cmd /c build.bat", NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);
            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return exitCode == 0;
        }

        return false;
    }
}
