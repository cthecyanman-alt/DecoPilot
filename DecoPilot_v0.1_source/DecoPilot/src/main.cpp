#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include "DecoPopup.hpp"

using namespace geode::prelude;
using decopilot::DecoPopup;

class $modify(DecoPilotPauseLayer, EditorPauseLayer) {
    struct Fields {
        LevelEditorLayer* editor = nullptr;
    };

    bool init(LevelEditorLayer* layer) {
        if (!EditorPauseLayer::init(layer)) return false;
        m_fields->editor = layer;

        auto* menu = getChildByID("actions-menu");
        if (!menu) menu = getChildByID("guidelines-menu");
        if (!menu) menu = getChildByID("small-actions-menu");
        if (!menu) return true;

        auto* sprite = ButtonSprite::create("AI Deco", 70, true, "bigFont.fnt", "GJ_button_02.png", 25.f, .55f);
        auto* button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(DecoPilotPauseLayer::onDecoPilot)
        );
        button->setID("decopilot-button"_spr);
        menu->addChild(button);
        menu->updateLayout();
        return true;
    }

    void onDecoPilot(CCObject*) {
        auto* editor = m_fields->editor ? m_fields->editor : LevelEditorLayer::get();
        if (!editor) return;
        if (auto* popup = DecoPopup::create(editor)) popup->show();
    }
};
