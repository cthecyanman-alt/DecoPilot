#include "History.hpp"

using namespace geode::prelude;

namespace decopilot {

History& History::get() {
    static History instance;
    return instance;
}

History::~History() {
    clear();
}

void History::push(CCArray* objects) {
    if (!objects || objects->count() == 0) return;
    objects->retain();
    m_generations.push_back(objects);
}

bool History::undo(LevelEditorLayer* editor) {
    if (!editor || !editor->m_editorUI || m_generations.empty()) return false;
    auto* arr = m_generations.back();
    m_generations.pop_back();

    // Delete in reverse order. We only ever delete objects DecoPilot itself added.
    for (int i = static_cast<int>(arr->count()) - 1; i >= 0; --i) {
        auto* obj = typeinfo_cast<GameObject*>(arr->objectAtIndex(i));
        if (!obj) continue;
        if (obj->getParent()) {
            editor->m_editorUI->deleteObject(obj, true);
        }
    }
    arr->release();
    return true;
}

void History::clear() {
    for (auto* arr : m_generations) {
        if (arr) arr->release();
    }
    m_generations.clear();
}

} // namespace decopilot
