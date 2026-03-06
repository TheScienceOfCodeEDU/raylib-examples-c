#pragma once
#ifndef UNITY_BUILD
#define IMPLEMENT_ALL   1
#include "main.h"
#endif


// > SUBMODULE: OVERLAY
// > INDEX
GUI_Overlay     GUI_MakeOverlay();
void            GUI_DrawOverlay();
bool            GUI_OverlayIsOpenBy(const char* text_id_owner);
bool            GUI_OverlayGetJustEnabled();
void            GUI_OverlayClose();
void            GUI_OverlayOpenFor(const char* id);
void            GUI_OverlaySetDrawCall(bool just_enabled, void (*draw_function)(void));
void            GUI_OverlaySetFinalShape(Rectangle shape);
void            GUI_CloseOverlayOnInteraction(bool force, Rectangle shape);

// > IMPLEMENTATION
#ifdef IMPLEMENT_ALL
GUI_Overlay GUI_MakeOverlay()
{
    GUI_Overlay overlay = {
        .id_ptr             = NULL,
        .window_target_id   = 0,
        .layout             = GUI_MakeLayoutTemp(),
        .just_enabled       = false,
        .final_shape        = (Rectangle){0,0,0,0},
        .function           = NULL,
    };
    return overlay;
}

void GUI_DrawOverlay()
{
    GUI_Overlay *overlay    = &GUI_CTX.temp->overlay;
    bool is_enabled         = overlay->function != NULL && overlay->id_ptr != NULL;

    if (is_enabled) {
        // Prepare state
        GUI_CTX.temp->layout                    = overlay->layout;
        GUI_CTX.temp->layout.force_overflow     = true;
        // No final_shape = CursorOverOverlay is FALSE then controls inside overlay->function();
        // can be interacted (See GUI_CheckCollisionCursorControl)
        overlay->final_shape                    = (Rectangle){0,0,0,0};
        // Draw call
        overlay->function();
        overlay->function = NULL;
    }
}

bool GUI_OverlayIsOpenBy(const char* text_id_owner)
{
    return GUI_CTX.temp->overlay.id_ptr == text_id_owner;
}

bool GUI_OverlayGetJustEnabled()
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
    GUI_CTX.temp->overlay.id_ptr = id;
}

void GUI_OverlaySetDrawCall(bool just_enabled, void (*draw_function)(void))
{
    GUI_CTX.temp->overlay.layout           = GUI_CTX.temp->layout;
    GUI_CTX.temp->overlay.window_target_id = GUI_CTX.temp->window_target_id;
    GUI_CTX.temp->overlay.just_enabled     = just_enabled;
    GUI_CTX.temp->overlay.function         = draw_function;
}

void GUI_OverlaySetFinalShape(Rectangle shape)
{
    Assert(GUI_CTX.temp->overlay.id_ptr != NULL);
    GUI_CTX.temp->overlay.final_shape = shape;
}

void GUI_CloseOverlayOnInteraction(bool force, Rectangle shape)
{
    bool interacted     = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool interactable   = GUI_OverlayGetJustEnabled() == false;
    bool is_active      = interacted && interactable;
    if (force || is_active) {
        GUI_OverlayClose();
    } else {
        Rectangle relative_shape = GUI_RelativePositionOnly(shape);
        GUI_OverlaySetFinalShape(relative_shape);
    #if DEV_DEBUG_GUI == 1
        DrawDebugRect(relative_shape, RED);
    #endif
    }
}
#endif
