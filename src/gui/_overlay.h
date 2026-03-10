#pragma once
#include "_grid.h"
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
GUI_ThemeColors GUI_OverlayColors();
void            GUI_OverlayClose();
void            GUI_OverlayOpenFor(const char* id);
void            GUI_OverlaySetDrawCall(bool just_enabled, GUI_ThemeColors colors, void (*draw_function)(void));
void            GUI_OverlaySetFinalShape(Rectangle shape);
bool            GUI_OverlayIsDrawing();
void            GUI_BeginOverlay();
void            GUI_EndOverlay(Rectangle final_shape);

// > IMPLEMENTATION
#ifdef IMPLEMENT_ALL
GUI_Overlay GUI_MakeOverlay()
{
    GUI_Overlay overlay = {
        .id_ptr             = NULL,
        .grid               = GUI_MakeGrid(),
        .colors             = (GUI_ThemeColors) { 0 },
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

        #if DEV_DEBUG_GUI_OVERLAY == 1
        DrawDebugRect(overlay->final_shape, RED);
        #endif
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

GUI_ThemeColors GUI_OverlayColors()
{
    return GUI_CTX.temp->overlay.colors;
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

void GUI_OverlaySetDrawCall(bool just_enabled, GUI_ThemeColors colors, void (*draw_function)(void))
{
    GUI_CTX.temp->overlay.grid             = GUI_CTX.temp->grid;
    GUI_CTX.temp->overlay.just_enabled     = just_enabled;
    GUI_CTX.temp->overlay.colors           = colors;
    GUI_CTX.temp->overlay.function         = draw_function;
}

void GUI_OverlaySetFinalShape(Rectangle shape)
{
    Assert(GUI_CTX.temp->overlay.id_ptr != NULL);
    Rectangle final_shape = (Rectangle) {
        .x      = shape.x,
        .y      = shape.y,// + GUI_CTX.temp->grid.current_scroll,
        .width  = shape.width,
        .height = shape.height
    };

    GUI_CTX.temp->overlay.grid.current_scroll = -GUI_CTX.temp->grid.current_scroll;
    GUI_CTX.temp->overlay.final_shape = final_shape;
}

bool GUI_OverlayIsDrawing()
{
    return GUI_CTX.temp->overlay.is_drawing;
}

void GUI_BeginOverlay()
{
    GUI_Overlay *overlay    = &GUI_CTX.temp->overlay;
    // Prepare state
    GUI_CTX.temp->grid                      = overlay->grid;
    GUI_CTX.temp->grid.force_overflow       = true;
    // Begin is drawing
    GUI_CTX.temp->overlay.is_drawing        = true;
}

void GUI_EndOverlay(Rectangle final_shape)
{
    bool interacted     = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool interactable   = GUI_OverlayWasJustEnabled() == false;
    if (interacted && interactable) {
        GUI_OverlayClose();
    } else {
        GUI_OverlaySetFinalShape(final_shape);
    }
    // End drawing
    GUI_CTX.temp->overlay.is_drawing = false;
}
#endif
