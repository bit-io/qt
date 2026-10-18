# qt

Bindingi **Qt6 Widgets** dla języka **[H#](https://github.com/HackerOS-Linux-System/H-Sharp)**,
budowane przez **[Bit](https://github.com/HackerOS-Linux-System/bit)**.

Rdzeń jest napisany w **C++17** (klasy, RAII, tablica uchwytów, ograniczona kolejka
zdarzeń). H# łączy się z nim wyłącznie przez `extern static [c++, "qthsharp"]`
— biblioteka jest linkowana statycznie, `extern dynamic` nie występuje nigdzie
(Bit i tak go odrzuca).

## Układ

```
Bit.hk                 manifest (format Bit: [package] [layout] [edition] [lib] [build] ...)
src/
  qt.h#                moduł-parasol (mod qt ładuje wszystko)
  qt_ffi.h#            surowe deklaracje extern static [c++] + take()
  qt_types.h#          Widget, Layout, Menu, Action, Timer, Event, EventKind
  qt_app.h#            init / run / quit / shutdown, wersje, schowek
  qt_widgets.h#        kontener, label, button, checkbox, groupbox, progress, tabs, scroll
  qt_inputs.h#         lineedit, textedit, radio, slider, spinbox, combobox, lista
  qt_layout.h#         vbox / hbox / grid / form
  qt_window.h#         okno, status bar, menu, toolbar, akcje
  qt_timer.h#          QTimer -> zdarzenie TimerTick
  qt_dialogs.h#        komunikaty, input, wybór pliku / katalogu
  qt_events.h#         poll_event()
  qt_keys.h#           kody klawiszy i modyfikatory
  qt_gfx.h#            obrazy, ikony, Canvas (rysowanie)
  qt_input.h#          klawiatura / mysz / kółko / resize, skróty
  qt_table.h#          QTableWidget
  qt_tree.h#           QTreeWidget
  qt_extras.h#         data / czas / kalendarz, splitter, stos stron, dock
  qt_tray.h#           tray, powiadomienia, menu samodzielne
  qt_theme.h#          motywy dark / light / system
native/
  include/qthsharp.h   publiczne C ABI
  src/core/            HandleTable, EventQueue, AppHost, HWindow, InputFilter,
                       Canvas (Q_OBJECT + paintEvent), TreeItems (czyste C++)
  src/api/             punkty wejścia extern "C" pogrupowane tematycznie
  tests/native_smoke.cpp   test headless (QT_QPA_PLATFORM=offscreen)
  CMakeLists.txt, build.sh
examples/  (hello_window, counter_app, widgets_demo, paint_demo, data_demo)
tests/
```

## Dlaczego eksporty są `extern "C"`

Kompilator H# wiąże `extern` po zwykłej nazwie symbolu i nie ma managlera nazw
C++ dla wolnych funkcji. Dlatego cienka warstwa wejściowa jest `extern "C"`,
a cała logika to klasy C++ w `native/src/core`.

## Budowanie

Wymagane: Qt6 (Widgets), CMake, kompilator C++17.
`bit build` sam uruchamia `sh native/build.sh` (patrz `[build] native` w Bit.hk).
Ręcznie: `sh native/build.sh` (buduje `native/libqthsharp.a` i uruchamia test natywny).

## Użycie

```hsharp
mod qt

fn main() is
    qt_app::init()
    let w: qt_types::Widget = qt_window::new_window("Hej", 300, 120)
    qt_widgets::show(w)
    ;; ... pętla: qt_app::process_events() + qt_events::poll_event()
end
```

Zdarzenia sprawdzaj przez `if ev.kind == qt_types::EventKind::Clicked is ... end`
(parser H# nie przyjmuje w `match` wzorca `modul::Typ::Wariant`).

## Nowe w 0.3

- **Tabele i drzewa:** `qt_table` (komórki, nagłówki, sortowanie, edycja, kolory komórek)
  i `qt_tree` (elementy jako id, podrzewa, rozwijanie). Zapis komórek z kodu jest „cichy” —
  nie zalewa kolejki zdarzeń.
- **Rysowanie i obrazy:** `qt_gfx` — obrazy w `QLabel`, ikony przycisków/akcji/okna
  (`theme:NAZWA` = ikona z motywu pulpitu), `Canvas` z liniami, prostokątami, elipsami,
  tekstem i obrazami, zrzut widgetu do PNG (`grab_png`).
- **Zdarzenia wejściowe:** `qt_input::watch(widget, WATCH_ALL)` → `KeyPressed`, `MousePressed`,
  `MouseMoved`, `Wheel`, `Resized`; skróty `new_shortcut(widget, "Ctrl+S")`.
  `Event` ma teraz `pos_x`, `pos_y`, `mods`. Kody klawiszy w `qt_keys`.
- **Widgety i dialogi:** data / czas / kalendarz, splitter, stos stron, dock, wybór koloru
  i czcionki, okno postępu, tray z powiadomieniami, menu samodzielne (`qt_tray`).
- **Wygoda:** `qt_layout::row([a, b, c])`, `column`, `grid_of`, `form_of`, `labeled`,
  `panel_column([...])`; motywy `qt_theme::apply_dark()` / `apply_light()` / `apply_system()`
  (paleta Fusion dla całej aplikacji) oraz gotowe style sheety.
- **Fasada `qt::`:** stare API 0.1 (`qt::new_button`, `qt::Widget` …) działa dalej jako cienkie
  wrappery na końcu `qt.h#`. Aliasy typów (`pub type Widget = qt_types::Widget`) przechodzą
  parser H#, ale nie sprawdzałem ich kompilatorem — jeśli `h#` ich nie rozwiąże, używaj `qt_types::Widget`.

Uwagi: `Resized` Qt wysyła tylko widgetom już pokazanym. `match` z wzorcem
`modul::Typ::Wariant` nie przechodzi parsera H# — porównuj `ev.kind == qt_types::EventKind::X`.
Stałe z modułów (`qt_keys::KEY_ESCAPE`, `qt_input::WATCH_ALL`) przeszły parser, ale nie były
kompilowane.

## Zmiany względem 0.1

- Jeden `qt::` rozbity na moduły (`qt_widgets::new_button` itd.); stare nazwy `qt::...` działają przez fasadę.
- `extern dynamic` → `extern static [c++]`.
- Uchwyty to bezpieczne id (nie wskaźniki); użycie uchwytu po zniszczeniu widgetu jest no-opem.
- Stringi zwracane z C++ są kopiowane i zwalniane (`qt_ffi::take`), brak wycieków.
- `EventKind::Timer` nie istnieje; zdarzenie timera to `TimerTick`.
- Dodane widgety i funkcje: textedit, radio, spinbox, double spinbox, combobox, lista, progress,
  groupbox, zakładki, scroll, separator, grid/form, menu, toolbar, status bar, timer, schowek,
  dialogi input/plików, `init_offscreen()`, `shutdown()`.
- Poprawki: `destroy()` naprawdę zwalnia obiekty w pętli z `process_events()`;
  `QApplication` nie jest niszczony w statycznym destruktorze (crash).

## Status testów

Warstwa C++ jest skompilowana i przetestowana na Qt 6.4.2 (offscreen, także ASan/UBSan), w tym
rysowanie (sprawdzane na pikselach zrzutu), zdarzenia wejściowe, tabele, drzewa i uchwyty po usunięciu.
Niesprawdzone w tym teście: dialogi blokujące (kolor, czcionka, plik) i faktyczny tray.
Pliki H# przechodzą parser H#; nie były kompilowane kompilatorem `h#`.

## Licencja

MIT
