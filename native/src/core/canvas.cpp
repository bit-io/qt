#include "canvas.hpp"

#include <QHash>
#include <QPainter>
#include <QPixmap>

namespace qth {

namespace {
// Loaded images are cached by path so a game loop does not hit the disk each frame.
const QPixmap& cached_pixmap(const QString& path) {
    static QHash<QString, QPixmap> cache;
    auto it = cache.find(path);
    if (it == cache.end()) it = cache.insert(path, QPixmap(path));
    return it.value();
}
} // namespace

Canvas::Canvas(int w, int h, QWidget* parent) : QWidget(parent) {
    setMinimumSize(w > 0 ? w : 1, h > 0 ? h : 1);
    resize(w > 0 ? w : 1, h > 0 ? h : 1);
    font_ = QWidget::font();
}

void Canvas::clear_ops() {
    ops_.clear();
    update();
}

void Canvas::set_font_spec(const QString& family, int pt, bool bold, bool italic) {
    if (!family.isEmpty()) font_.setFamily(family);
    if (pt > 0) font_.setPointSize(pt);
    font_.setBold(bold);
    font_.setItalic(italic);
}

void Canvas::add(Kind k, double a, double b, double c, double d, const QString& text) {
    Op op;
    op.kind = k; op.a = a; op.b = b; op.c = c; op.d = d; op.text = text;
    op.pen = pen_; op.fill = fill_; op.width = pen_width_; op.font = font_;
    ops_.push_back(std::move(op));
    if (ops_.size() > 200000) ops_.erase(ops_.begin(), ops_.begin() + 100000);   // runaway guard
    update();
}

void Canvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), background_);

    for (const Op& op : ops_) {
        QPen pen(op.pen);
        pen.setWidthF(op.width);
        p.setPen(pen);
        p.setBrush(op.fill.isValid() ? QBrush(op.fill) : QBrush(Qt::NoBrush));
        switch (op.kind) {
        case Kind::Line:      p.drawLine(QLineF(op.a, op.b, op.c, op.d)); break;
        case Kind::Rect:      p.drawRect(QRectF(op.a, op.b, op.c, op.d)); break;
        case Kind::RoundRect: p.drawRoundedRect(QRectF(op.a, op.b, op.c, op.d), 8, 8); break;
        case Kind::Ellipse:   p.drawEllipse(QRectF(op.a, op.b, op.c, op.d)); break;
        case Kind::Point:     p.drawPoint(QPointF(op.a, op.b)); break;
        case Kind::Text:
            p.setFont(op.font);
            p.drawText(QPointF(op.a, op.b), op.text);
            break;
        case Kind::Image: {
            const QPixmap& pm = cached_pixmap(op.text);
            if (pm.isNull()) break;
            if (op.c > 0 && op.d > 0) p.drawPixmap(QRectF(op.a, op.b, op.c, op.d), pm, pm.rect());
            else p.drawPixmap(QPointF(op.a, op.b), pm);
            break;
        }
        }
    }
}

} // namespace qth
