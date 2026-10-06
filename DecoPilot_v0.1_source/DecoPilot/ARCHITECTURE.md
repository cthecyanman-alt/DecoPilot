# DecoPilot architecture

## Priority order

1. Preserve gameplay perfectly
2. Decoration quality
3. Completely free local operation
4. Reference/style imitation
5. Easy installation
6. Speed

## Why this architecture

Direct LLM -> raw Geometry Dash objects is unreliable: malformed output, random IDs, destroyed layouts, conversational responses, and weak spatial understanding. DecoPilot instead separates **creative decisions** from **object placement**.

### Pass A — Level analysis
Read existing GD objects and compute structure/bounds/type statistics. Future versions add connected-component platform segmentation.

### Pass B — Style planning
Local Ollama model sees the user's prompt + level summary and returns a tiny validated JSON plan: palette, glow, outline, panel/background/air-deco density, variation and readability.

### Pass C — Deterministic placement
C++ code uses a curated object vocabulary and adjacency rules. It may add visuals on top of gameplay but never changes original collision objects.

### Pass D — History
Every generated object is tracked as a generation. Undo deletes only those generated objects.

### Planned Reference Lab
The user asked for *heavy* reference research. The robust design is not generic web scraping. It is a dedicated GD reference index:

- user enters one or more level IDs/names
- fetch actual level data
- sample many related/high-quality references
- extract block/object families, colors, layer distribution, glow density, air-deco density, motif repetition and section transitions
- cache summaries locally
- pass only compact reference fingerprints to the local model

This makes "search a ton" useful instead of simply stuffing screenshots/text into an LLM.
