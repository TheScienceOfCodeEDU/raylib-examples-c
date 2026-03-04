#pragma once
#ifndef UNITY_BUILD
#define UNITY_BUILD 0
#include "../common.h"
#endif

#include "types.h"

// > FUNCTIONS
//   INDEX

// > CONSTRUCTORS

GUI_State           GUI_MakeStateDefault(Vector2 screen_max);

GUI_Overlay         GUI_MakeOverlay();
GUI_Temp            GUI_MakeTempDefault();

void                GUI_SetFontType(EGUI_Font font);
float               GUI_CalcDefaultHeightScaled(EGUI_Font font);

void                GUI_AfterWindowContents();

// > POINTER
bool                GUI_IsCursorOverGui();
bool                GUI_IsCurrentWindowTarget(int window_id);
bool                GUI_IsCursorOverOverlay();
GUI_FontSetup*      GUI_GetFontSetup(EGUI_Font font);
Font                GUI_GetFontAsset(EGUI_Font font);
GUI_CursorSetup*    GUI_GetCursorSetup();


static inline Rectangle GUI_Relative(Rectangle shape);
static inline EGUI_Font GUI_GetFont();



// > FRAME
//   PIPELINE

void GUI_BeginDraw(EGUI_Cursor cursor_style);
void GUI_EndDraw();


#include "_layout.h"
#include "_setup.h"

#include "_window.h"
#include "_overlay.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "_controls.h"


// > FUNCTIONS
//   IMPLEMENTATION

#ifdef IMPLEMENT_ALL

// > CONSTRUCTORS
//   DEFAULTS

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

void GUI_SetFontType(EGUI_Font font)
{
    GUI_CTX.temp->layout.current_font = font;
}

float GUI_CalcDefaultHeightScaled(EGUI_Font font)
{
    GUI_Setup* setup = GUI_CTX.setup;
    GUI_State* state = GUI_CTX.state;
    return setup->fonts[font].default_height * state->scale;
}

void GUI_AfterWindowContents()
{
    GUI_DrawOverlay();
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

void GUI_EndDraw()
{
    GUI_CTX.temp->mouse_last = GUI_CTX.temp->mouse_current;
    GUI_DrawOverlay();

    Assert(GUI_CTX.temp->status == EGUI_Status_Drawing);
    GUI_CTX.temp->status = EGUI_Status_Ready;
}




#endif
