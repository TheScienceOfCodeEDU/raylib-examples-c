#pragma once
#ifndef NON_EDITOR_BUILD
#define IMPLEMENT_ALL   1
#include "../common.h"
#endif


// > SUBMODULE: OVERLAY
// > INDEX
GUI_Overlay     GUI_MakeOverlay();
void            GUI_DrawOverlay();
bool            GUI_OverlayOpenedBy(const char* text_id_owner);
bool            GUI_OverlayWasJustEnabled();
void            GUI_OverlayClose();
void            GUI_OverlayOpenFor(const char* id);
void            GUI_OverlaySetDrawCall(bool just_enabled, void (*draw_function)(void));
void            GUI_OverlaySetFinalShape(Rectangle shape);
bool            GUI_OverlayIsDrawing();
GUI_ThemeColors GUI_BeginOverlay();
void            GUI_EndOverlay(Rectangle final_shape);

// > IMPLEMENTATION
#ifdef IMPLEMENT_ALL
GUI_Overlay GUI_MakeOverlay()
{
    GUI_Overlay overlay = {
        .id_ptr             = NULL,
        .grid               = GUI_MakeGrid(),
        .just_enabled       = false,
        .final_shape        = (Rectangle){0,0,0,0},
        .is_drawing         = false,
        .function           = NULL,
    };
    return overlay;
}

void GUI_DrawOverlay()
{
    GUI_Overlay *overlay    = &GUI_CTX.temp->overlay;
    bool is_enabled         = overlay->function != NULL && overlay->id_ptr != NULL;

    if (is_enabled) {
        // Draw call
        overlay->function();
        overlay->function = NULL;
    }
}

bool GUI_OverlayOpenedBy(const char* text_id_owner)
{
    return GUI_CTX.temp->overlay.id_ptr == text_id_owner;
}

bool GUI_OverlayWasJustEnabled()
{
    return GUI_CTX.temp->overlay.just_enabled;
}

void GUI_OverlayClose()
{
    GUI_CTX.temp->overlay = GUI_MakeOverlay();
}

void GUI_OverlayOpenFor(const char* id)
{
    Assert(id != NULL);
    GUI_CTX.temp->overlay.id_ptr            = id;
}

void GUI_OverlaySetDrawCall(bool just_enabled, void (*draw_function)(void))
{
    GUI_CTX.temp->overlay.grid             = GUI_CTX.temp->grid;
    GUI_CTX.temp->overlay.just_enabled     = just_enabled;
    GUI_CTX.temp->overlay.function         = draw_function;
}

void GUI_OverlaySetFinalShape(Rectangle shape)
{
    Assert(GUI_CTX.temp->overlay.id_ptr != NULL);
    GUI_CTX.temp->overlay.final_shape = shape;
}

bool GUI_OverlayIsDrawing()
{
    return GUI_CTX.temp->overlay.is_drawing;
}

GUI_ThemeColors GUI_BeginOverlay()
{
    GUI_Overlay *overlay    = &GUI_CTX.temp->overlay;
    // Prepare state
    GUI_CTX.temp->grid                      = overlay->grid;
    GUI_CTX.temp->grid.force_overflow       = true;
    // Begin is drawing
    GUI_CTX.temp->overlay.is_drawing        = true;

    // Apply ThemeColors
    int window_id               = GUI_CTX.temp->window_current_id;
    bool has_window_target      = window_id != GUI_NO_WIN;
    if (has_window_target) {
        GUI_Window* window      = GUI_GetWindow(window_id);
        bool valid_window       = window->id != GUI_NO_WIN;
        if (valid_window) {
            return window->colors;
        }
    }
    return GUI_GetSetup()->theme.gray;
}

void GUI_EndOverlay(Rectangle final_shape)
{
    bool interacted     = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool interactable   = GUI_OverlayWasJustEnabled() == false;
    if (interacted && interactable) {
        GUI_OverlayClose();
    } else {
        Rectangle relative_shape = GUI_GridRelativePositionOnly(final_shape);
        GUI_OverlaySetFinalShape(relative_shape);
    #if DEV_DEBUG_GUI == 1
        DrawDebugRect(relative_shape, RED);
    #endif
    }
    // End drawing
    GUI_CTX.temp->overlay.is_drawing = false;
}
#endif
