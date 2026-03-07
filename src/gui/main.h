#pragma once
#ifndef UNITY_BUILD
#define UNITY_BUILD 0
#include "../common.h"
#endif

#include "types.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "_setup.h"

// > FUNCTIONS
//   INDEX

// > CONSTRUCTORS

GUI_State           GUI_MakeStateDefault(Vector2 screen_max);
GUI_Temp            GUI_MakeTempDefault();

EGUI_Font           GUI_GetFont();
void                GUI_SetFontType(EGUI_Font font);
float               GUI_CalcDefaultHeightScaled(EGUI_Font font);
void                GUI_ProcessWindow(GUI_Window* window, Rectangle limits);
void                GUI_AfterWindowContents();
bool                GUI_IsCursorOverGui();
bool                GUI_IsCurrentWindowTarget(int window_id);
bool                GUI_IsCursorOverOverlay();
Rectangle           GUI_GridRelative(Rectangle shape);
void                GUI_BeginDraw(EGUI_Cursor cursor_style);
void                GUI_EndDraw();


#include "_grid.h"
#include "_window.h"
#include "_overlay.h"
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
        .scale          = 1.0f,
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

GUI_Temp GUI_MakeTempDefault()
{
    GUI_Temp temp = {
        .status                     = EGUI_Status_Off,
        .control_focus_ptr          = NULL,
        .current_font               = EGUI_Font_Default,
        .grid                       = GUI_MakeGrid(),
        .overlay                    = GUI_MakeOverlay(),
        .cursor                     = EGUI_Cursor_Default,
        .cursor_last                = (Vector2){ 0.0f, 0.0f },
        .cursor_current             = (Vector2){ 0.0f, 0.0f },
        .cursor_over_gui            = false,
        .cursor_trail               = {{0}},
        .window_current_id          = GUI_NO_WIN,
        .window_target_id           = GUI_NO_WIN,
        .window_current_action      = EGUI_WindowAction_None,
    };
    return temp;
}

EGUI_Font GUI_GetFont()
{
    EGUI_Font font = GUI_CTX.temp->current_font;
    return font;
}

void GUI_SetFontType(EGUI_Font font)
{
    GUI_CTX.temp->current_font = font;
}

float GUI_CalcDefaultHeightScaled(EGUI_Font font)
{
    GUI_Setup* setup = GUI_CTX.setup;
    GUI_State* state = GUI_CTX.state;
    return setup->fonts[font].default_height * state->scale;
}

void GUI_ProcessWindow(GUI_Window* window, Rectangle limits)
{
    Assert(window->id > 0);
    Assert(window->contents != NULL);

    GUI_UpdateAndDrawWindow(window, limits);
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
    int target_id   = GUI_CTX.temp->window_target_id;
    bool no_target  = target_id == GUI_NO_WIN;
    bool is_target  = target_id == window_id;
    return no_target || is_target;
}

bool GUI_IsCursorOverOverlay()
{
    Vector2 cursor      = GUI_CTX.temp->cursor_current;
    Rectangle shape     = GUI_CTX.temp->overlay.final_shape;
    bool is_cursor_over = CheckCollisionPointRec(cursor, shape);
    return is_cursor_over;
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
    GUI_CTX.temp->cursor_current = LimitVector2Rect(GetMousePosition(), mouse_limits);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        GUI_CTX.temp->control_focus_ptr = NULL;
    }

    GUI_CTX.temp->grid = GUI_MakeGrid();
    GUI_CTX.temp->current_font = EGUI_Font_Default;
}

void GUI_EndDraw()
{
    GUI_CTX.temp->cursor_last = GUI_CTX.temp->cursor_current;
    GUI_DrawOverlay();

    Assert(GUI_CTX.temp->status == EGUI_Status_Drawing);
    GUI_CTX.temp->status = EGUI_Status_Ready;
}




#endif
