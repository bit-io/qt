#include "common.hpp"

#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QProgressDialog>
#include <QInputDialog>
#include <QMessageBox>

using namespace qth;
using namespace qth::api;

namespace {
bool g_input_accepted = false;
int  g_font_size = 0;
bool g_font_bold = false;
bool g_font_italic = false;
} // namespace

extern "C" {

void qth_show_info(int64_t parent, const char* title, const char* text) {
    QMessageBox::information(widget(parent), qs(title), qs(text));
}

void qth_show_warning(int64_t parent, const char* title, const char* text) {
    QMessageBox::warning(widget(parent), qs(title), qs(text));
}

void qth_show_error(int64_t parent, const char* title, const char* text) {
    QMessageBox::critical(widget(parent), qs(title), qs(text));
}

int8_t qth_show_question(int64_t parent, const char* title, const char* text) {
    const auto rc = QMessageBox::question(widget(parent), qs(title), qs(text),
                                          QMessageBox::Yes | QMessageBox::No);
    return rc == QMessageBox::Yes ? 1 : 0;
}

const char* qth_input_text(int64_t parent, const char* title, const char* label, const char* initial) {
    bool ok = false;
    const QString r = QInputDialog::getText(widget(parent), qs(title), qs(label),
                                            QLineEdit::Normal, qs(initial), &ok);
    g_input_accepted = ok;
    return out(ok ? r : QString());
}

int32_t qth_input_int(int64_t parent, const char* title, const char* label,
                      int32_t value, int32_t min, int32_t max) {
    bool ok = false;
    const int r = QInputDialog::getInt(widget(parent), qs(title), qs(label), value, min, max, 1, &ok);
    g_input_accepted = ok;
    return ok ? r : value;
}

int8_t qth_input_accepted(void) { return g_input_accepted ? 1 : 0; }

// An empty string means "cancelled".
const char* qth_file_open(int64_t parent, const char* title, const char* dir, const char* filter) {
    return out(QFileDialog::getOpenFileName(widget(parent), qs(title), qs(dir), qs(filter)));
}

const char* qth_file_save(int64_t parent, const char* title, const char* dir, const char* filter) {
    return out(QFileDialog::getSaveFileName(widget(parent), qs(title), qs(dir), qs(filter)));
}

const char* qth_dir_select(int64_t parent, const char* title, const char* dir) {
    return out(QFileDialog::getExistingDirectory(widget(parent), qs(title), qs(dir)));
}

// ── 0.3: colour / font / progress dialogs ──────────────────────────────

int32_t qth_color_pick(int64_t parent, const char* title, int32_t initial_rgb) {
    const QColor c = QColorDialog::getColor(color_from_rgb(initial_rgb), widget(parent), qs(title));
    g_input_accepted = c.isValid();
    if (!c.isValid()) return -1;
    return (c.red() << 16) | (c.green() << 8) | c.blue();
}

const char* qth_font_pick(int64_t parent, const char* title, const char* family, int32_t size) {
    bool ok = false;
    const QFont f = QFontDialog::getFont(&ok, QFont(qs(family), size > 0 ? size : 10), widget(parent), qs(title));
    g_input_accepted = ok;
    if (!ok) return out(QString());
    g_font_size = f.pointSize();
    g_font_bold = f.bold();
    g_font_italic = f.italic();
    return out(f.family());
}
int32_t qth_font_last_size(void)   { return g_font_size; }
int8_t  qth_font_last_bold(void)   { return g_font_bold ? 1 : 0; }
int8_t  qth_font_last_italic(void) { return g_font_italic ? 1 : 0; }

int64_t qth_progress_dialog_new(int64_t parent, const char* title, const char* label, int32_t min, int32_t max) {
    auto* d = new QProgressDialog(qs(label), QStringLiteral("Anuluj"), min, max, widget(parent));
    d->setWindowTitle(qs(title));
    d->setWindowModality(Qt::NonModal);
    d->setAutoClose(false);
    d->setAutoReset(false);
    d->setMinimumDuration(0);
    d->setValue(min);
    d->show();
    return reg(d);
}
void qth_progress_dialog_set_value(int64_t h, int32_t value) {
    if (auto* d = get<QProgressDialog>(h)) d->setValue(value);
}
void qth_progress_dialog_set_label(int64_t h, const char* text) {
    if (auto* d = get<QProgressDialog>(h)) d->setLabelText(qs(text));
}
int8_t qth_progress_dialog_was_canceled(int64_t h) {
    auto* d = get<QProgressDialog>(h);
    return d && d->wasCanceled() ? 1 : 0;
}

} // extern "C"
