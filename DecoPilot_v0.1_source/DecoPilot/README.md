# DecoPilot v0.1.0 — source prototype

**Goal:** a Geometry Dash / Geode AI decorator that is *decoration-only*. It never asks an LLM to invent raw GD objects. Instead:

1. DecoPilot reads the current editor's real object graph.
2. A **local, free Ollama model** converts your art direction into a small JSON `StylePlan`.
3. A deterministic C++ decoration engine places only curated, no-touch decorative objects.
4. Original gameplay objects are never moved, deleted, resized, or edited.
5. Each DecoPilot generation is stored as its own in-session undo version.

This architecture is intentionally different from EditorAI. The LLM is used for **taste/style planning**, not for fragile object syntax.

## Target

- Windows only
- Geometry Dash Steam **2.2081**
- Geode **5.10.1** target (GitHub Actions is configured to build with the latest SDK)
- Button location: editor pause menu (ESC) -> **AI Deco**
- Local model default: `qwen2.5vl:7b`

## What v0.1 already implements

- Editor pause-menu `AI Deco` button
- Free local Ollama connection (`127.0.0.1:11434`)
- Prompt + model field
- Modern / Glow / Minimal / Tech presets
- Actual level analysis (object counts, bounds, common IDs)
- Strict JSON-only AI style planning
- Custom palette -> real GD color channels
- Additive block panels, exposed-edge outlines, controlled glow, sparse backgrounds and air deco
- Original gameplay preservation by design (`m_isNoTouch` on generated objects; originals are never mutated)
- One-click `Undo AI` generation history for the current editor session
- Object-count safety cap

## Important v0.1 limitation

This is the **first source prototype**, not a finished production release. I could not compile the `.geode` binary in the ChatGPT container because the Geode SDK / Windows compiler toolchain is not installed here. The source is structured for a normal Geode Windows build.

Also, the requested "search a TON of GD references" system is **not wired into v0.1 yet**. The next major piece should be a Reference Lab that downloads/analyzes many reference levels and builds a reusable style library. I intentionally did not fake that feature with a brittle web scraper.

## Free AI setup

Install Ollama, then run:

```bat
ollama pull qwen2.5vl:7b
```

For a lighter fallback:

```bat
ollama pull gemma3:4b
```

Or double-click `setup_local_ai.bat`.

Your RTX 3070 is the intended class of GPU for the default 7B model.

### Why DecoPilot cannot simply use your ChatGPT app conversation

A Geode mod cannot directly piggyback on your logged-in ChatGPT conversation/subscription as an unofficial free API. That would require an authenticated supported API/integration. DecoPilot therefore uses Ollama locally so the core workflow is genuinely free and has no paid API key.

## Build

Prerequisites:

- Current Geode SDK
- CMake
- Visual Studio C++ build tools
- `GEODE_SDK` environment variable pointing to your SDK

Typical build from the project folder:

```bat
cmake -B build -A x64
cmake --build build --config Release
```

Geode's CMake tooling packages the resulting mod as a `.geode` file.

## Next build priorities

1. **Reference Lab:** enter level IDs/names; download many references, analyze block families, palettes, density, layers, glow usage and recurring motifs.
2. **Vision pass:** capture editor/playtest frames and let the local vision model critique readability/composition before the deterministic polish pass.
3. **Better geometry segmentation:** identify connected platform silhouettes, slopes, corners and structure boundaries rather than styling individual 30x30 cells.
4. **More curated style primitives:** modern, glow, tech, minimal, nature, hell, space, custom libraries.
5. **Persistent version history** across editor restarts, not just the current session.
6. **Regenerate / variation controls** and per-pass toggles.

## Safety invariant

The core rule is non-negotiable: **DecoPilot does not edit original gameplay objects.** Any visual replacement is placed over/around the original collision skeleton as no-touch decoration.

## Easiest way to get a compiled `.geode`

The project includes `.github/workflows/build.yml`. Put the source in a GitHub repository and push it; GitHub Actions will run the official `geode-sdk/build-geode-mod` Windows builder and upload a `DecoPilot-Windows` artifact containing the compiled `.geode`. This avoids setting up Visual Studio/Geode locally.
