#include "DecoEngine.hpp"
#include "History.hpp"
#include <unordered_set>
#include <vector>
#include <cmath>

using namespace geode::prelude;

namespace decopilot {
namespace {

struct GridKey {
    int x;
    int y;
    bool operator==(GridKey const& o) const { return x == o.x && y == o.y; }
};
struct GridHash {
    size_t operator()(GridKey const& k) const noexcept {
        return (static_cast<size_t>(static_cast<uint32_t>(k.x)) << 32) ^ static_cast<uint32_t>(k.y);
    }
};

GridKey gridKey(CCPoint p) {
    return { static_cast<int>(std::lround(p.x / 30.f)), static_cast<int>(std::lround(p.y / 30.f)) };
}

ccColor3B toColor(RGB c) {
    return ccc3(static_cast<GLubyte>(c.r), static_cast<GLubyte>(c.g), static_cast<GLubyte>(c.b));
}

int addColor(LevelEditorLayer* editor, RGB color) {
    auto id = editor->getNextColorChannel();
    if (id <= 0 || id >= 999) return -1;
    auto action = ColorAction::create(toColor(color), false, 0);
    if (!action || !editor->m_levelSettings || !editor->m_levelSettings->m_effectManager) return -1;
    editor->m_levelSettings->m_effectManager->setColorAction(action, id);
    return id;
}

GameObject* addDeco(
    LevelEditorLayer* editor,
    CCArray* created,
    int objectID,
    CCPoint pos,
    int colorID,
    ZLayer zLayer,
    float scale,
    float rotation,
    GLubyte opacity
) {
    auto* obj = editor->createObject(objectID, pos, true);
    if (!obj) return nullptr;

    obj->m_isNoTouch = true;
    obj->m_isDontFade = true;
    obj->m_isDontEnter = true;
    obj->m_zLayer = zLayer;
    obj->setRotation(rotation);
    obj->updateCustomScaleX(scale);
    obj->updateCustomScaleY(scale);
    obj->setOpacity(opacity);

    if (obj->m_baseColor && colorID > 0) {
        obj->m_baseColor->m_colorID = colorID;
        obj->updateMainColor();
        }
    if (obj->m_detailColor && colorID > 0) {
        obj->m_detailColor->m_colorID = colorID;
    }

    created->addObject(obj);
    return obj;
}

bool isSolidAnchor(GameObject* obj) {
    if (!obj || obj->m_isNoTouch) return false;
    return obj->m_objectType == GameObjectType::Solid || obj->m_objectType == GameObjectType::Breakable;
}

} // namespace

Result<GenerationStats> DecoEngine::generate(LevelEditorLayer* editor, StylePlan const& plan) {
    if (!editor || !editor->m_objects) return Err("Editor is not available.");

    // Snapshot original solid anchors BEFORE creating anything. This is the gameplay skeleton.
    std::vector<GameObject*> anchors;
    anchors.reserve(editor->m_objects->count());
    std::unordered_set<GridKey, GridHash> occupied;

    for (auto* obj : editor->m_objects->asExt<GameObject>()) {
        if (!isSolidAnchor(obj)) continue;
        anchors.push_back(obj);
        occupied.insert(gridKey(obj->getPosition()));
    }

    if (anchors.empty()) return Err("No solid gameplay blocks were found to decorate.");

    // Safety cap: we deliberately refuse runaway generations.
    constexpr size_t kMaxAnchors = 9000;
    if (anchors.size() > kMaxAnchors) {
        return Err(fmt::format("This level has {} solid anchors; v0.1 safety cap is {}.", anchors.size(), kMaxAnchors));
    }

    int primary = addColor(editor, plan.primary);
    int secondary = addColor(editor, plan.secondary);
    int accent = addColor(editor, plan.accent);
    int bg = addColor(editor, plan.background);
    if (primary < 0 || secondary < 0 || accent < 0 || bg < 0) {
        return Err("Could not allocate enough GD color channels.");
    }

    auto* created = CCArray::create();
    created->retain();

    GenerationStats stats;
    stats.anchors = static_cast<int>(anchors.size());

    const int panelEvery = std::max(1, static_cast<int>(std::round(1.f / std::max(0.05f, plan.panelDensity))));
    const int airEvery = std::max(4, static_cast<int>(std::round(5.f / std::max(0.05f, plan.airDecoDensity))));
    const int bgEvery = std::max(5, static_cast<int>(std::round(6.f / std::max(0.05f, plan.backgroundDensity))));

    size_t index = 0;
    for (auto* anchor : anchors) {
        auto p = anchor->getPosition();
        auto key = gridKey(p);
        float baseScale = std::max(0.2f, anchor->getScale());
        bool alternate = ((key.x + key.y) & 1) != 0;
        int mainColor = alternate ? secondary : primary;

        // 1) Inner panel: visually replaces the plain block while collision remains untouched underneath.
        if ((index % panelEvery) == 0) {
            int panelID = alternate ? 211 : 220; // small clean panel / color square family
            if (addDeco(editor, created, panelID, p, mainColor, ZLayer::T1,
                        baseScale * (0.72f + 0.12f * plan.variation),
                        anchor->getRotation(),
                        static_cast<GLubyte>(150 + 90 * plan.readability))) {
                ++stats.placed;
            }
        }

        // 2) Exposed-edge block design. ID 468 is the outline-top primitive; rotate it for each exposed side.
        auto exposed = [&](int dx, int dy) {
            return !occupied.contains(GridKey{key.x + dx, key.y + dy});
        };
        struct Edge { int dx, dy; float rot; CCPoint off; };
        Edge edges[] = {
            { 0,  1,   0.f, { 0.f,  15.f}},
            { 1,  0,  90.f, {15.f,   0.f}},
            { 0, -1, 180.f, { 0.f, -15.f}},
            {-1,  0, 270.f, {-15.f,  0.f}},
        };

        for (auto const& edge : edges) {
            if (!exposed(edge.dx, edge.dy)) continue;
            if (addDeco(editor, created, 468, {p.x + edge.off.x, p.y + edge.off.y}, accent, ZLayer::T2,
                        baseScale, edge.rot,
                        static_cast<GLubyte>(140 + 115 * plan.outlineStrength))) {
                ++stats.placed;
            }

            // Soft glow follows exposed edges, but stays restrained for readability.
            if (plan.glowStrength > 0.12f && ((index + edge.dx + edge.dy + 16) % 2 == 0)) {
                if (addDeco(editor, created, 503, {p.x + edge.off.x, p.y + edge.off.y}, mainColor, ZLayer::B1,
                            baseScale * 0.9f, edge.rot,
                            static_cast<GLubyte>(35 + 150 * plan.glowStrength))) {
                    ++stats.placed;
                }
            }
        }

        // 3) Sparse background motifs. These are no-touch and behind gameplay.
        if ((index % bgEvery) == 0) {
            float offsetY = 75.f + static_cast<float>((key.x % 5) * 18);
            float s = 0.75f + 1.1f * plan.variation;
            if (addDeco(editor, created, 496, {p.x, p.y + offsetY}, bg, ZLayer::B3,
                        s, static_cast<float>((key.x * 17) % 45),
                        static_cast<GLubyte>(35 + 80 * plan.backgroundDensity))) {
                ++stats.placed;
            }
        }

        // 4) Sparse air deco: circles/squares, intentionally low density.
        if ((index % airEvery) == 0) {
            int airID = alternate ? 497 : 495;
            float side = alternate ? 1.f : -1.f;
            if (addDeco(editor, created, airID,
                        {p.x + side * (55.f + (key.x % 3) * 20.f), p.y + 55.f + (key.y % 4) * 18.f},
                        alternate ? accent : secondary, ZLayer::B2,
                        0.22f + 0.28f * plan.variation,
                        static_cast<float>((key.x * 23) % 360),
                        static_cast<GLubyte>(45 + 100 * plan.airDecoDensity))) {
                ++stats.placed;
            }
        }

        ++index;
        if (created->count() > 30000) break;
    }

    if (created->count() == 0) {
        created->release();
        return Err("The decoration engine produced no objects.");
    }

    // Store this generation as one reversible version. Original gameplay was never mutated.
    History::get().push(created);
    created->release();
    return Ok(stats);
}

} // namespace decopilot
