#include "common.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>

using namespace qth;
using namespace qth::api;

extern "C" {

// ── creation ────────────────────────────────────────────────────────────

int64_t qth_lineedit_new(const char* placeholder) {
    auto* e = new QLineEdit();
    if (placeholder) e->setPlaceholderText(qs(placeholder));
    const int64_t id = reg(e);
    QObject::connect(e, &QLineEdit::textChanged, [id](const QString& t) {
        emit_event(QTH_EVENT_TEXT_CHANGED, id, t.size(), t);
    });
    QObject::connect(e, &QLineEdit::returnPressed, [id, e] {
        emit_event(QTH_EVENT_RETURN_PRESSED, id, 0, e->text());
    });
    return id;
}

int64_t qth_textedit_new(const char* placeholder) {
    auto* e = new QPlainTextEdit();
    if (placeholder) e->setPlaceholderText(qs(placeholder));
    const int64_t id = reg(e);
    QObject::connect(e, &QPlainTextEdit::textChanged, [id, e] {
        const QString t = e->toPlainText();
        emit_event(QTH_EVENT_TEXT_CHANGED, id, t.size(), t);
    });
    return id;
}

int64_t qth_radio_new(const char* text) {
    auto* r = new QRadioButton(qs(text));
    const int64_t id = reg(r);
    QObject::connect(r, &QRadioButton::toggled, [id](bool on) {
        emit_event(QTH_EVENT_TOGGLED, id, on ? 1 : 0);
    });
    return id;
}

int64_t qth_slider_new(int32_t min, int32_t max, int32_t initial) {
    auto* s = new QSlider(Qt::Horizontal);
    s->setRange(min, max);
    s->setValue(initial);
    const int64_t id = reg(s);
    QObject::connect(s, &QSlider::valueChanged, [id](int v) {
        emit_event(QTH_EVENT_VALUE_CHANGED, id, v);
    });
    return id;
}

int64_t qth_spinbox_new(int32_t min, int32_t max, int32_t initial) {
    auto* s = new QSpinBox();
    s->setRange(min, max);
    s->setValue(initial);
    const int64_t id = reg(s);
    QObject::connect(s, QOverload<int>::of(&QSpinBox::valueChanged), [id](int v) {
        emit_event(QTH_EVENT_VALUE_CHANGED, id, v);
    });
    return id;
}

int64_t qth_dspinbox_new(double min, double max, double initial, int32_t decimals) {
    auto* s = new QDoubleSpinBox();
    s->setDecimals(decimals < 0 ? 2 : decimals);
    s->setRange(min, max);
    s->setValue(initial);
    const int64_t id = reg(s);
    QObject::connect(s, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [id](double v) {
        emit_event(QTH_EVENT_VALUE_CHANGED, id, static_cast<int32_t>(v), QString::number(v, 'g', 15));
    });
    return id;
}

int64_t qth_combobox_new(void) {
    auto* c = new QComboBox();
    const int64_t id = reg(c);
    QObject::connect(c, QOverload<int>::of(&QComboBox::currentIndexChanged), [id, c](int i) {
        emit_event(QTH_EVENT_INDEX_CHANGED, id, i, c->itemText(i));
    });
    return id;
}

int64_t qth_list_new(void) {
    auto* l = new QListWidget();
    const int64_t id = reg(l);
    QObject::connect(l, &QListWidget::currentRowChanged, [id, l](int row) {
        auto* it = l->item(row);
        emit_event(QTH_EVENT_SELECTION_CHANGED, id, row, it ? it->text() : QString());
    });
    QObject::connect(l, &QListWidget::itemActivated, [id, l](QListWidgetItem* it) {
        emit_event(QTH_EVENT_ITEM_ACTIVATED, id, l->row(it), it ? it->text() : QString());
    });
    return id;
}

// ── QLineEdit ───────────────────────────────────────────────────────────

const char* qth_lineedit_get_text(int64_t h) {
    auto* e = get<QLineEdit>(h);
    return out(e ? e->text() : QString());
}
void qth_lineedit_set_text(int64_t h, const char* text) {
    if (auto* e = get<QLineEdit>(h)) e->setText(qs(text));
}
void qth_lineedit_set_placeholder(int64_t h, const char* text) {
    if (auto* e = get<QLineEdit>(h)) e->setPlaceholderText(qs(text));
}
void qth_lineedit_set_read_only(int64_t h, int8_t read_only) {
    if (auto* e = get<QLineEdit>(h)) e->setReadOnly(read_only != 0);
}
void qth_lineedit_set_echo_password(int64_t h, int8_t is_password) {
    if (auto* e = get<QLineEdit>(h)) {
        e->setEchoMode(is_password != 0 ? QLineEdit::Password : QLineEdit::Normal);
    }
}
void qth_lineedit_set_max_length(int64_t h, int32_t n) {
    if (auto* e = get<QLineEdit>(h)) e->setMaxLength(n);
}
void qth_lineedit_select_all(int64_t h) {
    if (auto* e = get<QLineEdit>(h)) e->selectAll();
}

// ── QPlainTextEdit ──────────────────────────────────────────────────────

const char* qth_textedit_get_text(int64_t h) {
    auto* e = get<QPlainTextEdit>(h);
    return out(e ? e->toPlainText() : QString());
}
void qth_textedit_set_text(int64_t h, const char* text) {
    if (auto* e = get<QPlainTextEdit>(h)) e->setPlainText(qs(text));
}
void qth_textedit_append(int64_t h, const char* text) {
    if (auto* e = get<QPlainTextEdit>(h)) e->appendPlainText(qs(text));
}
void qth_textedit_clear(int64_t h) {
    if (auto* e = get<QPlainTextEdit>(h)) e->clear();
}
void qth_textedit_set_read_only(int64_t h, int8_t read_only) {
    if (auto* e = get<QPlainTextEdit>(h)) e->setReadOnly(read_only != 0);
}
void qth_textedit_set_placeholder(int64_t h, const char* text) {
    if (auto* e = get<QPlainTextEdit>(h)) e->setPlaceholderText(qs(text));
}

// ── QRadioButton ────────────────────────────────────────────────────────

int8_t qth_radio_is_checked(int64_t h) {
    auto* r = get<QRadioButton>(h);
    return r && r->isChecked() ? 1 : 0;
}
void qth_radio_set_checked(int64_t h, int8_t checked) {
    if (auto* r = get<QRadioButton>(h)) r->setChecked(checked != 0);
}

// ── QSlider ─────────────────────────────────────────────────────────────

int32_t qth_slider_get_value(int64_t h) {
    auto* s = get<QSlider>(h);
    return s ? s->value() : 0;
}
void qth_slider_set_value(int64_t h, int32_t value) {
    if (auto* s = get<QSlider>(h)) s->setValue(value);
}
void qth_slider_set_range(int64_t h, int32_t min, int32_t max) {
    if (auto* s = get<QSlider>(h)) s->setRange(min, max);
}

// ── QSpinBox / QDoubleSpinBox ───────────────────────────────────────────

int32_t qth_spinbox_get_value(int64_t h) {
    auto* s = get<QSpinBox>(h);
    return s ? s->value() : 0;
}
void qth_spinbox_set_value(int64_t h, int32_t value) {
    if (auto* s = get<QSpinBox>(h)) s->setValue(value);
}
void qth_spinbox_set_range(int64_t h, int32_t min, int32_t max) {
    if (auto* s = get<QSpinBox>(h)) s->setRange(min, max);
}
void qth_spinbox_set_step(int64_t h, int32_t step) {
    if (auto* s = get<QSpinBox>(h)) s->setSingleStep(step);
}
void qth_spinbox_set_suffix(int64_t h, const char* suffix) {
    if (auto* s = get<QSpinBox>(h)) s->setSuffix(qs(suffix));
}
double qth_dspinbox_get_value(int64_t h) {
    auto* s = get<QDoubleSpinBox>(h);
    return s ? s->value() : 0.0;
}
void qth_dspinbox_set_value(int64_t h, double value) {
    if (auto* s = get<QDoubleSpinBox>(h)) s->setValue(value);
}
void qth_dspinbox_set_range(int64_t h, double min, double max) {
    if (auto* s = get<QDoubleSpinBox>(h)) s->setRange(min, max);
}
void qth_dspinbox_set_step(int64_t h, double step) {
    if (auto* s = get<QDoubleSpinBox>(h)) s->setSingleStep(step);
}

// ── QComboBox ───────────────────────────────────────────────────────────

void qth_combobox_add_item(int64_t h, const char* text) {
    if (auto* c = get<QComboBox>(h)) c->addItem(qs(text));
}
void qth_combobox_clear(int64_t h) {
    if (auto* c = get<QComboBox>(h)) c->clear();
}
int32_t qth_combobox_count(int64_t h) {
    auto* c = get<QComboBox>(h);
    return c ? c->count() : 0;
}
int32_t qth_combobox_get_index(int64_t h) {
    auto* c = get<QComboBox>(h);
    return c ? c->currentIndex() : -1;
}
void qth_combobox_set_index(int64_t h, int32_t index) {
    if (auto* c = get<QComboBox>(h)) c->setCurrentIndex(index);
}
const char* qth_combobox_get_text(int64_t h) {
    auto* c = get<QComboBox>(h);
    return out(c ? c->currentText() : QString());
}
const char* qth_combobox_item_text(int64_t h, int32_t index) {
    auto* c = get<QComboBox>(h);
    return out(c ? c->itemText(index) : QString());
}

// ── QListWidget ─────────────────────────────────────────────────────────

void qth_list_add_item(int64_t h, const char* text) {
    if (auto* l = get<QListWidget>(h)) l->addItem(qs(text));
}
void qth_list_clear(int64_t h) {
    if (auto* l = get<QListWidget>(h)) l->clear();
}
int32_t qth_list_count(int64_t h) {
    auto* l = get<QListWidget>(h);
    return l ? l->count() : 0;
}
int32_t qth_list_get_row(int64_t h) {
    auto* l = get<QListWidget>(h);
    return l ? l->currentRow() : -1;
}
void qth_list_set_row(int64_t h, int32_t row) {
    if (auto* l = get<QListWidget>(h)) l->setCurrentRow(row);
}
const char* qth_list_item_text(int64_t h, int32_t row) {
    auto* l = get<QListWidget>(h);
    auto* it = l ? l->item(row) : nullptr;
    return out(it ? it->text() : QString());
}
void qth_list_remove_row(int64_t h, int32_t row) {
    auto* l = get<QListWidget>(h);
    if (!l) return;
    delete l->takeItem(row);   // takeItem returns nullptr for a bad row; delete nullptr is fine
}

} // extern "C"
