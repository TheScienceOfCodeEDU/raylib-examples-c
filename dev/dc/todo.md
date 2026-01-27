imKairos

- Textbox improvements
    - int y float
    - scroll contents           OK
        - BUG: Poner textbox en 2da columna del layout -> scrolling no se ve bien.
    - copy/paste
    - jump between spaces
    

- Tabs!
    - Inside window

- Resize
    - Works?            OK
    - Add other points

- Icons
    - windows       OK
    - switch

- Implement Slider.
    - Play with hue-slider to test theme colors.

- Implement docking.
    - First by code. (Kinda?)
    - W/Interactions

- Organizar ajustes para temas                          OK
    - Remover padding y reemplazarlo por un delta       OK
    - Editor


DONE

- Reemplazar ids por automaticos asi:
    - Global[OK]/Window[? si es necesario]:
        id == posicion de memoria                       OK
        esto se usa solo para saber si estoy focused    ^
        Limitación menor.
- Add transparency          OK

- Determinar tamaño icons al 0.5 del original.          OK

- Cut controls
    - Test          (OK)
    - Do all rest    OK

- Change cursor when hover a textbox.   OK
    - Support multiple cursors.         OK
    - Change cursor/pointer.            OK


- Windows z-index
    - WindowStruct                                  (OK)
    - GUI_Window adds a Window GUI_RenderWindows    (OK)

- Interact conditions -> Functions to allow further customization.  OK

- Scrolling.                                OK
    - Auto scroll (do not set pre-value)    OK
        - Render stores window size and     OK
        -  uses it for next render.         OK

- Button menu           OK
    - Draw at the end   OK
    - Overflow          OK
    - Icon sizes        OK
    - Generalize
        - Postrender    OK
        - Macros        OK

- Add statuses  OK
- Fix IsPointerOverGui()    OK
- Asserts to be project-wise (rayext)   OK
- Close windows     OK
- Review GUI_CTX    OK