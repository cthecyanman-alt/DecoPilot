#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include "OllamaClient.hpp"

namespace decopilot {

class DecoPopup : public geode::Popup {
    LevelEditorLayer* m_editor = nullptr;
    geode::TextInput* m_prompt = nullptr;
    geode::TextInput* m_model = nullptr;
    cocos2d::CCLabelBMFont* m_status = nullptr;
    OllamaClient m_ollama;

protected:
    bool init(LevelEditorLayer* editor);
    void onGenerate(cocos2d::CCObject*);
    void onUndo(cocos2d::CCObject*);
    void onPresetModern(cocos2d::CCObject*);
    void onPresetGlow(cocos2d::CCObject*);
    void onPresetMinimal(cocos2d::CCObject*);
    void onPresetTech(cocos2d::CCObject*);
    void setStatus(std::string const& text, cocos2d::ccColor3B color = cocos2d::ccWHITE);
    void applyPreset(std::string const& prompt);

public:
    static DecoPopup* create(LevelEditorLayer* editor);
};

} // namespace decopilot
