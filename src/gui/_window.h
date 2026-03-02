#pragma once
#ifndef UNITY_BUILD
#define IMPLEMENT_ALL
#include "main.h"
#endif


// > SUBMODULE: WINDOW
// > INDEX
void GUI_DrawOverlay();
// > WINDOW
void            GUI_WindowButtonPanel(GUI_Window* window, EGUI_Font font);
void            GUI_WindowEndingPanel(GUI_Window* window, EGUI_Font font);
void            GUI_DrawWindow(GUI_Window* window,  EGUI_ControlStatus status, EGUI_Font font);
void            GUI_UpdateAndDrawWindow(GUI_Window *window, Rectangle limits);
void            GUI_CleanAndPrepareZIndex();
void            GUI_UpdateAndDrawWindows(Rectangle limits);
Rectangle       GUI_BeginWindowContents(GUI_Window* window, EGUI_Font font);
void            GUI_EndWindowContents(GUI_Window* window);

// > INDEX
GUI_WindowTemp  GUI_MakeWindowTemp();
GUI_Window      GUI_MakeEmptyWindow(void);

// > IMPLEMENTATION
#ifdef IMPLEMENT_ALL
GUI_WindowTemp GUI_MakeWindowTemp()
{
        GUI_WindowTemp result = {
            .current_action     = EGUI_WindowAction_None,
            .window_target_id   = 0
        };
    return result;
}

// > WINDOW

void GUI_WindowButtonPanel(GUI_Window* window, EGUI_Font font)
{
    GUI_Icons *icons            = GUI_GetIcons();
    GUI_FontSetup *font_setup   = GUI_GetFontSetup(font);
    GUI_ThemeColors colors      = window->colors;

    float border                = font_setup->border;
    float scale                 = GUI_CTX.state->scale;
    float icon_sm_width         = GUI_GetIconSmallWidth();

    Rectangle shape_panel       = GUI_GetWindowPanel(window->shape);
    Vector2 position_button     = (Vector2) { shape_panel.x + border * scale, shape_panel.y };

    DrawRectangleRec(shape_panel, colors.bg_color_0);
    if (GUI_IconButton(&icons->CloseSmall, position_button, icon_sm_width, WHITE)) {
        GUI_RemoveWindow(window->id);
    }
    GUI_Icon(&icons->MinimizeSmall, AddVector2(position_button, 0, icon_sm_width + border * scale), icon_sm_width, WHITE);
}

void GUI_WindowEndingPanel(GUI_Window* window, EGUI_Font font)
{
    GUI_FontSetup *font_setup   = GUI_GetFontSetup(font);
    GUI_ThemeColors colors      = window->colors;

    float border        = font_setup->border;
    float scale         = GUI_CTX.state->scale;

    Rectangle shape_bottom   = GUI_GetWindowBottom(window->shape);
    DrawRectangleRec((Rectangle) {
        shape_bottom.x + border * scale,
        shape_bottom.y,
        shape_bottom.width - border * scale * 2,
        border * scale
    }, colors.bg_color_2);
    DrawRectangleRec((Rectangle) {
        shape_bottom.x + border * scale,
        shape_bottom.y + border * scale,
        shape_bottom.width - border * scale * 2,
        border * scale
    }, colors.bg_color_0);
}

void GUI_DrawWindow(GUI_Window* window,  EGUI_ControlStatus status, EGUI_Font font)
{
    GUI_State       *state          = GUI_CTX.state;
    GUI_FontSetup   *font_setup     = GUI_GetFontSetup(font);

    Rectangle        shape          = window->shape;
    Rectangle        shape_title    = GUI_GetWindowTitle(window->shape);
    //Rectangle        shape_panel    = GUI_WindowPanel(window->shape);
    //Rectangle        shape_bottom   = GUI_WindowBottom(window->shape);
    GUI_ThemeColors  colors         = window->colors;
    GUI_Theme        *theme         = &GUI_CTX.setup->theme;

    float border        = font_setup->border;
    float scale         = state->scale;
    float color_change  = theme->color_change;
    float bg_alpha      = theme->bg_alpha;

    // Background
    DrawRectangleRec((Rectangle){shape.x + border * scale, shape.y + border * scale, shape.width - border * scale, shape.height - 2 * border * scale}, ColorAlpha(colors.bg_color_1, bg_alpha));
    GUI_DrawBorders(shape, colors.bg_color_0, colors.bg_color_2, border * scale, true);

    if (status == EGUI_ControlStatus_Default) {
        DrawRectangleRec(shape_title, ColorAlpha(colors.bg_color_2, bg_alpha));
        GUI_DrawBorders(shape_title, colors.bg_color_2, colors.bg_color_0, border * scale, false);
    } if (status == EGUI_ControlStatus_Focused) {
        DrawRectangleRec(shape_title, ColorAlpha(ColorBrightness(colors.bg_color_3, -color_change), bg_alpha));
        GUI_DrawBorders(shape_title, colors.bg_color_2, colors.bg_color_0, border * scale, false);
        GUI_DrawBorders(shape, colors.bg_color_0, ColorBrightness(colors.bg_color_3,-color_change), border * scale, true);
    } if (status == EGUI_ControlStatus_Collide) {
        DrawRectangleRec(shape_title, ColorAlpha(ColorBrightness(colors.bg_color_3, color_change), bg_alpha));
        GUI_DrawBorders(shape_title, colors.bg_color_2, colors.bg_color_0, border * scale, false);
        GUI_DrawBorders(shape, colors.bg_color_0, ColorBrightness(colors.bg_color_3, color_change), border * scale, true);
    }

    bool reserve_icon_space = window->icon != NULL || (status == EGUI_ControlStatus_Focused && window->focused_face);
    float icon_w = reserve_icon_space ? GUI_GetIconWidth() : 0;

    GUI_BeginInnerControlScissor(shape_title, border, scale);
        GUI_DrawAdjustedTextEx(window->title,
            (Vector2) { shape_title.x + icon_w + (border) * scale, shape_title.y + (border) * scale },
            colors.tx_color_0, scale, EGUI_Font_GUI);
    EndScissorMode();

    Vector2 icon_position = { shape_title.x + border * scale, shape_title.y + border * scale };
    if (window->icon != NULL && icon_w > 0) {
        GUI_Icon(window->icon, icon_position, icon_w, WHITE);
    }
    if (status == EGUI_ControlStatus_Focused && icon_w > 0 && window->focused_face) {
        GUI_Face((Vector2) { shape_title.x + border * scale, shape_title.y + border * scale }, icon_w);
    }

    // Vertical scroll
    // Scrollbar
    Rectangle workspace = GUI_GetWindowWorkspace(window);
    if (workspace.height < window->content_height) {
        float ratio = workspace.height / window->content_height;
        float bar_h = ratio * workspace.height;
        float bar_y = workspace.y + (window->scroll_offset / window->content_height) * workspace.height;

        DrawRectangleRec((Rectangle){
            workspace.x + workspace.width,
            workspace.y,
            border * scale * 3,
            workspace.height
        }, colors.bg_color_2);
        DrawRectangleRec((Rectangle){
            workspace.x + workspace.width + border * scale,
            bar_y,
            border * scale,
            bar_h
        }, colors.tx_color_0);
    }

    #if DEV_DEBUG_GUI == 1
    DrawDebugRect(shape_bottom, ColorAlpha(YELLOW, 0.85f));
    DrawDebugRect(shape_title, ColorAlpha(WHITE, 0.75f));
    DrawDebugRect(shape_panel, ColorAlpha(RED, 0.85f));
    DrawDebugRect(GUI_WindowWorkspace(window), ColorAlpha(GREEN, 0.25f));
    #endif
}

void GUI_UpdateAndDrawWindow(GUI_Window *window, Rectangle limits)
{
    EGUI_Font font          = EGUI_Font_GUI;
    Rectangle shape_title   = GUI_GetWindowTitle(window->shape);
    Rectangle shape_panel   = GUI_GetWindowPanel(window->shape);
    Rectangle shape_bottom  = GUI_GetWindowBottom(window->shape);
    Rectangle workspace     = GUI_GetWindowWorkspace(window);
    Vector2 mouse           = GUI_CTX.temp->mouse_current;
    GUI_WindowTemp *temp    = &GUI_CTX.temp->window;

    // Conditions
    bool is_window_target       = GUI_IsCurrentWindowTarget(window->id);
    bool is_cursor_overlay     = GUI_IsCursorOverOverlay();
    bool is_focusable           = is_window_target && temp->current_action == EGUI_WindowAction_None && is_cursor_overlay == false;
    bool is_cursor_over        = CheckCollisionPointRec(mouse, window->shape);
    bool is_cursor_over_panel  = is_focusable && CheckCollisionPointRec(mouse, shape_panel);
    bool is_cursor_over_title  = is_focusable && CheckCollisionPointRec(mouse, shape_title) && !is_cursor_over_panel;
    bool is_cursor_over_bottom = is_focusable && CheckCollisionPointRec(mouse, shape_bottom);
    bool just_interacted        = is_cursor_over && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool is_z_priority          = GUI_CTX.state->z_index[0] == window->id;

    // Update cursor_over_gui
    if (is_cursor_over) GUI_CTX.temp->cursor_over_gui = true;

    // Focus?
    if (is_focusable && just_interacted && is_z_priority) {
        temp->current_action =  is_cursor_over_title    ? EGUI_WindowAction_Moving :
                                is_cursor_over_bottom   ? EGUI_WindowAction_Resizing
                                                        : EGUI_WindowAction_None;
    }

    if (is_cursor_over_bottom || temp->current_action == EGUI_WindowAction_Resizing) {
        GUI_CTX.temp->cursor = EGUI_Cursor_Resize;
    }

    // Active
    if (is_z_priority) {
        bool is_mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        if (is_mouse_down == false) {
            temp->current_action = EGUI_WindowAction_None;
        } else {
            // Movement
            if (temp->current_action == EGUI_WindowAction_Moving) {
                Vector2 mouse_current_valid     = LimitVector2Rect(GUI_CTX.temp->mouse_current, limits);
                Vector2 mouse_last_valid        = LimitVector2Rect(GUI_CTX.temp->mouse_last, limits);
                Vector2 displacement            = Vector2Subtract(mouse_current_valid, mouse_last_valid);

                window->shape.x += displacement.x;
                window->shape.y += displacement.y;
            }
            // Resizing
            if (temp->current_action == EGUI_WindowAction_Resizing) {
                Vector2 mouse_valid     = LimitVector2Rect(GUI_CTX.temp->mouse_current, limits);
                window->shape.width     = FloatMax(mouse_valid.x - window->shape.x, GUI_MIN_WIN_SIZE);
                window->shape.height    = FloatMax(mouse_valid.y - window->shape.y, GUI_MIN_WIN_SIZE);
            }
            // Handled by GUI, as move/resize can make that is_cursor_over is false during frames
            GUI_CTX.temp->cursor_over_gui = temp->current_action != EGUI_WindowAction_None;
        }
    }

    // Limit
    window->shape   = LimitRect(window->shape, limits);
    // shape_title  = GUI_WindowTitle(window->shape);

    // Vertical scroll
    bool horizontal_scroll  = workspace.height < window->content_height;
    if (horizontal_scroll) {
        if (is_cursor_over) {
            float wheel = GetMouseWheelMove();
            if (wheel != 0.0f) window->scroll_offset -= wheel * GUI_SCROLL_SPEED;
        }
        window->scroll_offset = Clamp(window->scroll_offset, 0, window->content_height - workspace.height);
    } else {
        window->scroll_offset = 0;
    }

    // Draw
    EGUI_ControlStatus status =
        is_z_priority           ? EGUI_ControlStatus_Focused :
        is_cursor_over_title   ? EGUI_ControlStatus_Collide :
                                  EGUI_ControlStatus_Default;
    GUI_DrawWindow(window, status, font);
    // Generate button panel
    GUI_WindowButtonPanel(window, font);
    GUI_WindowEndingPanel(window, font);
}


void GUI_CleanAndPrepareZIndex()
{
    GUI_State* state = GUI_CTX.state;

    // Remove unused window ids
    for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) {
        int id = state->z_index[j];
        if (id == 0)
            continue;

        bool found_id = false;
        for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
            GUI_Window* window = &state->window_s[i];
            if (window->id == id) {
                found_id = true;
                break;
            }
        }

        if (found_id == false) {
            state->z_index[j] = 0; // Clean
        }
    }

    // Grant that all window_s are in the z-index
    for (int i = 0; i < GUI_MAX_OPEN_WINS; i++) {
        GUI_Window* window = &state->window_s[i];
        if (window->id == 0)
            continue;

        int first_zero = -1;
        bool found_id = false;
        for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) {
            if (state->z_index[j] == window->id){
                found_id = true;
                break;
            }
            if (state->z_index[j] == 0 && first_zero < 0) {
                first_zero = j;
            }
        }
        if (found_id == false) {
            state->z_index[first_zero] = window->id;
        }
    }
}

void GUI_UpdateAndDrawWindows(Rectangle limits)
{
    GUI_State *state        = GUI_CTX.state;
    GUI_WindowTemp *temp    = &GUI_CTX.temp->window;

    GUI_CleanAndPrepareZIndex();

    // This variable allows to set force the z_index during the current frame
    bool is_cursor_overlay = GUI_IsCursorOverOverlay();
    bool force_z_index      = state->force_z_index > 0;
    bool interacting        = !force_z_index
                                && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
                                && is_cursor_overlay == false;

    if (interacting || force_z_index) {
        // Find ID
        int interacted_id       = state->force_z_index;
        bool forcing_z_index    = state->force_z_index > 0;
        // Restore state values -> z-index is being updated
        state->force_z_index = 0;

        int current_zindex  = -1;
        for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) {
            int id = state->z_index[j];
            if (id == 0) continue;

            GUI_Window* window = GUI_GetWindow(id);
            if (window == NULL) continue;

            bool find_window    = forcing_z_index && interacted_id == window->id;
            bool check_window   = !forcing_z_index && CheckCollisionPointRec(GUI_CTX.temp->mouse_current, window->shape);
            if (find_window || check_window) {
                interacted_id = window->id;
                current_zindex = j;
                break;
            }
        }

        if (interacted_id > 0 && current_zindex > 0) {
            // Move all elements to the right starting at current_zindex
            for (int j = current_zindex; j > 0; --j) {
                state->z_index[j] = state->z_index[j - 1];
            }

            // Add this one
            state->z_index[0] = interacted_id;
        }
    }

    // UPDATE WINDOW TARGET ID
    // Check collisions to determine current window_target_id (not only z-index priority but actual collision for this frame
    // you can be pointing to a 2nd window with a lower z-index priority.
    temp->window_target_id = 0;
    // If overlay is displayed then force window_target_id
    // This allows clicking on the overlay when its in front of other window(s).
    if (GUI_CTX.temp->overlay_draw.window_target_id != 0) {
        temp->window_target_id = GUI_CTX.temp->overlay_draw.window_target_id;
    }
    // Normal windows
    if (temp->window_target_id  == 0) {
        for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) {
            int id = state->z_index[j];
            if (id == 0) continue;

            GUI_Window  *window         = GUI_GetWindow(id);
            bool        is_cursor_over = CheckCollisionPointRec(GUI_CTX.temp->mouse_current, window->shape);
            if (is_cursor_over) {
                temp->window_target_id = window->id;
                break;
            }
        }
    }

    // Process
    for (int j = GUI_MAX_OPEN_WINS - 1; j >= 0 ; --j) {
        int id = state->z_index[j];
        if (id == 0) continue;

        for (int i = 0; i < GUI_MAX_OPEN_WINS; i++) {
            GUI_Window* window = &state->window_s[i];
            if (window->id != id) continue;

            GUI_UpdateAndDrawWindow(window, limits);
            window->contents(window);
        }
    }
}

Rectangle GUI_BeginWindowContents(GUI_Window* window, EGUI_Font font)
{
    // Grant min dimensions
    window->shape.width     = FloatMax(GUI_MIN_WIN_SIZE, window->shape.width);
    window->shape.height    = FloatMax(GUI_MIN_WIN_SIZE, window->shape.height);

    // Data
    Rectangle window_workspace = GUI_GetWindowWorkspace(window);
    // Begin window stuff
    GUI_LayoutReset(window_workspace);
    GUI_SetFontType(font);

    // Vertical scroll
    GUI_CTX.temp->layout.current_window_idx         = window->id;
    GUI_CTX.temp->layout.current_window_workspace   = window_workspace;
    GUI_CTX.temp->layout.current_scroll             = -window->scroll_offset;

    // Vertical scroll
    rlPushMatrix();
    rlTranslatef(0, -window->scroll_offset, 0);

    return window_workspace;
}

void GUI_EndWindowContents(GUI_Window* window)
{
    // End window stuff
    GUI_LayoutAutoJump();

    // Vertical scroll
    // Stored layout height
    window->content_height                   = GUI_CTX.temp->layout.used_height;
    // Reset temp values
    GUI_CTX.temp->layout                     = GUI_MakeLayoutTemp();

    // Finish draw instructions
    GUI_DrawOverlay();
    rlPopMatrix();
}

#endif
