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
bool            GUI_OverlayGetJustInteracted();
void            GUI_OverlayClose();
void            GUI_OverlayOpenFor(const char* id);
void            GUI_OverlaySetDrawCall(bool just_interacted, void (*draw_function)(void));
void            GUI_OverlaySetShapeDrawed(Rectangle shape_drawed);
void            GUI_CloseOverlayOnInteraction(bool force, Rectangle shape);

// > IMPLEMENTATION
#ifdef IMPLEMENT_ALL
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

void GUI_DrawOverlay()
{
    GUI_Overlay *overlay    = &GUI_CTX.temp->overlay_draw;
    bool is_enabled         = overlay->function != NULL && overlay->id_ptr != NULL;

    if (is_enabled) {
        GUI_CTX.temp->layout                    = overlay->layout;
        GUI_CTX.temp->layout.force_overflow     = true;

        overlay->function();
        overlay->function = NULL;
    }
}

bool GUI_OverlayIsOpenBy(const char* text_id_owner)
{
    return GUI_CTX.temp->overlay_draw.id_ptr == text_id_owner;
}

bool GUI_OverlayGetJustInteracted()
{
    return GUI_CTX.temp->overlay_draw.just_interacted;
}

void GUI_OverlayClose()
{
    GUI_CTX.temp->overlay_draw = GUI_MakeOverlay();
}

void GUI_OverlayOpenFor(const char* id)
{
    Assert(id != NULL);
    GUI_CTX.temp->overlay_draw.id_ptr = id;
}

void GUI_OverlaySetDrawCall(
    bool just_interacted,
    void (*draw_function)(void))
{
    GUI_CTX.temp->overlay_draw.layout           = GUI_CTX.temp->layout;
    GUI_CTX.temp->overlay_draw.window_target_id = GUI_CTX.temp->window_target_id;
    GUI_CTX.temp->overlay_draw.just_interacted  = just_interacted;
    GUI_CTX.temp->overlay_draw.function         = draw_function;
}

void GUI_OverlaySetShapeDrawed(Rectangle shape_drawed)
{
    Assert(GUI_CTX.temp->overlay_draw.id_ptr != NULL);
    GUI_CTX.temp->overlay_draw.shape_drawed = shape_drawed;
}

void GUI_CloseOverlayOnInteraction(bool force, Rectangle shape)
{
    bool interacted     = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool interactable   = GUI_OverlayGetJustInteracted() == false;
    bool is_active      = interacted && interactable;
    if (force || is_active) {
        GUI_OverlayClose();
    } else {
        Rectangle relative_shape = GUI_RelativePositionOnly(shape);
        GUI_OverlaySetShapeDrawed(relative_shape);
    #if DEV_DEBUG_GUI == 1
        DrawDebugRect(relative_shape, RED);
    #endif
    }
}
#endif
