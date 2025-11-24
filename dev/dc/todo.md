imKairos

- Scrolling.                                OK
    - Auto scroll (do not set pre-value)    OK
        - Render stores window size and     OK
        -  uses it for next render.         OK
- Resize

- Button menu       OK

- Close windows

- Review GUI_CTX    OK

- Textbox improvements
    - scroll contents       OK
    - copy/paste
    - jump between spaces

- Clean code    WIP
- Reemplazar ids por automaticos asi:
    - Global[OK]/Window[? si es necesario]:
        id == posicion de memoria                       OK
        esto se usa solo para saber si estoy focused    ^
        Limitación menor.

- Improve outside usage     WIP
- Add transparency          OK

- Determinar tamaño icons al 0.5 del original.          OK
- Organizar ajustes para temas                          OK
    - Remover padding y reemplazarlo por un delta       OK
    - Editor

- Tabs!
    - Inside window

- Cut controls
    - Test          (OK)
    - Do all rest    OK

- Change cursor when hover a textbox.   OK
    - Support multiple cursors.         OK
    - Change cursor/pointer.            OK

- Icons
    - windows       OK
    - switch

- Implement docking.
    - First by code. (Kinda?)
    - W/Interactions

- Implement Slider.
    - Play with hue-slider to test theme colors.

- Windows z-index
    - WindowStruct                                  (OK)
    - GUI_Window adds a Window GUI_RenderWindows    (OK)

- Interact conditions -> Functions to allow further customization.