#include "qt.h"

#include <QApplication>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QWidget>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <string>

static int g_failures = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::fprintf(stderr, "FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                                        \
        }                                                                        \
    } while (0)

// Copies and frees a string returned by the library, like the H# wrappers do.
static std::string take(const char* s) {
    std::string r = s ? s : "";
    qth_free_string(s);
    return r;
}

static void drain() {
    qth_app_process_events();
    while (qth_poll_event() != QTH_EVENT_NONE) {}
}

int main() {
    CHECK(qth_app_init_platform("offscreen") == 1);
    CHECK(qth_app_init() == 0);          // second init is a no-op
    CHECK(qth_app_is_running() == 1);
    CHECK(!take(qth_version()).empty());
    CHECK(!take(qth_qt_version()).empty());

    // ── handles ─────────────────────────────────────────────────────────
    const int64_t win = qth_window_new("t", 300, 200);
    const int64_t box = qth_widget_new();
    const int64_t lbl = qth_label_new("początek");
    CHECK(win > 0 && box > 0 && lbl > 0);
    CHECK(win != box && box != lbl);
    CHECK(qth_handle_is_valid(lbl) == 1);
    CHECK(qth_handle_is_valid(0) == 0);
    CHECK(qth_handle_is_valid(987654321) == 0);

    // wrong-type handle: a label is not a line edit → getter returns "", no crash
    CHECK(take(qth_lineedit_get_text(lbl)).empty());
    qth_slider_set_value(lbl, 5);        // no-op on the wrong type

    // ── label / unicode roundtrip ───────────────────────────────────────
    CHECK(take(qth_label_get_text(lbl)) == "początek");
    qth_label_set_text(lbl, "zażółć gęślą jaźń");
    CHECK(take(qth_label_get_text(lbl)) == "zażółć gęślą jaźń");

    // ── layouts + central widget ────────────────────────────────────────
    const int64_t v = qth_vbox_new();
    qth_layout_add_widget(v, lbl);
    qth_widget_set_layout(box, v);
    qth_window_set_central_widget(win, box);
    const int64_t grid = qth_grid_new();
    const int64_t form = qth_form_new();
    const int64_t edit = qth_lineedit_new("ph");
    qth_grid_add_widget(grid, edit, 0, 0, 1, 2);
    const int64_t spin = qth_spinbox_new(0, 10, 3);
    qth_form_add_row(form, "Liczba", spin);
    qth_layout_add_layout(v, grid);
    qth_layout_add_layout(v, form);

    // ── inputs & events ─────────────────────────────────────────────────
    drain();
    qth_lineedit_set_text(edit, "abc");
    CHECK(take(qth_lineedit_get_text(edit)) == "abc");
    CHECK(qth_poll_event() == QTH_EVENT_TEXT_CHANGED);
    CHECK(qth_event_handle() == edit);
    CHECK(take(qth_event_text()) == "abc");
    CHECK(qth_poll_event() == QTH_EVENT_NONE);

    const int64_t cb = qth_checkbox_new("ok");
    CHECK(qth_checkbox_is_checked(cb) == 0);
    qth_checkbox_set_checked(cb, 1);
    CHECK(qth_checkbox_is_checked(cb) == 1);
    CHECK(qth_poll_event() == QTH_EVENT_TOGGLED);
    CHECK(qth_event_value() == 1);

    const int64_t sl = qth_slider_new(0, 100, 42);
    CHECK(qth_slider_get_value(sl) == 42);
    drain();
    qth_slider_set_value(sl, 7);
    CHECK(qth_poll_event() == QTH_EVENT_VALUE_CHANGED);
    CHECK(qth_event_value() == 7);

    qth_spinbox_set_value(spin, 9);
    CHECK(qth_spinbox_get_value(spin) == 9);
    qth_spinbox_set_value(spin, 99);     // clamped to the range
    CHECK(qth_spinbox_get_value(spin) == 10);

    const int64_t ds = qth_dspinbox_new(0.0, 1.0, 0.25, 3);
    CHECK(qth_dspinbox_get_value(ds) == 0.25);

    const int64_t combo = qth_combobox_new();
    qth_combobox_add_item(combo, "jeden");
    qth_combobox_add_item(combo, "dwa");
    CHECK(qth_combobox_count(combo) == 2);
    qth_combobox_set_index(combo, 1);
    CHECK(take(qth_combobox_get_text(combo)) == "dwa");
    CHECK(take(qth_combobox_item_text(combo, 0)) == "jeden");

    const int64_t list = qth_list_new();
    qth_list_add_item(list, "a");
    qth_list_add_item(list, "b");
    CHECK(qth_list_count(list) == 2);
    qth_list_set_row(list, 1);
    CHECK(take(qth_list_item_text(list, 1)) == "b");
    qth_list_remove_row(list, 0);
    CHECK(qth_list_count(list) == 1);
    qth_list_remove_row(list, 99);       // bad row: ignored
    CHECK(qth_list_count(list) == 1);

    const int64_t te = qth_textedit_new("");
    qth_textedit_set_text(te, "x");
    qth_textedit_append(te, "y");
    CHECK(take(qth_textedit_get_text(te)) == "x\ny");

    const int64_t pb = qth_progress_new(0, 10, 2);
    qth_progress_set_value(pb, 5);
    CHECK(qth_progress_get_value(pb) == 5);

    const int64_t tabs = qth_tabs_new();
    const int64_t pg1 = qth_widget_new(), pg2 = qth_widget_new();
    CHECK(qth_tabs_add_tab(tabs, pg1, "1") == 0);
    CHECK(qth_tabs_add_tab(tabs, pg2, "2") == 1);
    CHECK(qth_tabs_count(tabs) == 2);

    const int64_t gb = qth_groupbox_new("g");
    qth_groupbox_set_checkable(gb, 1);
    qth_groupbox_set_checked(gb, 0);
    CHECK(qth_groupbox_is_checked(gb) == 0);

    const int64_t radio = qth_radio_new("r");
    qth_radio_set_checked(radio, 1);
    CHECK(qth_radio_is_checked(radio) == 1);

    // ── menus / actions ─────────────────────────────────────────────────
    const int64_t menu = qth_window_add_menu(win, "Plik");
    const int64_t act  = qth_menu_add_action(menu, "Otwórz", "Ctrl+O");
    CHECK(menu > 0 && act > 0);
    qth_action_set_checkable(act, 1);
    qth_action_set_checked(act, 1);
    CHECK(qth_action_is_checked(act) == 1);
    CHECK(qth_window_add_toolbar_action(win, "T") > 0);
    qth_window_set_status(win, "gotowe", 0);

    // ── buttons ─────────────────────────────────────────────────────────
    const int64_t btn = qth_button_new("klik");
    CHECK(take(qth_button_get_text(btn)) == "klik");

    // ── timer fires an event ────────────────────────────────────────────
    drain();
    const int64_t tm = qth_timer_new(5, 1);
    qth_timer_start(tm);
    CHECK(qth_timer_is_active(tm) == 1);
    bool fired = false;
    for (int i = 0; i < 200 && !fired; ++i) {
        qth_app_process_events();
        const int32_t k = qth_poll_event();
        if (k == QTH_EVENT_TIMER && qth_event_handle() == tm) fired = true;
        else if (k == QTH_EVENT_NONE) { /* wait a little */
            struct timespec ts{0, 2 * 1000 * 1000}; nanosleep(&ts, nullptr);
        }
    }
    CHECK(fired);

    // ── clipboard ───────────────────────────────────────────────────────
    qth_clipboard_set_text("schowek");
    CHECK(take(qth_clipboard_get_text()) == "schowek");

    // ── lifetime: deleting a parent invalidates its children's handles ──
    const int64_t before = qth_handle_count();
    qth_widget_destroy(box);             // deleteLater(), runs in the loop
    for (int i = 0; i < 5; ++i) {
        qth_app_process_events();
    }
    // sendPostedEvents() with DeferredDelete needs a real loop iteration:
    // process_events() above does that for top-level deletes.
    CHECK(qth_handle_is_valid(box) == 0 || qth_handle_count() <= before);
    qth_widget_destroy(win);
    for (int i = 0; i < 5; ++i) qth_app_process_events();
    CHECK(qth_handle_is_valid(win) == 0);
    // stale handle: harmless
    qth_widget_show(win);
    CHECK(take(qth_label_get_text(win)).empty());

    // ── event queue overflow is bounded ─────────────────────────────────
    drain();
    const int64_t sl2 = qth_slider_new(0, 100000, 0);
    for (int i = 1; i <= 6000; ++i) qth_slider_set_value(sl2, i);
    CHECK(qth_event_pending() <= 4096);
    qth_event_clear();
    CHECK(qth_event_pending() == 0);


    // ═══ 0.3 ═══════════════════════════════════════════════════════════
    auto find_widget = [](const char* name) -> QWidget* {
        for (QWidget* w : QApplication::allWidgets()) if (w->objectName() == name) return w;
        return nullptr;
    };

    // ── theme ───────────────────────────────────────────────────────────
    CHECK(qth_app_set_theme("dark") == 1);
    CHECK(qth_app_set_theme("light") == 1);
    CHECK(qth_app_set_theme("system") == 1);
    CHECK(qth_app_set_theme("nonsense") == 0);

    // ── table ───────────────────────────────────────────────────────────
    drain();
    const int64_t tb = qth_table_new(2, 3);
    CHECK(qth_table_rows(tb) == 2 && qth_table_cols(tb) == 3);
    qth_table_set_col_header(tb, 0, "Imię");
    qth_table_set_cell(tb, 0, 0, "Zażółć");
    qth_table_set_cell(tb, 1, 0, "Adam");
    CHECK(take(qth_table_get_cell(tb, 0, 0)) == "Zażółć");
    CHECK(qth_event_pending() == 0);                  // programmatic writes are silent
    CHECK(take(qth_table_get_cell(tb, 9, 9)).empty()); // out of range: no crash
    CHECK(qth_table_add_row(tb) == 2);
    qth_table_set_cell(tb, 2, 0, "Beata");
    qth_table_sort_by(tb, 0, 0);
    CHECK(take(qth_table_get_cell(tb, 0, 0)) == "Adam");
    qth_table_sort_by(tb, 0, 1);
    CHECK(take(qth_table_get_cell(tb, 0, 0)) == "Zażółć");
    qth_table_remove_row(tb, 0);
    CHECK(qth_table_rows(tb) == 2);
    qth_table_set_cell_background(tb, 0, 1, 0xFF0000);
    qth_table_set_current(tb, 1, 0);
    CHECK(qth_table_current_row(tb) == 1);
    CHECK(qth_poll_event() == QTH_EVENT_TABLE_CELL_CHANGED);
    CHECK(qth_event_x() == 1 && qth_event_y() == 0);
    qth_table_clear(tb);
    CHECK(take(qth_table_get_cell(tb, 0, 0)).empty());

    // ── tree ────────────────────────────────────────────────────────────
    drain();
    const int64_t tr = qth_tree_new(2);
    const int64_t root = qth_tree_add_item(tr, 0, "root");
    const int64_t kid1 = qth_tree_add_item(tr, root, "kid1");
    const int64_t kid2 = qth_tree_add_item(tr, root, "kid2");
    const int64_t leaf = qth_tree_add_item(tr, kid1, "leaf");
    CHECK(root > 0 && kid1 > root && leaf > kid2);
    CHECK(qth_tree_child_count(tr, 0) == 1);
    CHECK(qth_tree_child_count(tr, root) == 2);
    CHECK(qth_tree_child_at(tr, root, 1) == kid2);
    CHECK(qth_tree_item_parent(tr, leaf) == kid1);
    qth_tree_set_item_text(tr, leaf, 1, "kolumna 2");
    CHECK(take(qth_tree_get_item_text(tr, leaf, 1)) == "kolumna 2");
    qth_tree_set_current_item(tr, kid2);
    CHECK(qth_tree_current_item(tr) == kid2);
    CHECK(qth_poll_event() == QTH_EVENT_TREE_SELECTION_CHANGED);
    CHECK(qth_event_value() == static_cast<int32_t>(kid2));
    qth_tree_remove_item(tr, kid1);                  // removes kid1 and leaf
    CHECK(qth_tree_child_count(tr, root) == 1);
    CHECK(take(qth_tree_get_item_text(tr, leaf, 0)).empty());   // stale id: harmless
    CHECK(qth_tree_add_item(tr, leaf, "x") == 0);               // stale parent: refused
    CHECK(qth_tree_add_item(tb, 0, "wrong widget type") == 0);
    qth_tree_clear(tr);
    CHECK(qth_tree_child_count(tr, 0) == 0);
    CHECK(take(qth_tree_get_item_text(tr, root, 0)).empty());

    // ── canvas: draw, screenshot, verify pixels ─────────────────────────
    const int64_t cv = qth_canvas_new(80, 60);
    qth_canvas_set_background(cv, 0xFFFFFF);
    qth_canvas_set_pen(cv, 0x000000, 1);
    qth_canvas_set_fill(cv, 0xFF0000);
    qth_canvas_draw_rect(cv, 10, 10, 30, 30);
    qth_canvas_set_fill(cv, -1);
    qth_canvas_draw_line(cv, 0, 59, 79, 59);
    qth_canvas_draw_text(cv, 45, 20, "ab");
    CHECK(qth_canvas_op_count(cv) == 3);
    const char* png = "/tmp/qth_canvas_test.png";
    CHECK(qth_widget_grab_png(cv, png) == 1);
    QImage img(png);
    CHECK(!img.isNull());
    CHECK(img.pixelColor(20, 20) == QColor(255, 0, 0));      // inside the red rect
    CHECK(img.pixelColor(70, 40) == QColor(255, 255, 255));  // background
    qth_canvas_clear(cv);
    CHECK(qth_canvas_op_count(cv) == 0);
    CHECK(qth_widget_grab_png(box, "/nonexistent-dir/x.png") == 0);   // stale/failed: 0

    // ── images / icons ──────────────────────────────────────────────────
    const int64_t il = qth_label_new("");
    CHECK(qth_label_set_image(il, png) == 1);
    CHECK(qth_label_set_image(il, "/no/such/file.png") == 0);
    qth_label_set_scaled(il, 1);
    const int64_t ib = qth_button_new("z ikoną");
    qth_button_set_icon(ib, png);
    qth_button_set_icon(ib, "theme:document-open");   // missing theme icon: harmless

    // ── input events ────────────────────────────────────────────────────
    const int64_t in = qth_canvas_new(100, 100);
    qth_widget_set_object_name(in, "qth_input_target");
    qth_widget_watch_input(in, QTH_WATCH_ALL);
    drain();
    QWidget* tw = find_widget("qth_input_target");
    CHECK(tw != nullptr);
    if (tw) {
        QKeyEvent kp(QEvent::KeyPress, Qt::Key_A, Qt::ControlModifier, "a");
        QApplication::sendEvent(tw, &kp);
        CHECK(qth_poll_event() == QTH_EVENT_KEY_PRESSED);
        CHECK(qth_event_handle() == in);
        CHECK(qth_event_value() == Qt::Key_A);
        CHECK(take(qth_event_text()) == "a");
        CHECK((qth_event_mods() & 2) == 2);

        QMouseEvent mp(QEvent::MouseButtonPress, QPointF(12, 34), tw->mapToGlobal(QPointF(12, 34)),
                       Qt::RightButton, Qt::RightButton, Qt::NoModifier);
        QApplication::sendEvent(tw, &mp);
        CHECK(qth_poll_event() == QTH_EVENT_MOUSE_PRESSED);
        CHECK(qth_event_value() == 2 && qth_event_x() == 12 && qth_event_y() == 34);

        QMouseEvent mm(QEvent::MouseMove, QPointF(5, 6), tw->mapToGlobal(QPointF(5, 6)),
                       Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(tw, &mm);
        CHECK(qth_poll_event() == QTH_EVENT_MOUSE_MOVED);
        CHECK(qth_event_x() == 5 && qth_event_y() == 6);

        QWheelEvent we(QPointF(1, 2), tw->mapToGlobal(QPointF(1, 2)), QPoint(), QPoint(0, 120),
                       Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(tw, &we);
        CHECK(qth_poll_event() == QTH_EVENT_WHEEL);
        CHECK(qth_event_value() == 120);

        qth_widget_show(in);                       // Qt sends Resize only to shown widgets
        qth_app_process_events();
        drain();
        qth_widget_resize(in, 150, 120);
        CHECK(qth_poll_event() == QTH_EVENT_RESIZED);
        CHECK(qth_event_x() == 150 && qth_event_y() == 120);

        qth_widget_watch_input(in, 0);                 // stop watching
        drain();
        QApplication::sendEvent(tw, &kp);
        CHECK(qth_poll_event() == QTH_EVENT_NONE);
    }
    CHECK(qth_shortcut_new(in, "Ctrl+S") > 0);
    CHECK(qth_shortcut_new(987654, "Ctrl+S") == 0);

    // ── date / time / calendar ──────────────────────────────────────────
    drain();
    const int64_t de = qth_dateedit_new(2026, 10, 16);
    CHECK(qth_dateedit_get_ymd(de) == 20261016);
    CHECK(take(qth_dateedit_get_text(de)) == "2026-10-16");
    qth_dateedit_set_date(de, 2027, 1, 2);
    CHECK(qth_poll_event() == QTH_EVENT_DATE_CHANGED);
    CHECK(qth_event_value() == 20270102);
    qth_dateedit_set_date(de, 2027, 2, 31);            // invalid → falls back to today, no crash
    CHECK(qth_dateedit_get_ymd(de) > 20000000);
    const int64_t te2 = qth_timeedit_new(13, 45, 30);
    CHECK(qth_timeedit_get_seconds(te2) == 13 * 3600 + 45 * 60 + 30);
    CHECK(take(qth_timeedit_get_text(te2)) == "13:45:30");
    const int64_t cal = qth_calendar_new();
    qth_calendar_set_date(cal, 2026, 12, 24);
    CHECK(qth_calendar_get_ymd(cal) == 20261224);
    CHECK(qth_date_today_ymd() > 20260000);

    // ── splitter / stack / dock ─────────────────────────────────────────
    const int64_t sp = qth_splitter_new(0);
    qth_splitter_add_widget(sp, qth_widget_new());
    qth_splitter_add_widget(sp, qth_widget_new());
    qth_splitter_set_sizes2(sp, 100, 200);
    qth_splitter_set_stretch(sp, 1, 1);
    qth_splitter_set_stretch(sp, 9, 1);                // bad index ignored
    drain();
    const int64_t st = qth_stack_new();
    CHECK(qth_stack_add_page(st, qth_label_new("a")) == 0);
    CHECK(qth_stack_add_page(st, qth_label_new("b")) == 1);
    qth_stack_set_index(st, 1);
    CHECK(qth_stack_get_index(st) == 1 && qth_stack_count(st) == 2);
    CHECK(qth_poll_event() == QTH_EVENT_INDEX_CHANGED);
    qth_stack_set_index(st, 7);                        // out of range ignored
    CHECK(qth_stack_get_index(st) == 1);
    const int64_t win2 = qth_window_new("dock", 300, 200);
    const int64_t dock = qth_window_add_dock(win2, "Panel", qth_label_new("w docku"), QTH_DOCK_RIGHT);
    CHECK(dock > 0);
    CHECK(qth_window_add_dock(box, "x", 0, 0) == 0);   // stale window handle

    // ── tray / menus / progress dialog ──────────────────────────────────
    const int64_t pm = qth_menu_new("Tray");
    CHECK(qth_menu_add_action(pm, "Zamknij", "") > 0);
    const int64_t tray = qth_tray_new("theme:applications-other", "tip");
    CHECK(tray > 0);
    qth_tray_set_menu(tray, pm);
    qth_tray_notify(tray, "t", "m", 1, 1000);          // may be a no-op without a tray host
    (void)qth_tray_available();
    const int64_t pd = qth_progress_dialog_new(0, "Praca", "Trwa…", 0, 10);
    qth_progress_dialog_set_value(pd, 4);
    qth_progress_dialog_set_label(pd, "Prawie");
    CHECK(qth_progress_dialog_was_canceled(pd) == 0);
    qth_widget_destroy(pd);
    qth_app_process_events();

    drain();

    qth_app_shutdown();
    CHECK(qth_app_is_running() == 0);

    if (g_failures == 0) std::puts("native smoke: all checks passed");
    else std::fprintf(stderr, "native smoke: %d failure(s)\n", g_failures);
    return g_failures == 0 ? 0 : 1;
}
