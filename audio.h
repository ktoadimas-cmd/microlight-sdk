#pragma once
#include <string>

namespace ml {

class Audio {
public:
    bool Init();
    void Shutdown();

    // Р вЂ”Р В°Р С–РЎР‚РЎС“Р В·Р С‘РЎвЂљРЎРЉ Р С‘ Р В·Р В°Р С—РЎС“РЎРѓРЎвЂљР С‘РЎвЂљРЎРЉ Р СРЎС“Р В·РЎвЂ№Р С”РЎС“ (loop)
    bool PlayMusic(const std::string& path, float volume = 0.5f);
    void StopMusic();
    void SetMusicVolume(float v);

    // Р С›Р Т‘Р Р…Р С•РЎР‚Р В°Р В·Р С•Р Р†РЎвЂ№Р в„– Р В·Р Р†РЎС“Р С” (Р Р†РЎвЂ№РЎРѓРЎвЂљРЎР‚Р ВµР В», РЎв‚¬Р В°Р С– Р С‘ РЎвЂљ.Р Т‘.)
    void PlaySoundFile(const std::string& path, float volume = 1.0f);

    // Фоновый гул (loop, играет параллельно с music)
    bool PlayAmbient(const std::string& path, float volume = 0.5f);
    void StopAmbient();

private:
    bool ready_ = false;
    void* engine_ = nullptr;  // ma_engine*
    void* music_  = nullptr;  // ma_sound*
    void* ambient_ = nullptr; // ma_sound* (для фонового гула)
};

} // namespace ml
