#include "gl.h"
#include "window.h"
#include "renderer.h"
#include "texture.h"
#include "core/gameinfo.h"
#include "core/gamemenu.h"
#include "core/camera.h"
#include "core/physics.h"
#include "core/model.h"
#include "core/npc.h"
#include "core/audio.h"
#include "core/map.h"
#include "core/skybox.h"
#include "core/map_registry.h"
#include "scripting/lua_state.h"
#include "ui/ui_renderer.h"
#include "ui/menu_renderer.h"
#include "ui/map_select.h"
#include "ui/font.h"
#include <glm/glm.hpp>
#include <windows.h>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <functional>

enum class GameState { Menu, MapSelect, Playing };

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::string forcedMap;
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "+map") {
            forcedMap = argv[i + 1];
            break;
        }
    }
    if (!forcedMap.empty()) {
        std::cout << "[main] Forced map: " << forcedMap << "\n";
    }

    // === Lua ===
    ml::LuaState lua;
    lua.DoFile("game/scripts/hello.lua");

    std::string giArg = (argc > 1) ? argv[1] : "";
    auto gameInfo = ml::GameInfo::Load(giArg);
    if (!gameInfo) {
        MessageBoxA(nullptr, "gameinfo.txt not found", "AndyBox", MB_ICONERROR);
        return 1;
    }
    gameInfo->Dump();

    auto gameMenu = ml::GameMenu::Load("game/menu/gamemenu.txt");
    if (!gameMenu) {
        MessageBoxA(nullptr, "gamemenu.txt not found", "AndyBox", MB_ICONERROR);
        return 1;
    }
    gameMenu->Dump();

    auto maps = ml::MapRegistry::ScanDetailed("game/maps");

    try {
        ml::Window window(1280, 720, "ANDY'S-BOX - " + gameInfo->title);

        if (!gladLoadGL()) {
            MessageBoxA(nullptr, "gladLoadGL failed", "AndyBox", MB_ICONERROR);
            return 1;
        }
        printf("OpenGL:   %s\n", glGetString(GL_VERSION));

        ml::Renderer renderer;
        if (!renderer.Init()) return 1;

        ml::UIRenderer ui;
        if (!ui.Init()) return 1;

        ml::Skybox skyboxDay;
        {
            std::string dir = "game/textures/skybox/";
            skyboxDay.LoadFromFiles(
                dir + "right.png", dir + "left.png", dir + "top.png",
                dir + "bottom.png", dir + "front.png", dir + "back.png"
            );
        }
        ml::Skybox skyboxNight;
        {
            std::string dir = "game/textures/skybox_night/";
            skyboxNight.LoadFromFiles(
                dir + "right.png", dir + "left.png", dir + "top.png",
                dir + "bottom.png", dir + "front.png", dir + "back.png"
            );
        }
        ml::Skybox* activeSkybox = &skyboxDay;

        ml::Font font;
        bool fontOk = font.Load("assets/consola.ttf", 32.0f);

        ml::MenuRenderer menu;
        menu.Init(*gameMenu, gameMenu->background);
        if (fontOk) menu.SetFont(&font);

        ml::MapSelectMenu mapSelect;
        mapSelect.Init(maps);
        if (fontOk) mapSelect.SetFont(&font);

        std::unordered_map<std::string, std::shared_ptr<ml::Texture>> textureCache;
        auto getTexture = [&](const std::string& name) -> std::shared_ptr<ml::Texture> {
            auto it = textureCache.find(name);
            if (it != textureCache.end()) return it->second;
            auto tex = std::make_shared<ml::Texture>();
            std::string path = "assets/" + name;
            if (!tex->LoadFromFile(path)) {
                path = "game/textures/" + name;
                if (!tex->LoadFromFile(path)) {
                    std::cerr << "Tex not found: " << name << "\n";
                }
            }
            textureCache[name] = tex;
            return tex;
        };

        std::unordered_map<std::string, std::shared_ptr<ml::Model>> modelCache;
        auto getModel = [&](const std::string& file) -> std::shared_ptr<ml::Model> {
            auto it = modelCache.find(file);
            if (it != modelCache.end()) return it->second;
            auto m = std::make_shared<ml::Model>();
            std::string p1 = "game/models/" + file;
            std::string p2 = "assets/" + file;
            if (std::filesystem::exists(p1)) m->LoadOBJ(p1);
            else m->LoadOBJ(p2);
            modelCache[file] = m;
            return m;
        };

        ml::Model weaponModel;
        if (!weaponModel.LoadMLM("game/models/ak47.mlm")) {
            std::cerr << "[Weapon] ak47.mlm not found, trying obj...\n";
            weaponModel.LoadOBJ("game/models/ak47.obj");
        }

        std::unique_ptr<ml::Map> currentMap;
        int currentMapIndex = -1;

        ml::PhysWorld phys;
        ml::Npc npc;
        npc.Init(glm::vec3(3.0f, 0.0f, 5.0f));

        std::vector<std::string> npcTexNames = {
            "back.png", "car2.png", "clown.png",
            "help me.png", "sos.png", "ya ne zna.png"
        };
        int   npcTexIndex    = 0;
        float npcTexTimer    = 0.0f;
        const float npcTexPeriod = 0.4f;

        ml::Audio audio;
        audio.Init();
        ml::Camera camera;
        camera.eyeHeight = camera.standHeight;
        camera.collHeight = camera.collStandH;
        glm::vec3 feetPos = { 0.0f, 0.0f, 5.0f };

        // === Смерть игрока ===
        bool  playerDead = false;
        float deathTimer = 0.0f;
        float deathShakePhase = 0.0f;
        const float DEATH_DURATION  = 5.0f;
        const float DEATH_KILL_DIST = 0.0f;  // отключено в SDK

        // === Красное небо (раз в 20 сек) ===
        float bloodTimer  = 0.0f;
        bool  bloodActive = false;
        const float BLOOD_PERIOD = 999999.0f;  // отключено в SDK
        const float BLOOD_DURATION = 2.5f;
        camera.SetFeetPosition(feetPos);
        camera.UpdateVectors();

        GameState state = GameState::Menu;
        // audio music отключён в SDK; audio.StopAmbient();
        float t_global = 0.0f;
        float t_map = 0.0f;

        auto start = std::chrono::steady_clock::now();
        auto lastFrame = start;
        auto lastFps = start;
        int frames = 0;

        auto loadMap = [&](int idx) {
            if (idx < 0 || idx >= (int)maps.size()) return;
            currentMap = ml::Map::Load(maps[idx].path);
            if (!currentMap) return;
            currentMapIndex = idx;
            currentMap->Dump();
            t_map = 0.0f;

            phys.Clear();
            for (auto& b : currentMap->brushes) {
                glm::vec3 half = b.size * 0.5f;
                ml::AABB box;
                box.min = b.pos - half;
                box.max = b.pos + half;
                phys.AddBox(box);
            }

            feetPos = currentMap->spawn.pos;
            camera.position = feetPos;
            camera.position.y += camera.eyeHeight;
            camera.yaw = currentMap->spawn.yaw;
            camera.pitch = 0.0f;
            camera.velocity = {0,0,0};
            camera.UpdateVectors();
            // Переключение скайбокса по карте
            if (currentMap && currentMap->skybox == "night") {
                activeSkybox = &skyboxNight;
            } else {
                activeSkybox = &skyboxDay;
            }
        };
        // === Сохранение ===
        auto saveGame = [&]() {
            std::ofstream f("save.txt");
            if (!f) { std::cerr << "[Save] cannot write save.txt\n"; return; }
            f << "map " << currentMapIndex << "\n";
            f << "player " << feetPos.x << " " << feetPos.y << " " << feetPos.z << "\n";
            f << "camera " << camera.yaw << " " << camera.pitch << "\n";
            f << "npc " << npc.position.x << " " << npc.position.y << " " << npc.position.z << "\n";
            std::cout << "[Save] Saved (map=" << currentMapIndex
                      << " player=(" << feetPos.x << "," << feetPos.y << "," << feetPos.z << "))\n";
        };

        // === Загрузка ===
        auto loadGame = [&]() -> bool {
            std::ifstream f("save.txt");
            if (!f) { std::cerr << "[Save] no save.txt\n"; return false; }
            std::string key;
            while (f >> key) {
                if (key == "map") {
                    int m = 0; f >> m;
                    loadMap(m);
                } else if (key == "player") {
                    f >> feetPos.x >> feetPos.y >> feetPos.z;
                    camera.position = feetPos;
                    camera.position.y += camera.eyeHeight;
                } else if (key == "camera") {
                    f >> camera.yaw >> camera.pitch;
                } else if (key == "npc") {
                    f >> npc.position.x >> npc.position.y >> npc.position.z;
                }
            }
            camera.velocity = {0,0,0};
            camera.UpdateVectors();
            std::cout << "[Save] Loaded (map=" << currentMapIndex
                      << " player=(" << feetPos.x << "," << feetPos.y << "," << feetPos.z << "))\n";
            return true;
        };


        if (!forcedMap.empty()) {
            ml::MapEntry forced;
            forced.path = forcedMap;
            forced.name = forcedMap;
            maps.clear();
            maps.push_back(forced);
            loadMap(0);
            state = GameState::Playing;
                audio.StopMusic(); // audio ambient отключён в SDK;
        }

        while (window.PumpMessages()) {
            auto now = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(now - lastFrame).count();
            lastFrame = now;
            if (dt > 0.05f) dt = 0.05f;
            t_global += dt;
            renderer.SetTime(t_global);

            POINT mp;
            GetCursorPos(&mp);
            ScreenToClient(window.Handle(), &mp);
            float mouseX = (float)mp.x;
            float mouseY = (float)mp.y;

            bool lmbPressed = window.WasLeftClicked();

            if (state == GameState::MapSelect) {
                if (window.IsMouseCaptured()) window.SetMouseCaptured(false);

                if (lmbPressed) {
                    int result = mapSelect.HandleClick(mouseX, mouseY,
                                                       window.Width(), window.Height());
                    if (result == -1) {
                        state = GameState::Menu;
                // audio music отключён в SDK; audio.StopAmbient();
                    } else if (result >= 0) {
                        loadMap(result);
                        state = GameState::Playing;
                audio.StopMusic(); // audio ambient отключён в SDK;
                    }
                }
                if (window.WasKeyPressed(VK_ESCAPE)) state = GameState::Menu;
                // audio music отключён в SDK; audio.StopAmbient();

                renderer.BeginFrame(window.Width(), window.Height(), camera, 0, 0, 0);
                renderer.EndFrame();
                ui.Begin(window.Width(), window.Height());
                mapSelect.Render(ui, window.Width(), window.Height(), mouseX, mouseY);
                ui.End();
                window.SwapBuffers();
            }
            else if (state == GameState::Menu) {
                if (window.IsMouseCaptured()) window.SetMouseCaptured(false);

                if (lmbPressed) {
                    ml::MenuAction a = menu.HandleClick(mouseX, mouseY,
                                                        window.Width(), window.Height());
                    switch (a) {
                        case ml::MenuAction::StartGame:
                            state = GameState::MapSelect;
                            break;
                        case ml::MenuAction::Load:
                            if (loadGame()) {
                                state = GameState::Playing;
                                audio.StopMusic();
                                // audio ambient отключён в SDK;
                            }
                            break;
                        case ml::MenuAction::Quit:
                            return 0;
                        default: break;
                    }
                }

                renderer.BeginFrame(window.Width(), window.Height(), camera, 0, 0, 0);
                renderer.EndFrame();
                ui.Begin(window.Width(), window.Height());
                menu.Render(ui, window.Width(), window.Height(), mouseX, mouseY);
                ui.End();
                window.SwapBuffers();
            }
            else {
                if (lmbPressed && !window.IsMouseCaptured()) {
                    window.SetMouseCaptured(true);
                }
                if (window.WasKeyPressed(VK_ESCAPE)) state = GameState::Menu;
                // audio music отключён в SDK; audio.StopAmbient();

                float mdx, mdy;
                window.GetMouseDelta(mdx, mdy);
                if (window.IsMouseCaptured()) camera.ProcessMouse(mdx, mdy);

                bool w = window.IsKeyDown('W');
                bool s = window.IsKeyDown('S');
                bool a = window.IsKeyDown('A');
                bool d = window.IsKeyDown('D');
                bool crouch = window.IsKeyDown(VK_CONTROL);
                bool jump = window.IsKeyDown(VK_SPACE);   // зажат = авто-прыжок
                if (window.WasKeyPressed(VK_F7)) saveGame();

                glm::vec3 forward = camera.front;
                forward.y = 0.0f;
                if (glm::length(forward) > 0.001f) forward = glm::normalize(forward);
                glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

                glm::vec3 wishDir(0.0f);
                if (w) wishDir += forward;
                if (s) wishDir -= forward;
                if (d) wishDir += right;
                if (a) wishDir -= right;

                if (!playerDead) {
                bool shift = window.IsKeyDown(VK_SHIFT);
                    camera.ApplyInput(dt, wishDir, jump, crouch, shift, w);
                }

                bool onGround = false;
                if (!playerDead) {
                    feetPos = phys.MoveCharacter(feetPos,
                                                  camera.collRadius,
                                                  camera.collHeight,
                                                  camera.velocity,
                                                  dt,
                                                  onGround);
                }
                camera.onGround = onGround;
                camera.SetFeetPosition(feetPos);
                npc.Update(dt, feetPos, phys);

                // === ДЕТЕКТ СМЕРТИ ===
                if (!playerDead) {
                    glm::vec3 npcCenter    = npc.position + glm::vec3(0.0f, npc.height * 0.5f, 0.0f);
                    glm::vec3 playerCenter = feetPos + glm::vec3(0.0f, camera.eyeHeight * 0.5f, 0.0f);
                    float d = glm::length(npcCenter - playerCenter);
                    static float dbgT = 0.0f;
                    dbgT += dt;
                    if (dbgT > 0.4f) {
                        dbgT = 0.0f;
                        std::cout << "[NPC] d=" << d
                                  << " npc=(" << npcCenter.x << "," << npcCenter.y << "," << npcCenter.z << ")"
                                  << " plr=(" << playerCenter.x << "," << playerCenter.y << "," << playerCenter.z << ")"
                                  << " dead=" << (playerDead ? 1 : 0) << "\n";
                    }
                    if (d < DEATH_KILL_DIST) {
                        playerDead = true;
                        deathTimer = 0.0f;
                        deathShakePhase = 0.0f;
                        audio.StopMusic(); // audio ambient отключён в SDK;
                        // audio death отключён
                    }
                }
                if (playerDead) {
                    deathTimer += dt;
                    deathShakePhase += dt * 45.0f;
                    if (deathTimer >= DEATH_DURATION) {
                        playerDead = false;
                        deathTimer = 0.0f;
                        loadMap(currentMapIndex);
                        npc.Init(glm::vec3(3.0f, 0.0f, 5.0f));
                    }
                }
                // === Красное небо: таймер ===
                bloodTimer += dt;
                if (!bloodActive && bloodTimer >= BLOOD_PERIOD) {
                    bloodActive = true;
                    bloodTimer = 0.0f;
                }
                if (bloodActive && bloodTimer >= BLOOD_DURATION) {
                    bloodActive = false;
                    bloodTimer = 0.0f;
                }

                float bloodK = 0.0f;
                if (bloodActive) {
                    float tt = bloodTimer / BLOOD_DURATION;
                    bloodK = (tt < 0.3f) ? (tt / 0.3f)
                           : (tt < 0.7f) ? 1.0f
                           : (1.0f - (tt - 0.7f) / 0.3f);
                }
                glm::vec3 skyTint = glm::mix(
                    glm::vec3(1.0f),
                    glm::vec3(2.5f, 0.08f, 0.08f),
                    bloodK
                );
                if (activeSkybox) activeSkybox->SetTint(skyTint);

                // npcTexTimer отключён
                if (false) {
                    npcTexTimer -= npcTexPeriod;
                    npcTexIndex = (npcTexIndex + 1) % (int)npcTexNames.size();
                }
                camera.aspect = (float)window.Width() / (float)window.Height();

                t_map += dt;

                glm::vec3 sky = currentMap ? currentMap->skyColor : glm::vec3(0.05f, 0.06f, 0.09f);

                // Shadow pass
                glm::vec3 lightDir = currentMap
                    ? glm::normalize(currentMap->light.dir)
                    : glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));
                // === ТЕСТ: 3 хардкод-лампы ===
                {
                    std::vector<ml::PointLight> pls;
                    ml::PointLight pl1;
                    pl1.pos   = { feetPos.x,        feetPos.y + 3.0f, feetPos.z - 8.0f };
                    pl1.color = { 1.0f, 0.9f, 0.6f };
                    pl1.range = 12.0f;
                    pls.push_back(pl1);

                    ml::PointLight pl2;
                    pl2.pos   = { feetPos.x - 6.0f, feetPos.y + 3.0f, feetPos.z + 5.0f };
                    pl2.color = { 1.0f, 0.5f, 0.3f };
                    pl2.range = 10.0f;
                    pls.push_back(pl2);

                    ml::PointLight pl3;
                    pl3.pos   = { feetPos.x + 6.0f, feetPos.y + 3.0f, feetPos.z + 5.0f };
                    pl3.color = { 0.4f, 0.6f, 1.0f };
                    pl3.range = 10.0f;
                    pls.push_back(pl3);

                    renderer.SetLights(pls);
                }

                renderer.BeginShadowPass(lightDir);
                if (currentMap) {
                    for (auto& b : currentMap->brushes) {
                        auto tex = getTexture(b.texture);
                        if (!tex) continue;
                        glm::mat4 m(1.0f);
                        m = glm::translate(m, b.pos);
                        m = glm::scale(m, b.size);
                        renderer.DrawCubeScaled(m, *tex, b.tint);
                    }
                }

                // Main pass
                renderer.BeginFrame(window.Width(), window.Height(), camera,
                                    sky.x, sky.y, sky.z);

                if (activeSkybox && activeSkybox->IsValid()) {
                    activeSkybox->Render(camera.View(), camera.Projection());
                }

                if (currentMap) {
                    for (auto& b : currentMap->brushes) {
                        auto tex = getTexture(b.texture);
                        glm::mat4 m(1.0f);
                        m = glm::translate(m, b.pos);
                        m = glm::scale(m, b.size);
                        renderer.DrawCubeScaled(m, *tex, b.tint);
                    }

                {
                    glm::mat4 npcM(1.0f);
                    npcM = glm::translate(npcM, npc.position + glm::vec3(0.0f, npc.height * 0.5f, 0.0f));
                    float npcYaw = std::atan2(feetPos.x - npc.position.x, feetPos.z - npc.position.z);
                    npcM = glm::rotate(npcM, npcYaw, glm::vec3(0.0f, 1.0f, 0.0f));
                    npcM = glm::scale(npcM, glm::vec3(2.0f, npc.height, 0.3f));
                    auto npcTex = getTexture("white.png");
                    if (npcTex) renderer.DrawCubeScaled(npcM, *npcTex, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
                }
                    for (auto& e : currentMap->models) {
                        auto mdl = getModel(e.file);
                        if (!mdl || !mdl->IsValid()) continue;
                        glm::mat4 m(1.0f);
                        m = glm::translate(m, e.pos);
                        if (e.spin.x != 0) m = glm::rotate(m, t_map * e.spin.x, glm::vec3(1,0,0));
                        if (e.spin.y != 0) m = glm::rotate(m, t_map * e.spin.y, glm::vec3(0,1,0));
                        if (e.spin.z != 0) m = glm::rotate(m, t_map * e.spin.z, glm::vec3(0,0,1));
                        m = glm::scale(m, e.scale);
                        renderer.DrawModel(*mdl, m);
                    }
                }

                if (weaponModel.IsValid()) {
                    renderer.BeginViewModel(camera);

                    glm::mat4 weapon(1.0f);
                    weapon = glm::translate(weapon, glm::vec3(0.25f, -0.25f, -0.5f));
                    weapon = glm::rotate(weapon, glm::radians(-5.0f), glm::vec3(0, 0, 1));
                    weapon = glm::scale(weapon, glm::vec3(0.05f));

                    renderer.DrawWeapon(weaponModel, weapon);
                }

                renderer.EndFrame();

                // === ЭКРАН СМЕРТИ (поверх всего) ===
                if (playerDead) {
                    auto deathTex = getTexture(npcTexNames[npcTexIndex]);
                    if (deathTex) {
                        ui.Begin(window.Width(), window.Height());
                        // NDC: (-1..+1)
                        float zoom = (deathTimer > 2.5f) ? (1.0f + (deathTimer - 2.5f) * 0.15f) : 1.0f;
                        float shakeX = std::sin(deathShakePhase) * 0.02f * zoom;
                        float shakeY = std::cos(deathShakePhase * 1.3f) * 0.02f * zoom;
                        float half = zoom;
                        float x = -half + shakeX;
                        float y = -half + shakeY;
                        float w = half * 2.0f;
                        float h = half * 2.0f;
                        ui.DrawTexturedRect(glm::vec2(x, y), glm::vec2(w, h),
                                            *deathTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                        ui.End();
                    }
                }

                window.SwapBuffers();
            }

            frames++;
            if (std::chrono::duration<float>(now - lastFps).count() >= 1.0f) {
                const char* stateName = (state == GameState::Menu) ? "MENU"
                                      : (state == GameState::MapSelect) ? "SELECT"
                                      : "PLAYING";
                char title[240];
                std::snprintf(title, sizeof(title), "%s [%s] - %d FPS - map: %s",
                              gameInfo->title.c_str(), stateName, frames,
                              currentMap ? currentMap->name.c_str() : "-");
                SetWindowTextA(window.Handle(), title);
                frames = 0;
                lastFps = now;
            }
        }

        renderer.Shutdown();
        ui.Shutdown();
        menu.Shutdown();
    } catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "AndyBox Error", MB_ICONERROR);
        return 1;
    }
    return 0;
}
