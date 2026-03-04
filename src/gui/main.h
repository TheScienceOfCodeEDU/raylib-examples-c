#pragma once
#ifndef UNITY_BUILD
#define UNITY_BUILD 0
#include "../common.h"
#endif

#include "types.h"
#include "_layout.h"
#include "_setup.h"

// > FUNCTIONS
//   INDEX

// > CONSTRUCTORS

GUI_State           GUI_MakeStateDefault(Vector2 screen_max);

GUI_Overlay         GUI_MakeOverlay();
GUI_Temp            GUI_MakeTempDefault();

void GUI_DrawOverlay();

// > POINTER
bool                GUI_IsCursorOverGui();
bool                GUI_IsCurrentWindowTarget(int window_id);
bool                GUI_IsCursorOverOverlay();
GUI_FontSetup*      GUI_GetFontSetup(EGUI_Font font);
Font                GUI_GetFontAsset(EGUI_Font font);
GUI_CursorSetup*    GUI_GetCursorSetup();


static inline Rectangle GUI_Relative(Rectangle shape);
static inline EGUI_Font GUI_GetFont();

// > OVERLAY
//   API

static inline bool GUI_OverlayIsOpenBy(const char* text_id_owner);
static inline GUI_Overlay* GUI_OverlayGetDraw();
static inline bool GUI_OverlayGetJustInteracted();
static inline void GUI_OverlayClose();
static inline void GUI_OverlayOpenFor(const char* id);
static inline void GUI_OverlaySetDrawCall(
    bool just_interacted,
    void (*draw_function)(void));
static inline void GUI_OverlaySetShapeDrawed(Rectangle shape_drawed);
void GUI_CloseOverlayOnInteraction(bool force, Rectangle shape);

// > FRAME
//   PIPELINE

void GUI_BeginDraw(EGUI_Cursor cursor_style);

void GUI_EndDraw();

#include "_window.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "_controls.h"


// > FUNCTIONS
//   IMPLEMENTATION

#ifdef IMPLEMENT_ALL

// > CONSTRUCTORS
//   DEFAULTS

GUI_Window GUI_MakeEmptyWindow(void)
{
    GUI_Window window = {
        .id             = 0,
        .shape          = (Rectangle){ 0, 0, 0, 0 },
        .colors         = {{ 0 }},
        .title          = NULL,
        .icon           = NULL,
        .scroll_offset  = 0.0f,
        .content_height = 0.0f,
        .focused_face   = true,
        .contents       = NULL,
    };
    return window;
}

GUI_State GUI_MakeStateDefault(Vector2 screen_max)
{
    GUI_State state = {
        .buffer         = LoadRenderTexture((int)screen_max.x, (int)screen_max.y),
        .scale          = 2.0f,
        .force_z_index  = 0
    };

    // IMPORTANT: keep GUI buffer pixel-perfect
    SetTextureFilter(state.buffer.texture, TEXTURE_FILTER_POINT);

    for (int i = 0; i < GUI_MAX_OPEN_WINS; i++) {
        state.window_s[i] = GUI_MakeEmptyWindow();
    }

    memset(state.z_index, 0, sizeof(state.z_index));
    return state;
}



GUI_Overlay GUI_MakeOverlay()
{
    GUI_Overlay overlay = {
        .id_ptr             = NULL,
        .window_target_id   = 0,
        .layout             = GUI_MakeLayoutTemp(),
        .just_interacted    = false,
        .shape_drawed       = (Rectangle){0,0,0,0},
        .function           = NULL,
    };
    return overlay;
}

GUI_Temp GUI_MakeTempDefault()
{
    GUI_Temp temp = {
        .status             = EGUI_Status_Off,

        .cursor             = EGUI_Cursor_Default,
        .mouse_last         = (Vector2){ 0.0f, 0.0f },
        .mouse_current      = (Vector2){ 0.0f, 0.0f },
        .cursor_over_gui    = false,
        .cursor_trail       = {{0}},

        .overlay_draw       = GUI_MakeOverlay(),
        .window             = GUI_MakeWindowTemp(),
        .layout             = GUI_MakeLayoutTemp()

    };
    return temp;
}

// > CONTEXT
//   API

void GUI_SetContext(GUI_State* state, GUI_Setup* setup, GUI_Temp* temp)
{
    // Update statuses
    // Outgoing temp
    if (GUI_CTX.temp != NULL) {
        GUI_CTX.temp->status = EGUI_Status_Off;
    }
    // Incoming
    temp->status = EGUI_Status_Ready;

    GUI_CTX.setup = setup;
    GUI_CTX.state = state;
    GUI_CTX.temp  = temp;
}

// To be used out-side this module
GUI_State* GUI_GetState()
{
    return GUI_CTX.state;
}

// To be used out-side this module
GUI_Setup* GUI_GetSetup()
{
    return GUI_CTX.setup;
}


// > POINTER
//   STATE

// GUI_IsCursorOverGui() is meant to be called after EndDraw
// If you need it internally, it means that you're creating and internal component so you could use GUI_CTX.temp->cursor_over_gui
bool GUI_IsCursorOverGui()
{
    Assert(GUI_CTX.temp->status == EGUI_Status_Ready);
    return GUI_CTX.temp->cursor_over_gui;
}

// Returns true if window id is the interactable target (cursor is over and z-index is the lowest possible)
bool GUI_IsCurrentWindowTarget(int window_id)
{
    GUI_WindowTemp temp    = GUI_CTX.temp->window;
    return temp.window_target_id == 0 || temp.window_target_id == window_id;
}

bool GUI_IsCursorOverOverlay()
{
    bool is_cursor_over = CheckCollisionPointRec(GUI_CTX.temp->mouse_current, GUI_CTX.temp->overlay_draw.shape_drawed);
    return is_cursor_over;
}

GUI_FontSetup* GUI_GetFontSetup(EGUI_Font font)
{
    Assert(font >= 0 && font < EGUI_Font_Count);
    return &GUI_CTX.setup->fonts[font];
}

Font GUI_GetFontAsset(EGUI_Font font)
{
    GUI_Setup *setup = GUI_CTX.setup;
    if (setup->fonts[font].use_custom)
        return setup->fonts[font].custom;
    else
        return GetFontDefault();
}

GUI_CursorSetup* GUI_GetCursorSetup()
{
    EGUI_Cursor cursor = GUI_CTX.temp->cursor;
    return &GUI_CTX.setup->cursors[cursor];
}



static inline EGUI_Font GUI_GetFont()
{
    EGUI_Font font = GUI_CTX.temp->layout.current_font;
    return font;
}


// > OVERLAY
//   API
//   STABILITY : █████████░  90%

static inline bool GUI_OverlayIsOpenBy(const char* text_id_owner)
{
    return GUI_CTX.temp->overlay_draw.id_ptr == text_id_owner;
}

static inline GUI_Overlay* GUI_OverlayGetDraw()
{
    return &GUI_CTX.temp->overlay_draw;
}

static inline bool GUI_OverlayGetJustInteracted()
{
    return GUI_CTX.temp->overlay_draw.just_interacted;
}

static inline void GUI_OverlayClose()
{
    GUI_CTX.temp->overlay_draw = GUI_MakeOverlay();
}

static inline void GUI_OverlayOpenFor(const char* id)
{
    Assert(id != NULL);
    GUI_CTX.temp->overlay_draw.id_ptr = id;
}

static inline void GUI_OverlaySetDrawCall(
    bool just_interacted,
    void (*draw_function)(void))
{
    GUI_CTX.temp->overlay_draw.layout           = GUI_CTX.temp->layout;
    GUI_CTX.temp->overlay_draw.window_target_id = GUI_CTX.temp->window.window_target_id;
    GUI_CTX.temp->overlay_draw.just_interacted  = just_interacted;
    GUI_CTX.temp->overlay_draw.function         = draw_function;
}

static inline void GUI_OverlaySetShapeDrawed(Rectangle shape_drawed)
{
    Assert(GUI_CTX.temp->overlay_draw.id_ptr != NULL);
    GUI_CTX.temp->overlay_draw.shape_drawed = shape_drawed;
}

void GUI_CloseOverlayOnInteraction(bool force, Rectangle shape)
{
    bool interacted     = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool interactable   = GUI_OverlayGetJustInteracted() == false;

    if (force || (interacted && interactable)) {
        GUI_ForceZindex(GUI_CTX.temp->overlay_draw.window_target_id);
        GUI_OverlayClose();
    } else {
        Rectangle relative_shape = GUI_RelativePositionOnly(shape);
        GUI_OverlaySetShapeDrawed(relative_shape);
        #if DEV_DEBUG_GUI == 1
        DrawDebugRect(relative_shape, RED);
        #endif
    }
}

// > FRAME
//   PIPELINE

void GUI_BeginDraw(EGUI_Cursor cursor_style)
{
    Assert(GUI_CTX.temp->status == EGUI_Status_Ready);
    GUI_CTX.temp->status = EGUI_Status_Drawing;

    GUI_CTX.temp->cursor            = cursor_style;
    GUI_CTX.temp->cursor_over_gui   = false;

    Rectangle mouse_limits = (Rectangle) {
        0,
        0,
        (float) GetScreenWidth(),
        (float) GetScreenHeight()
    };
    GUI_CTX.temp->mouse_current = LimitVector2Rect(GetMousePosition(), mouse_limits);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        GUI_CTX.temp->control_focus_ptr = NULL;
    }

    GUI_CTX.temp->layout                       = GUI_MakeLayoutTemp();
}

void GUI_DrawOverlay()
{
    GUI_Overlay *overlay    = &GUI_CTX.temp->overlay_draw;
    bool is_enabled             = overlay->function != NULL && overlay->id_ptr != NULL;

    if (is_enabled) {
        GUI_CTX.temp->layout                    = overlay->layout;
        GUI_CTX.temp->layout.force_overflow     = true;

        overlay->function();
        overlay->function = NULL;
    }
}

void GUI_EndDraw()
{
    GUI_CTX.temp->mouse_last = GUI_CTX.temp->mouse_current;
    GUI_DrawOverlay();

    Assert(GUI_CTX.temp->status == EGUI_Status_Drawing);
    GUI_CTX.temp->status = EGUI_Status_Ready;
}




#endif
