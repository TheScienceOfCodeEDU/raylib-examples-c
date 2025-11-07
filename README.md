# Abstractica Game Toolkit - Overview

EN: [Installation guide](https://thescienceofcode.com/raylib-vscode-c-cpp-debug/).
ES: [Guía de instalacion](https://thescienceofcode.com/es/raylib-vscode-c-cpp-debug/)

## 🧱 Project Structure

**Folders use 3-letter prefixes (except root):**

```
bin     →  dbg-OS / rel-OS builds (linux/win)
lib     →  third-party libraries
root    →  main game directory (runtime path)
src     →  source code
run     →  build / execution scripts (TODO: migrate all build scripts here)
tmp     →  temporary build folder
```

Eventually, `root` will split into a `res` folder for assets requiring preprocessing during build.

---

## 🌿 Philosophy

**Abstractica Game Toolkit**

We are a group of old-school programmers writing a toolkit for making games.\
The code must be very clear and simple, prioritizing **readability** above everything else.

We are creating a toolkit that’s easy to use, like finding a 1989 floppy disk with a tool that lets you make games without being a professional.\
**It’s not an engine. It’s not complicated. It’s a toolkit.**

---

## 🎮 General Style

- **Language:** C99 (no C++ stuff)
- **Goal:** Readable code that tells a story.

---

## ✏️ Naming Rules (Simple & Friendly)

| Kind      | Example                                | Notes                      |
| --------- | -------------------------------------- | -------------------------- |
| Modules   | `Game_`, `GUI_`                        | Prefix with module name    |
| Structs   | `Game_Character`                       | PascalCase + module prefix |
| Functions | `Game_LoadLevel()`, `GUI_DrawWindow()` | Verb + clear noun          |
| Variables | `player_speed`, `window_shape`         | lower\_snake\_case         |
| Constants | `PLAYER_MAX_SPEED`, `GUI_MAX_WINDOWS`  | All caps                   |

Readable beats fancy. If someone can guess what it does from the name — perfect.

---

## 🧠 Example — Clear, Story-like Code

```c
bool Game_CheckRingCollision(Game_Character* character_1, Game_Character* character_2, float radius)
{
    // Extract data
    Vector2 center_1   = RectCenter(character_1->shape);
    Vector2 center_2   = RectCenter(character_2->shape);
    // Process
    float distance     = Vector2Distance(center_1, center_2);
    bool within_radius = distance < (radius * 2);
    return within_radius;
}
```

💡 **Why this works:**

- Each step has meaning: you can “read” what’s happening.
- Variable names express the logic — no need for extra comments.
- Easy to tweak or reuse later.

That’s the whole idea of the Abstractica style.

**Another example:**

```c
// Let's start with this
Vector2 GUI_MeasureAdjustedText_WORST(const char* text, EGUI_FontType font_type)
{
    return Vector2Add(
        (Vector2){
            MeasureTextEx(GUI_GetFont(font_type), text,
                GUI_GetFont(font_type).baseSize *
                GUI_GetSetup()->font_setups[font_type].font_scale *
                GUI_GetState()->scale,
                GUI_GetSetup()->font_setups[font_type].font_spacing).x +
            GUI_GetSetup()->font_setups[font_type].blink_delta.x *
                GUI_GetState()->scale *
                GUI_GetSetup()->font_setups[font_type].font_scale,

            MeasureTextEx(GUI_GetFont(font_type), text,
                GUI_GetFont(font_type).baseSize *
                GUI_GetSetup()->font_setups[font_type].font_scale *
                GUI_GetState()->scale,
                GUI_GetSetup()->font_setups[font_type].font_spacing).y +
            GUI_GetSetup()->font_setups[font_type].blink_delta.y *
                GUI_GetState()->scale *
                GUI_GetSetup()->font_setups[font_type].font_scale
        },
        Vector2Scale(
            GUI_GetSetup()->font_setups[font_type].font_delta,
            GUI_GetState()->scale
        )
    );
}

// Then make it more readable
Vector2 GUI_MeasureAdjustedText_READABLE(const char* text, EGUI_FontType font_type)
{
    // We are extracting data
    GUI_FontSetup* setup    = &GUI_GetSetup()->font_setups[font_type];
    GUI_State* state        = GUI_GetState();
    Font font               = GUI_GetFont(font_type);

    // But here looks clunky and hard to read
    Vector2 result = {
        MeasureTextEx(font, text, font.baseSize * setup->font_scale * state->scale, setup->font_spacing).x + setup->blink_delta.x * state->scale * setup->font_scale,
        MeasureTextEx(font, text, font.baseSize * setup->font_scale * state->scale, setup->font_spacing).y + setup->blink_delta.y * state->scale * setup->font_scale
    };

    return Vector2Add(result, Vector2Scale(setup->font_delta, state->scale));
}

// And polish it!
Vector2 GUI_MeasureAdjustedText(const char* text, EGUI_FontType font_type)
{
    // Extract data
    GUI_State *state        = GUI_CTX.state;
    GUI_FontSetup* setup    = &GUI_GetSetup()->font_setups[font_type];
    Font font               = GUI_GetFont(font_type);

    // Process it
    float font_scaled       = font.baseSize * setup->font_scale * state->scale;
    Vector2 text_measure    = MeasureTextEx(font, text, font_scaled, setup->font_spacing);

    // Results (or statements)
    Vector2 result = {
        text_measure.x + setup->blink_delta.x * state->scale * setup->font_scale,
        text_measure.y + setup->blink_delta.y * state->scale * setup->font_scale
    };

    // Adjust
    Vector2 delta_scaled = Vector2Scale(setup->font_delta, state->scale);
    return Vector2Add(result, delta_scaled);
}
```

---

## 🪄 Example — Creating a Custom Button

Use macros to make reusable control logic while keeping the code readable.

```c
bool GUI_IconButton(Texture2D* texture2d, Vector2 position, float height, Color tint)
{
    Rectangle shape = RectFromVector2(position, height, height);

    // Apply layout
    GUI_MACRO_CONTROL_LAYOUT(shape);

    // Control activated logic
    GUI_MACRO_CONTROL_ACTIVATED(shape);

    // Draw icon
    GUI_Icon(texture2d, position, height, tint);

    return is_active;
}
```

These macros automatically handle input, focus, and pointer logic, exposing a few useful statements (as shown in the next example: bool is_active = is_pointer_over && is_pointer_active).
This allows us to simply return that value, clearly indicating that the button was activated.

---

## 🧩 Reusable Macros — Extendable GUI Controls

### GUI\_MACRO\_CONTROL\_ACTIVATED

```c
#define GUI_MACRO_CONTROL_ACTIVATED(shape) \
    bool is_activable       = GUI_CTX.temp.current_action == EGUI_ActionNone;           \
    bool is_pointer_over    = GUI_CheckCollisionPointerControlCurrentWin(shape);        \
    bool is_pointer_active  = is_activable && IsMouseButtonReleased(MOUSE_BUTTON_LEFT); \
    bool is_active = is_pointer_over && is_pointer_active;     \
    if (is_pointer_over) GUI_CTX.temp.pointer_over_gui = true; \
```

### GUI\_MACRO\_CONTROL\_FOCUSED

```c
#define GUI_MACRO_CONTROL_FOCUSED(value, shape) \
    bool is_activable       = GUI_CTX.temp.current_action == EGUI_ActionNone;    \
    bool is_pointer_over    = GUI_CheckCollisionPointerControlCurrentWin(shape);  \
    bool is_pointer_active  = is_activable && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyEnterPressed()); \
    bool just_focused = is_pointer_over && is_pointer_active;  \
    if (just_focused) { GUI_CTX.temp.control_focus_ptr = value; } \
    bool is_focused = GUI_CTX.temp.control_focus_ptr == value; \
    if (is_pointer_over) GUI_CTX.temp.pointer_over_gui = true; \
```

These macros make it easy to create new GUI elements that share consistent input logic. You can build on them without rewriting boilerplate every time.

**We use macros** because they provide the same base code for your own controls **without requiring inheritance or complex abstractions** — you just use them directly. We designed them to be simple, readable, and extendable in a script-like fashion.

We chose macros because:

- They give you direct access to useful statements like **is_active** or **is_pointer_over**, which you can treat as events in your scripts.

- You’re encouraged to **read and understand them** — we show you the actual code. If you’re building something inside our framework, **you should know how it works**.

*Hide nothing. Customize **everything**.*

---

## 🔢 Example — Enums and Meaningful Order

```c
typedef enum {
    EGUI_Status_Default,
    EGUI_Status_Collide,
    EGUI_Status_Focused
} GUI_ElementStatus;

// EXAMPLE: Enum order matters
typedef enum {
    EGUI_Focus_Available,
    EGUI_Focus_CanOverride,
    EGUI_Focus_Granted
} EGUI_Focus;

// NOTE: define a function near the type instead of using it everywhere.
//       Now, we know that the order matters for this Enum.
bool FocusOverridable(EGUI_Focus focus)
{
    return focus <= EGUI_Focus_CanOverride;
}
```

Keep related enums and helper functions close to each other.
It tells the next developer how the logic flows and keeps meaning explicit.

---

## 🏗️ Make Initialization Style

```c
typedef struct {
    int current_character;
    int alive_characters;
    Game_Character characters[CHARACTERS];
    Camera2D camera2D;
} Game_State;

Game_State Game_MakeState(void)
{
    Game_State state = {
        .current_character = 0,
        .alive_characters  = 2,
        .characters = {
            (Game_Character){  0,  0, 10, 20, RED,    Vector2Zero(), 0 },
            (Game_Character){ 10, 30, 10, 20, BLUE,   Vector2Zero(), 0 },
            (Game_Character){ 50, 60, 10, 20, GREEN,  Vector2Zero(), 0 },
            (Game_Character){ 80, 60, 10, 20, ORANGE, Vector2Zero(), 0 },
        },
        .camera2D = {
            .offset   = (Vector2){ 0, 0 },
            .target   = (Vector2){ 0, 0 },
            .rotation = 0.0f,
            .zoom     = 1.0f
        }
    };

    return state;
}
```

This style shows everything clearly — easy to read, extend, and reason about.

---

## 🌍 Global Context Setup

```c
static struct {
    Game_State          *state;
    Game_WindowState    *win_state;
} GAME_CTX = { 0 };

void Game_SetContext(Game_State *state, Game_WindowState *win_state)
{
    GAME_CTX.state      = state;
    GAME_CTX.win_state  = win_state;
}
```

Having a `SetContext()` function allows each arena or system to manage its own game state independently.\
By combining **Make + SetContext**, components don’t need to know object lifetimes — an external manager handles that.
Perfect for save/load systems or isolated simulations.

---

## 🧩 Raylib Extensions (Rayext)

General-purpose helpers for Raylib integration: math, geometry, vector, and rect functions.

Example:

```c
Rectangle RectFromVector2(Vector2 position, float w, float h)
{
    Rectangle result = { position.x, position.y, w, h };
    return result;
}
```

These don’t follow the `Module_Function()` naming style because they’re global utilities.
Only subsystem-specific code (like GUI, Game, etc.) uses that prefix.

---

## 🧪 Experimental Code (lab / lab\_\*)

`lab.c` and `lab_{experiment}.c` are reserved for testing and prototyping new features.
They don’t follow full standards — they’re playgrounds for ideas.

---

## 🧱 Unity Build Integration

In `common.h`, add new components with include guards by module:

```c
#ifndef INCLUDES_BASE_H
 #define INCLUDES_BASE_H
 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <raylib.h>
 #include <rlgl.h>
 #include <raymath.h>
 #include "env.h"
 #include "rayext.h"

 #if defined(INCLUDE_GUI)
  #include "gui_setup.h"
  #include "gui_structs.h"
  #include "gui.h"
 #endif

 #if defined(INCLUDE_GAME)
  #include "game_structs.h"
  #include "game_gui.h"
  #include "game.h"
 #endif
#endif
```

This makes it easy to reuse and import full modules when needed.

**Example:** `game.h`

```c
#ifndef UNITY_BUILD
 #define UNITY_BUILD 0
 #define INCLUDE_GUI
 #include "common.h"
 #include "game_structs.h"
 #include "game_gui.h"
#endif
```

This imports GUI but not `GAME` itself (since it’s the current module). The main file can bring all:

```c
#define UNITY_BUILD 1
#define INCLUDE_GUI
#define INCLUDE_GAME
#include "common.h"
#include "lab.h" // Experimental modules added separately
```

---

**Abstractica Game Toolkit**
*Keep it simple, keep it kind.*
