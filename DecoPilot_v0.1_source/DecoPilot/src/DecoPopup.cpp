#include "DecoPopup.hpp"
#include "LevelAnalyzer.hpp"
#include "DecoEngine.hpp"
#include "History.hpp"

using namespace geode::prelude;

namespace decopilot {

DecoPopup* DecoPopup::create(LevelEditorLayer* editor) {
    auto* ret = new DecoPopup();
    if (ret && ret->init(editor)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool DecoPopup::init(LevelEditorLayer* editor) {
    if (!Popup::init(430.f, 265.f)) return false;
    m_editor = editor;
    setTitle("DecoPilot");

    auto* subtitle = CCLabelBMFont::create("AI chooses the style. The mod places safe decoration.", "bigFont.fnt");
    subtitle->setScale(.30f);
    subtitle->setColor({200, 220, 255});
    subtitle->setPosition({215.f, 220.f});
    m_mainLayer->addChild(subtitle);

    m_prompt = TextInput::create(370.f, "Describe the decoration style...");
    m_prompt->setCommonFilter(CommonFilter::Any);
    m_prompt->setMaxCharCount(1200);
    m_prompt->setTextAlign(TextInputAlign::Left);
    m_prompt->setPosition({215.f, 183.f});
    m_prompt->setString("Clean modern decoration, vibrant but readable, polished rated-level look. Preserve gameplay perfectly.");
    m_mainLayer->addChild(m_prompt);

    m_model = TextInput::create(220.f, "Ollama model");
    m_model->setCommonFilter(CommonFilter::Any);
    m_model->setMaxCharCount(80);
    m_model->setPosition({145.f, 144.f});
    m_model->setString("qwen2.5vl:7b");
    m_mainLayer->addChild(m_model);

    auto* localLabel = CCLabelBMFont::create("Local / free Ollama", "bigFont.fnt");
    localLabel->setScale(.28f);
    localLabel->setPosition({328.f, 144.f});
    m_mainLayer->addChild(localLabel);

    auto makeSmallButton = [&](char const* text, CCPoint pos, SEL_MenuHandler cb) {
        auto* btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(text, 70, true, "bigFont.fnt", "GJ_button_04.png", 25.f, .55f),
            this,
            cb
        );
        btn->setPosition(pos);
        m_buttonMenu->addChild(btn);
        return btn;
    };

    makeSmallButton("Modern", {72.f, 106.f}, menu_selector(DecoPopup::onPresetModern));
    makeSmallButton("Glow", {165.f, 106.f}, menu_selector(DecoPopup::onPresetGlow));
    makeSmallButton("Minimal", {258.f, 106.f}, menu_selector(DecoPopup::onPresetMinimal));
    makeSmallButton("Tech", {351.f, 106.f}, menu_selector(DecoPopup::onPresetTech));

    auto* generate = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Decorate", 110, true, "bigFont.fnt", "GJ_button_01.png", 30.f, .65f),
        this,
        menu_selector(DecoPopup::onGenerate)
    );
    generate->setPosition({145.f, 58.f});
    m_buttonMenu->addChild(generate);

    auto* undo = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Undo AI", 100, true, "bigFont.fnt", "GJ_button_06.png", 30.f, .60f),
        this,
        menu_selector(DecoPopup::onUndo)
    );
    undo->setPosition({285.f, 58.f});
    m_buttonMenu->addChild(undo);

    m_status = CCLabelBMFont::create("Ready. Original gameplay is locked by design.", "bigFont.fnt");
    m_status->setScale(.26f);
    m_status->setPosition({215.f, 27.f});
    m_mainLayer->addChild(m_status);

    return true;
}

void DecoPopup::setStatus(std::string const& text, ccColor3B color) {
    if (!m_status) return;
    m_status->setString(text.c_str());
    m_status->setColor(color);
    m_status->limitLabelWidth(390.f, .26f, .13f);
}

void DecoPopup::applyPreset(std::string const& prompt) {
    if (m_prompt) m_prompt->setString(prompt);
}

void DecoPopup::onPresetModern(CCObject*) {
    applyPreset("Modern clean rated-level decoration. Strong geometric block design, crisp outlines, restrained glow, layered background, sparse air deco, high readability. Pick a cohesive vibrant palette.");
}
void DecoPopup::onPresetGlow(CCObject*) {
    applyPreset("Polished glow style with strong silhouettes, luminous exposed edges, deep dark background, controlled bloom-like accents, readable gameplay, no visual clutter.");
}
void DecoPopup::onPresetMinimal(CCObject*) {
    applyPreset("Minimal modern decoration: simple shapes, lots of breathing room, subtle outlines, very low air deco, elegant palette, almost no glow, maximum readability.");
}
void DecoPopup::onPresetTech(CCObject*) {
    applyPreset("Clean futuristic tech style: panels, circuit-like geometry, sharp outlines, controlled neon accents, layered technical background, sparse particles, clear gameplay path.");
}

void DecoPopup::onGenerate(CCObject*) {
    if (!m_editor || LevelEditorLayer::get() != m_editor) {
        setStatus("Editor changed. Reopen DecoPilot.", {255, 100, 100});
        return;
    }
    if (m_ollama.isPending()) {
        setStatus("Already planning...", {255, 220, 120});
        return;
    }

    auto prompt = std::string(m_prompt->getString());
    auto model = std::string(m_model->getString());
    if (prompt.empty()) prompt = "Modern clean decoration with high readability.";
    if (model.empty()) model = "qwen2.5vl:7b";

    auto summary = LevelAnalyzer::summarize(m_editor);
    setStatus("Planning style locally with Ollama...", {120, 220, 255});

    m_ollama.plan(model, prompt, summary, [this](Result<StylePlan> result) {
        if (!m_editor || LevelEditorLayer::get() != m_editor) return;

        if (result.isErr()) {
            setStatus(result.unwrapErr(), {255, 100, 100});
            return;
        }

        setStatus("Style ready. Building safe decoration...", {140, 255, 160});
        auto gen = DecoEngine::generate(m_editor, result.unwrap());
        if (gen.isErr()) {
            setStatus(gen.unwrapErr(), {255, 100, 100});
            return;
        }

        auto stats = gen.unwrap();
        setStatus(
            fmt::format("Done: {} deco objects over {} gameplay anchors. Gameplay untouched.", stats.placed, stats.anchors),
            {120, 255, 150}
        );
    });
}

void DecoPopup::onUndo(CCObject*) {
    if (!m_editor || LevelEditorLayer::get() != m_editor) return;
    if (History::get().undo(m_editor)) {
        setStatus(fmt::format("Undid last AI generation. {} version(s) left.", History::get().size()), {255, 220, 120});
    } else {
        setStatus("Nothing from DecoPilot to undo.", {220, 220, 220});
    }
}

} // namespace decopilot
