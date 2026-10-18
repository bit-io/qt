#pragma once

#include <QColor>
#include <QFont>
#include <QString>
#include <QWidget>
#include <vector>

namespace qth {

class Canvas : public QWidget {
    Q_OBJECT
public:
    explicit Canvas(int w, int h, QWidget* parent = nullptr);

    enum class Kind { Line, Rect, Ellipse, Text, Point, Image, RoundRect };

    struct Op {
        Kind    kind;
        double  a = 0, b = 0, c = 0, d = 0;   // geometry
        QString text;                         // text / image path
        QColor  pen;
        QColor  fill;                         // invalid = no fill
        double  width = 1;
        QFont   font;
    };

    void clear_ops();
    void set_background(const QColor& c) { background_ = c; update(); }
    void set_pen(const QColor& c, double w) { pen_ = c; pen_width_ = w; }
    void set_fill(const QColor& c) { fill_ = c; }            // invalid = none
    void set_font_spec(const QString& family, int pt, bool bold, bool italic);

    void add(Kind k, double a, double b, double c, double d, const QString& text = {});
    size_t op_count() const { return ops_.size(); }

protected:
    void paintEvent(QPaintEvent*) override;

private:
    std::vector<Op> ops_;
    QColor background_{Qt::white};
    QColor pen_{Qt::black};
    QColor fill_;                // invalid
    double pen_width_ = 1;
    QFont  font_;
};

} // namespace qth
