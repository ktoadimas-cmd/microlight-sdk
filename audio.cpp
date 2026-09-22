#include "audio.h"
#include <iostream>

// miniaudio РЎРѓ РЎР‚Р ВµР В°Р В»Р С‘Р В·Р В°РЎвЂ Р С‘Р ВµР в„– РІР‚вЂќ РЎвЂљР С•Р В»РЎРЉР С”Р С• Р В·Р Т‘Р ВµРЎРѓРЎРЉ
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

namespace ml {

bool Audio::Init() {
    ma_engine* eng = new ma_engine();
    if (ma_engine_init(NULL, eng) != MA_SUCCESS) {
        std::cerr << "[Audio] engine init failed\n";
        delete eng;
        return false;
    }
    engine_ = eng;
    ready_ = true;
    std::cout << "[Audio] initialized\n";
    return true;
}

void Audio::Shutdown() {
    if (music_) {
        ma_sound* s = (ma_sound*)music_;
        ma_sound_uninit(s);
        delete s;
        music_ = nullptr;
    }
    if (engine_) {
        ma_engine* e = (ma_engine*)engine_;
        ma_engine_uninit(e);
        delete e;
        engine_ = nullptr;
    }
    ready_ = false;
}

bool Audio::PlayMusic(const std::string& path, float volume) {
    if (!ready_) return false;

    // Р РЋРЎвЂљР С•Р С— РЎРѓРЎвЂљР В°РЎР‚РЎС“РЎР‹
    StopMusic();

    ma_sound* s = new ma_sound();
    if (ma_sound_init_from_file((ma_engine*)engine_, path.c_str(),
                                 MA_SOUND_FLAG_STREAM, NULL, NULL, s) != MA_SUCCESS) {
        std::cerr << "[Audio] cannot load " << path << "\n";
        delete s;
        return false;
    }

    ma_sound_set_looping(s, MA_TRUE);
    ma_sound_set_volume(s, volume);
    ma_sound_start(s);
    music_ = s;
    std::cout << "[Audio] music: " << path << "\n";
    return true;
}

void Audio::StopMusic() {
    if (music_) {
        ma_sound* s = (ma_sound*)music_;
        ma_sound_stop(s);
        ma_sound_uninit(s);
        delete s;
        music_ = nullptr;
    }
}

void Audio::SetMusicVolume(float v) {
    if (music_) {
        ma_sound_set_volume((ma_sound*)music_, v);
    }
}

void Audio::PlaySoundFile(const std::string& path, float volume) {
    if (!ready_) return;
    ma_engine_play_sound((ma_engine*)engine_, path.c_str(), NULL);
    // volume РЎвЂЎР ВµРЎР‚Р ВµР В· engine Р Р…Р ВµР В»РЎРЉР В·РЎРЏ, Р Р…Р С• Р Т‘Р В»РЎРЏ Р С•Р Т‘Р Р…Р С•РЎР‚Р В°Р В·Р С•Р Р†Р С•Р С–Р С• РЎвЂ¦Р Р†Р В°РЎвЂљР С‘РЎвЂљ
}

// === Фоновый гул (loop, отдельный от music) ===
bool Audio::PlayAmbient(const std::string& path, float volume) {
    if (!ready_) return false;

    // Останавливаем старый (если есть)
    StopAmbient();

    ma_sound* s = new ma_sound();
    if (ma_sound_init_from_file((ma_engine*)engine_, path.c_str(),
                                 MA_SOUND_FLAG_STREAM, NULL, NULL, s) != MA_SUCCESS) {
        std::cerr << "[Audio] cannot load ambient " << path << "\n";
        delete s;
        return false;
    }

    ma_sound_set_looping(s, MA_TRUE);
    ma_sound_set_volume(s, volume);
    ma_sound_set_spatialization_enabled(s, MA_FALSE);   // фон, не 3D
    ma_sound_start(s);
    ambient_ = s;
    std::cout << "[Audio] ambient: " << path << "\n";
    return true;
}

void Audio::StopAmbient() {
    if (ambient_) {
        ma_sound* s = (ma_sound*)ambient_;
        ma_sound_stop(s);
        ma_sound_uninit(s);
        delete s;
        ambient_ = nullptr;
    }
}

} // namespace ml
