#pragma once
#include <Geode/Geode.hpp>
#include <vector>

namespace decopilot {

class History {
    std::vector<cocos2d::CCArray*> m_generations;
public:
    static History& get();
    ~History();

    void push(cocos2d::CCArray* objects);
    bool undo(LevelEditorLayer* editor);
    size_t size() const { return m_generations.size(); }
    void clear();
};

} // namespace decopilot
