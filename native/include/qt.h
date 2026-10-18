#ifndef QTHSHARP_H
#define QTHSHARP_H

/*
 * qthsharp — flat C ABI over Qt6 Widgets, implemented in C++17.
 *
 * H# declares these functions in `extern static [c++, "qthsharp"]` blocks
 * (src/qt_ffi.h#). The H# compiler resolves an extern by its plain symbol
 * name and has no Itanium name mangler for free functions, so the exported
 * layer has to be unmangled (`extern "C"`). All the real work lives in the
 * C++ classes under native/src/core (HandleTable, EventQueue, AppHost,
 * HWindow); the functions below are only thin entry points into them.
 *
 * Conventions
 *   - Handles are opaque, never-reused int64 ids (0 = invalid). A handle
 *     whose object was deleted resolves to nothing: calls become no-ops and
 *     getters return 0 / "".
 *   - Strings in are UTF-8 `const char*` (NULL = ""). Strings out are
 *     malloc'ed and must be released with qth_free_string().
 *   - Booleans are int8_t (0 / 1).
 *   - Everything must be called from the thread that called qth_app_init().
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QTH_VERSION_MAJOR 0
#define QTH_VERSION_MINOR 3
#define QTH_VERSION_PATCH 0

/* ── Event kinds (qth_poll_event) ────────────────────────────────────── */
#define QTH_EVENT_NONE               0
#define QTH_EVENT_CLICKED            1   /* QPushButton::clicked                       */
#define QTH_EVENT_TEXT_CHANGED       2   /* QLineEdit / QPlainTextEdit text changed    */
#define QTH_EVENT_RETURN_PRESSED     3   /* QLineEdit::returnPressed                   */
#define QTH_EVENT_TOGGLED            4   /* QCheckBox / QRadioButton / QGroupBox       */
#define QTH_EVENT_WINDOW_CLOSED      5   /* close requested on a QMainWindow           */
#define QTH_EVENT_VALUE_CHANGED      6   /* QSlider / QSpinBox / QDoubleSpinBox        */
#define QTH_EVENT_INDEX_CHANGED      7   /* QComboBox current index changed            */
#define QTH_EVENT_TAB_CHANGED        8   /* QTabWidget current tab changed             */
#define QTH_EVENT_SELECTION_CHANGED  9   /* QListWidget current row changed            */
#define QTH_EVENT_ITEM_ACTIVATED    10   /* QListWidget item double-clicked / Enter    */
#define QTH_EVENT_ACTION_TRIGGERED  11   /* QAction (menu / toolbar) triggered         */
#define QTH_EVENT_TIMER             12   /* QTimer timeout                             */
/* Input (only after qth_widget_watch_input; x/y/mods via qth_event_x/y/mods) */
#define QTH_EVENT_KEY_PRESSED       13   /* value = Qt key code, text = typed text     */
#define QTH_EVENT_KEY_RELEASED      14
#define QTH_EVENT_MOUSE_PRESSED     15   /* value = button (1 left, 2 right, 3 middle), x/y */
#define QTH_EVENT_MOUSE_RELEASED    16
#define QTH_EVENT_MOUSE_MOVED       17   /* value = Qt buttons bitmask, x/y            */
#define QTH_EVENT_WHEEL             18   /* value = vertical angle delta (120 = 1 notch) */
#define QTH_EVENT_RESIZED           19   /* x = new width, y = new height              */
#define QTH_EVENT_SHORTCUT          20   /* QShortcut activated (handle = shortcut)    */
/* Tables and trees */
#define QTH_EVENT_TABLE_CELL_CHANGED   21   /* current cell moved: x = row, y = column */
#define QTH_EVENT_TABLE_CELL_EDITED    22   /* user edited a cell: x, y, text          */
#define QTH_EVENT_TABLE_CELL_ACTIVATED 23   /* double click / Enter: x, y              */
#define QTH_EVENT_TREE_SELECTION_CHANGED 24 /* value = item id (0 = none), text        */
#define QTH_EVENT_TREE_ITEM_ACTIVATED  25   /* value = item id, text                   */
/* Date / time / tray */
#define QTH_EVENT_DATE_CHANGED      26   /* value = yyyymmdd, text = ISO date          */
#define QTH_EVENT_TIME_CHANGED      27   /* value = seconds since midnight, text = HH:mm:ss */
#define QTH_EVENT_TRAY_ACTIVATED    28   /* value = QSystemTrayIcon::ActivationReason */

/* Watch masks for qth_widget_watch_input */
#define QTH_WATCH_KEYS     1
#define QTH_WATCH_MOUSE    2   /* button press / release */
#define QTH_WATCH_MOVE     4   /* mouse move (enables mouse tracking) */
#define QTH_WATCH_WHEEL    8
#define QTH_WATCH_RESIZE  16
#define QTH_WATCH_ALL     31

/* Dock areas for qth_window_add_dock */
#define QTH_DOCK_LEFT   0
#define QTH_DOCK_RIGHT  1
#define QTH_DOCK_TOP    2
#define QTH_DOCK_BOTTOM 3

/* Text alignment values for qth_label_set_alignment */
#define QTH_ALIGN_LEFT    0
#define QTH_ALIGN_CENTER  1
#define QTH_ALIGN_RIGHT   2

/* ── Library info ────────────────────────────────────────────────────── */
const char* qth_version(void);      /* "0.2.0"          — free with qth_free_string */
const char* qth_qt_version(void);   /* e.g. "6.4.2"     — free with qth_free_string */

/* ── Application lifecycle ───────────────────────────────────────────── */
int64_t qth_app_init(void);                       /* 1 = created now, 0 = already existed */
int64_t qth_app_init_platform(const char* qpa);   /* same, with QT_QPA_PLATFORM (e.g. "offscreen") */
int32_t qth_app_exec(void);                       /* blocking Qt loop; returns exit code   */
void    qth_app_quit(void);
void    qth_app_shutdown(void);                   /* destroys QApplication + all windows; optional, call last */
void    qth_app_process_events(void);
int8_t  qth_app_is_running(void);
void    qth_app_set_style(const char* style_name);
void    qth_app_set_app_name(const char* name);
void    qth_app_set_style_sheet(const char* css); /* application-wide Qt CSS */
void    qth_app_set_quit_on_last_window_closed(int8_t on);

/* ── Handles ─────────────────────────────────────────────────────────── */
int8_t  qth_handle_is_valid(int64_t h);
int64_t qth_handle_count(void);                   /* live registered objects (diagnostics) */
void    qth_object_destroy(int64_t h);            /* deleteLater() on any object: timer, action, widget … */

/* ── Window / widget creation (return a handle) ──────────────────────── */
int64_t qth_window_new(const char* title, int32_t width, int32_t height);  /* QMainWindow */
int64_t qth_widget_new(void);                                              /* plain QWidget */
int64_t qth_button_new(const char* text);                                  /* QPushButton */
int64_t qth_label_new(const char* text);                                   /* QLabel */
int64_t qth_lineedit_new(const char* placeholder);                         /* QLineEdit */
int64_t qth_textedit_new(const char* placeholder);                         /* QPlainTextEdit */
int64_t qth_checkbox_new(const char* text);                                /* QCheckBox */
int64_t qth_radio_new(const char* text);                                   /* QRadioButton */
int64_t qth_slider_new(int32_t min, int32_t max, int32_t initial);         /* horizontal QSlider */
int64_t qth_spinbox_new(int32_t min, int32_t max, int32_t initial);        /* QSpinBox */
int64_t qth_dspinbox_new(double min, double max, double initial, int32_t decimals); /* QDoubleSpinBox */
int64_t qth_combobox_new(void);                                            /* QComboBox */
int64_t qth_progress_new(int32_t min, int32_t max, int32_t initial);       /* QProgressBar */
int64_t qth_groupbox_new(const char* title);                               /* checkable-less QGroupBox */
int64_t qth_tabs_new(void);                                                /* QTabWidget */
int64_t qth_list_new(void);                                                /* QListWidget */
int64_t qth_scroll_new(void);                                              /* QScrollArea */
int64_t qth_separator_new(int8_t vertical);                                /* QFrame line */

/* ── Layouts (return a handle) ───────────────────────────────────────── */
int64_t qth_vbox_new(void);    /* QVBoxLayout */
int64_t qth_hbox_new(void);    /* QHBoxLayout */
int64_t qth_grid_new(void);    /* QGridLayout */
int64_t qth_form_new(void);    /* QFormLayout */
void    qth_layout_add_widget(int64_t layout, int64_t widget);
void    qth_layout_add_layout(int64_t layout, int64_t child_layout);
void    qth_layout_add_spacing(int64_t layout, int32_t px);        /* box layouts */
void    qth_layout_add_stretch(int64_t layout, int32_t stretch);   /* box layouts */
void    qth_layout_set_margins(int64_t layout, int32_t l, int32_t t, int32_t r, int32_t b);
void    qth_layout_set_spacing(int64_t layout, int32_t px);
void    qth_grid_add_widget(int64_t layout, int64_t widget, int32_t row, int32_t col,
                            int32_t row_span, int32_t col_span);
void    qth_form_add_row(int64_t layout, const char* label, int64_t widget);
void    qth_widget_set_layout(int64_t widget, int64_t layout);

/* ── Generic QWidget operations ──────────────────────────────────────── */
void    qth_widget_show(int64_t h);
void    qth_widget_hide(int64_t h);
void    qth_widget_close(int64_t h);
void    qth_widget_resize(int64_t h, int32_t w, int32_t hgt);
void    qth_widget_move(int64_t h, int32_t x, int32_t y);
void    qth_widget_set_enabled(int64_t h, int8_t enabled);
void    qth_widget_set_visible(int64_t h, int8_t visible);
void    qth_widget_set_style_sheet(int64_t h, const char* css);
void    qth_widget_set_fixed_size(int64_t h, int32_t w, int32_t hgt);
void    qth_widget_set_minimum_size(int64_t h, int32_t w, int32_t hgt);
void    qth_widget_set_maximum_size(int64_t h, int32_t w, int32_t hgt);
void    qth_widget_set_tooltip(int64_t h, const char* text);
void    qth_widget_set_object_name(int64_t h, const char* name);
void    qth_widget_set_font(int64_t h, const char* family, int32_t point_size, int8_t bold, int8_t italic);
void    qth_widget_set_focus(int64_t h);
int8_t  qth_widget_is_visible(int64_t h);
int8_t  qth_widget_is_enabled(int64_t h);
int32_t qth_widget_width(int64_t h);
int32_t qth_widget_height(int64_t h);
void    qth_widget_destroy(int64_t h);                   /* deleteLater() */

/* ── QMainWindow ─────────────────────────────────────────────────────── */
void    qth_window_set_central_widget(int64_t window, int64_t widget);
void    qth_window_set_title(int64_t h, const char* title);
void    qth_window_set_auto_close(int64_t window, int8_t on);   /* accept close after the event */
void    qth_window_set_status(int64_t window, const char* text, int32_t timeout_ms);
int64_t qth_window_add_menu(int64_t window, const char* title);               /* → menu handle   */
int64_t qth_window_add_toolbar_action(int64_t window, const char* text);      /* → action handle */

/* ── Menus & actions ─────────────────────────────────────────────────── */
int64_t qth_menu_add_action(int64_t menu, const char* text, const char* shortcut); /* → action */
void    qth_menu_add_separator(int64_t menu);
int64_t qth_menu_add_submenu(int64_t menu, const char* title);                /* → menu handle   */
void    qth_action_set_enabled(int64_t action, int8_t enabled);
void    qth_action_set_checkable(int64_t action, int8_t checkable);
int8_t  qth_action_is_checked(int64_t action);
void    qth_action_set_checked(int64_t action, int8_t checked);
void    qth_action_set_text(int64_t action, const char* text);

/* ── QLabel ──────────────────────────────────────────────────────────── */
void        qth_label_set_text(int64_t h, const char* text);
const char* qth_label_get_text(int64_t h);
void        qth_label_set_word_wrap(int64_t h, int8_t wrap);
void        qth_label_set_alignment(int64_t h, int32_t alignment);

/* ── QPushButton ─────────────────────────────────────────────────────── */
void        qth_button_set_text(int64_t h, const char* text);
const char* qth_button_get_text(int64_t h);

/* ── QLineEdit ───────────────────────────────────────────────────────── */
const char* qth_lineedit_get_text(int64_t h);
void        qth_lineedit_set_text(int64_t h, const char* text);
void        qth_lineedit_set_placeholder(int64_t h, const char* text);
void        qth_lineedit_set_read_only(int64_t h, int8_t read_only);
void        qth_lineedit_set_echo_password(int64_t h, int8_t is_password);
void        qth_lineedit_set_max_length(int64_t h, int32_t n);
void        qth_lineedit_select_all(int64_t h);

/* ── QPlainTextEdit ──────────────────────────────────────────────────── */
const char* qth_textedit_get_text(int64_t h);
void        qth_textedit_set_text(int64_t h, const char* text);
void        qth_textedit_append(int64_t h, const char* text);
void        qth_textedit_clear(int64_t h);
void        qth_textedit_set_read_only(int64_t h, int8_t read_only);
void        qth_textedit_set_placeholder(int64_t h, const char* text);

/* ── QCheckBox / QRadioButton / QGroupBox (checkable) ────────────────── */
int8_t  qth_checkbox_is_checked(int64_t h);
void    qth_checkbox_set_checked(int64_t h, int8_t checked);
void    qth_checkbox_set_text(int64_t h, const char* text);
int8_t  qth_radio_is_checked(int64_t h);
void    qth_radio_set_checked(int64_t h, int8_t checked);
void    qth_groupbox_set_title(int64_t h, const char* title);
void    qth_groupbox_set_checkable(int64_t h, int8_t checkable);
int8_t  qth_groupbox_is_checked(int64_t h);
void    qth_groupbox_set_checked(int64_t h, int8_t checked);

/* ── QSlider ─────────────────────────────────────────────────────────── */
int32_t qth_slider_get_value(int64_t h);
void    qth_slider_set_value(int64_t h, int32_t value);
void    qth_slider_set_range(int64_t h, int32_t min, int32_t max);

/* ── QSpinBox / QDoubleSpinBox ───────────────────────────────────────── */
int32_t qth_spinbox_get_value(int64_t h);
void    qth_spinbox_set_value(int64_t h, int32_t value);
void    qth_spinbox_set_range(int64_t h, int32_t min, int32_t max);
void    qth_spinbox_set_step(int64_t h, int32_t step);
void    qth_spinbox_set_suffix(int64_t h, const char* suffix);
double  qth_dspinbox_get_value(int64_t h);
void    qth_dspinbox_set_value(int64_t h, double value);
void    qth_dspinbox_set_range(int64_t h, double min, double max);
void    qth_dspinbox_set_step(int64_t h, double step);

/* ── QComboBox ───────────────────────────────────────────────────────── */
void        qth_combobox_add_item(int64_t h, const char* text);
void        qth_combobox_clear(int64_t h);
int32_t     qth_combobox_count(int64_t h);
int32_t     qth_combobox_get_index(int64_t h);
void        qth_combobox_set_index(int64_t h, int32_t index);
const char* qth_combobox_get_text(int64_t h);
const char* qth_combobox_item_text(int64_t h, int32_t index);

/* ── QProgressBar ────────────────────────────────────────────────────── */
void    qth_progress_set_value(int64_t h, int32_t value);
int32_t qth_progress_get_value(int64_t h);
void    qth_progress_set_range(int64_t h, int32_t min, int32_t max);
void    qth_progress_set_text_visible(int64_t h, int8_t visible);

/* ── QTabWidget ──────────────────────────────────────────────────────── */
int32_t qth_tabs_add_tab(int64_t tabs, int64_t page, const char* title);   /* → index */
int32_t qth_tabs_get_index(int64_t tabs);
void    qth_tabs_set_index(int64_t tabs, int32_t index);
int32_t qth_tabs_count(int64_t tabs);

/* ── QListWidget ─────────────────────────────────────────────────────── */
void        qth_list_add_item(int64_t h, const char* text);
void        qth_list_clear(int64_t h);
int32_t     qth_list_count(int64_t h);
int32_t     qth_list_get_row(int64_t h);              /* -1 = nothing selected */
void        qth_list_set_row(int64_t h, int32_t row);
const char* qth_list_item_text(int64_t h, int32_t row);
void        qth_list_remove_row(int64_t h, int32_t row);

/* ── QScrollArea ─────────────────────────────────────────────────────── */
void    qth_scroll_set_widget(int64_t scroll, int64_t widget);
void    qth_scroll_set_widget_resizable(int64_t scroll, int8_t resizable);

/* ── QTimer ──────────────────────────────────────────────────────────── */
int64_t qth_timer_new(int32_t interval_ms, int8_t single_shot);   /* not started */
void    qth_timer_start(int64_t timer);
void    qth_timer_stop(int64_t timer);
void    qth_timer_set_interval(int64_t timer, int32_t interval_ms);
int8_t  qth_timer_is_active(int64_t timer);

/* ── Clipboard ───────────────────────────────────────────────────────── */
void        qth_clipboard_set_text(const char* text);
const char* qth_clipboard_get_text(void);

/* ── Dialogs (blocking) ──────────────────────────────────────────────── */
void        qth_show_info(int64_t parent, const char* title, const char* text);
void        qth_show_warning(int64_t parent, const char* title, const char* text);
void        qth_show_error(int64_t parent, const char* title, const char* text);
int8_t      qth_show_question(int64_t parent, const char* title, const char* text); /* 1 = Yes */
const char* qth_input_text(int64_t parent, const char* title, const char* label, const char* initial);
int32_t     qth_input_int(int64_t parent, const char* title, const char* label,
                          int32_t value, int32_t min, int32_t max);
int8_t      qth_input_accepted(void);   /* did the last qth_input_* end with OK? */
const char* qth_file_open(int64_t parent, const char* title, const char* dir, const char* filter);
const char* qth_file_save(int64_t parent, const char* title, const char* dir, const char* filter);
const char* qth_dir_select(int64_t parent, const char* title, const char* dir);

/* ── Event queue ─────────────────────────────────────────────────────── */
/* qth_poll_event() pops the next event and returns its kind (0 = queue
 * empty). The accessors below describe the event that was just popped. */
int32_t     qth_poll_event(void);
int64_t     qth_event_handle(void);
int32_t     qth_event_value(void);
const char* qth_event_text(void);
int32_t     qth_event_pending(void);
void        qth_event_clear(void);


/* ══ 0.3 additions ═════════════════════════════════════════════════════ */

/* ── Themes ──────────────────────────────────────────────────────────── */
/* "dark" | "light" | "system" (Fusion style + palette; "system" restores defaults). 1 = known name. */
int8_t  qth_app_set_theme(const char* name);

/* ── Images & icons ──────────────────────────────────────────────────── */
/* Paths are files (PNG/JPG/SVG/…); "theme:NAME" selects an icon from the desktop icon theme. */
int8_t  qth_label_set_image(int64_t label, const char* path);     /* 1 = loaded */
void    qth_label_set_scaled(int64_t label, int8_t scaled);       /* stretch image to the label */
void    qth_button_set_icon(int64_t button, const char* path);
void    qth_action_set_icon(int64_t action, const char* path);
void    qth_window_set_icon(int64_t window, const char* path);
void    qth_app_set_icon(const char* path);
int8_t  qth_widget_grab_png(int64_t widget, const char* path);    /* screenshot of a widget; 1 = saved */

/* ── Canvas (custom-painted widget) ──────────────────────────────────── */
int64_t qth_canvas_new(int32_t width, int32_t height);
void    qth_canvas_clear(int64_t c);                              /* drop all recorded drawing */
void    qth_canvas_set_background(int64_t c, int32_t rgb);        /* 0xRRGGBB */
void    qth_canvas_set_pen(int64_t c, int32_t rgb, double width);
void    qth_canvas_set_fill(int64_t c, int32_t rgb);              /* rgb < 0 = no fill */
void    qth_canvas_set_font(int64_t c, const char* family, int32_t point_size, int8_t bold, int8_t italic);
void    qth_canvas_draw_line(int64_t c, double x1, double y1, double x2, double y2);
void    qth_canvas_draw_rect(int64_t c, double x, double y, double w, double h);
void    qth_canvas_draw_round_rect(int64_t c, double x, double y, double w, double h);
void    qth_canvas_draw_ellipse(int64_t c, double x, double y, double w, double h);
void    qth_canvas_draw_point(int64_t c, double x, double y);
void    qth_canvas_draw_text(int64_t c, double x, double y, const char* text);
void    qth_canvas_draw_image(int64_t c, const char* path, double x, double y, double w, double h); /* w/h <= 0: natural size */
int32_t qth_canvas_op_count(int64_t c);
void    qth_canvas_update(int64_t c);

/* ── Input events ────────────────────────────────────────────────────── */
void    qth_widget_watch_input(int64_t widget, int32_t mask);     /* QTH_WATCH_* ; 0 stops watching */
int64_t qth_shortcut_new(int64_t widget, const char* sequence);   /* "Ctrl+S" … → QTH_EVENT_SHORTCUT */
int32_t qth_event_x(void);
int32_t qth_event_y(void);
int32_t qth_event_mods(void);                                     /* 1 shift, 2 ctrl, 4 alt, 8 meta */

/* ── QTableWidget ────────────────────────────────────────────────────── */
int64_t     qth_table_new(int32_t rows, int32_t cols);
void        qth_table_set_size(int64_t t, int32_t rows, int32_t cols);
int32_t     qth_table_rows(int64_t t);
int32_t     qth_table_cols(int64_t t);
void        qth_table_set_cell(int64_t t, int32_t row, int32_t col, const char* text);  /* silent: no event */
const char* qth_table_get_cell(int64_t t, int32_t row, int32_t col);
void        qth_table_set_col_header(int64_t t, int32_t col, const char* text);
void        qth_table_set_row_header(int64_t t, int32_t row, const char* text);
int32_t     qth_table_add_row(int64_t t);                         /* → index of the new row */
void        qth_table_insert_row(int64_t t, int32_t at);
void        qth_table_remove_row(int64_t t, int32_t row);
void        qth_table_clear(int64_t t);                           /* clears cells, keeps size & headers */
int32_t     qth_table_current_row(int64_t t);                     /* -1 = none */
int32_t     qth_table_current_col(int64_t t);
void        qth_table_set_current(int64_t t, int32_t row, int32_t col);
void        qth_table_set_sorting(int64_t t, int8_t enabled);     /* click on header sorts */
void        qth_table_sort_by(int64_t t, int32_t col, int8_t descending);
void        qth_table_set_editable(int64_t t, int8_t editable);
void        qth_table_set_select_rows(int64_t t, int8_t whole_rows);
void        qth_table_set_alternating_colors(int64_t t, int8_t on);
void        qth_table_set_col_width(int64_t t, int32_t col, int32_t px);
void        qth_table_stretch_columns(int64_t t, int8_t stretch);
void        qth_table_set_cell_background(int64_t t, int32_t row, int32_t col, int32_t rgb);
void        qth_table_set_cell_foreground(int64_t t, int32_t row, int32_t col, int32_t rgb);

/* ── QTreeWidget (items are int64 ids; 0 = invisible root / none) ────── */
int64_t     qth_tree_new(int32_t columns);
void        qth_tree_set_header(int64_t t, int32_t col, const char* text);
void        qth_tree_set_header_visible(int64_t t, int8_t visible);
int64_t     qth_tree_add_item(int64_t t, int64_t parent, const char* text);   /* → item id */
void        qth_tree_set_item_text(int64_t t, int64_t item, int32_t col, const char* text);
const char* qth_tree_get_item_text(int64_t t, int64_t item, int32_t col);
void        qth_tree_remove_item(int64_t t, int64_t item);
void        qth_tree_clear(int64_t t);
int64_t     qth_tree_current_item(int64_t t);
void        qth_tree_set_current_item(int64_t t, int64_t item);
int64_t     qth_tree_item_parent(int64_t t, int64_t item);
int32_t     qth_tree_child_count(int64_t t, int64_t item);                    /* item 0 = top level */
int64_t     qth_tree_child_at(int64_t t, int64_t item, int32_t index);
void        qth_tree_set_expanded(int64_t t, int64_t item, int8_t expanded);
void        qth_tree_expand_all(int64_t t);
void        qth_tree_collapse_all(int64_t t);
void        qth_tree_set_sorting(int64_t t, int8_t enabled);

/* ── Date / time / calendar ──────────────────────────────────────────── */
int64_t     qth_dateedit_new(int32_t year, int32_t month, int32_t day);
void        qth_dateedit_set_date(int64_t h, int32_t year, int32_t month, int32_t day);
int32_t     qth_dateedit_get_ymd(int64_t h);                      /* yyyymmdd */
const char* qth_dateedit_get_text(int64_t h);                     /* "yyyy-MM-dd" */
void        qth_dateedit_set_popup(int64_t h, int8_t popup);      /* calendar drop-down */
int64_t     qth_timeedit_new(int32_t hour, int32_t minute, int32_t second);
void        qth_timeedit_set_time(int64_t h, int32_t hour, int32_t minute, int32_t second);
int32_t     qth_timeedit_get_seconds(int64_t h);                  /* seconds since midnight */
const char* qth_timeedit_get_text(int64_t h);                     /* "HH:mm:ss" */
int64_t     qth_calendar_new(void);
void        qth_calendar_set_date(int64_t h, int32_t year, int32_t month, int32_t day);
int32_t     qth_calendar_get_ymd(int64_t h);
const char* qth_calendar_get_text(int64_t h);
int32_t     qth_date_today_ymd(void);

/* ── Splitter / stacked widget / dock ────────────────────────────────── */
int64_t qth_splitter_new(int8_t vertical);
void    qth_splitter_add_widget(int64_t s, int64_t widget);
void    qth_splitter_set_sizes2(int64_t s, int32_t a, int32_t b);
void    qth_splitter_set_sizes3(int64_t s, int32_t a, int32_t b, int32_t c);
void    qth_splitter_set_stretch(int64_t s, int32_t index, int32_t stretch);
int64_t qth_stack_new(void);
int32_t qth_stack_add_page(int64_t s, int64_t page);              /* → index */
void    qth_stack_set_index(int64_t s, int32_t index);            /* emits QTH_EVENT_INDEX_CHANGED */
int32_t qth_stack_get_index(int64_t s);
int32_t qth_stack_count(int64_t s);
int64_t qth_window_add_dock(int64_t window, const char* title, int64_t content, int32_t area);

/* ── More dialogs ────────────────────────────────────────────────────── */
int32_t     qth_color_pick(int64_t parent, const char* title, int32_t initial_rgb); /* -1 = cancelled */
const char* qth_font_pick(int64_t parent, const char* title, const char* family, int32_t size); /* family; "" = cancelled */
int32_t     qth_font_last_size(void);
int8_t      qth_font_last_bold(void);
int8_t      qth_font_last_italic(void);
int64_t     qth_progress_dialog_new(int64_t parent, const char* title, const char* label,
                                    int32_t min, int32_t max);    /* non-modal QProgressDialog */
void        qth_progress_dialog_set_value(int64_t h, int32_t value);
void        qth_progress_dialog_set_label(int64_t h, const char* text);
int8_t      qth_progress_dialog_was_canceled(int64_t h);

/* ── Standalone menus, system tray, notifications ────────────────────── */
int64_t qth_menu_new(const char* title);                          /* parentless QMenu (tray / context) */
void    qth_menu_popup(int64_t menu, int32_t x, int32_t y);       /* non-blocking context popup at screen x/y */
int8_t  qth_tray_available(void);
int64_t qth_tray_new(const char* icon_path, const char* tooltip);
void    qth_tray_set_icon(int64_t tray, const char* icon_path);
void    qth_tray_set_tooltip(int64_t tray, const char* tooltip);
void    qth_tray_set_menu(int64_t tray, int64_t menu);
void    qth_tray_show(int64_t tray);
void    qth_tray_hide(int64_t tray);
/* kind: 0 none, 1 info, 2 warning, 3 critical */
void    qth_tray_notify(int64_t tray, const char* title, const char* message, int32_t kind, int32_t timeout_ms);

/* ── Memory ──────────────────────────────────────────────────────────── */
void qth_free_string(const char* s);

#ifdef __cplusplus
}
#endif

#endif /* QTHSHARP_H */
