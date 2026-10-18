#pragma once

#include <QObject>

namespace qth {

class InputFilter : public QObject {
    Q_OBJECT
public:
    enum Mask { Keys = 1, MouseButtons = 2, MouseMove = 4, Wheel = 8, Resize = 16 };

    // Owned by (child of) `target`, so it dies with it.
    InputFilter(QObject* target, int mask);

    void set_mask(int mask) { mask_ = mask; }
    int mask() const { return mask_; }

    // Existing filter of `target`, or nullptr.
    static InputFilter* of(QObject* target);

protected:
    bool eventFilter(QObject* watched, QEvent* ev) override;

private:
    int mask_;
};

} // namespace qth
