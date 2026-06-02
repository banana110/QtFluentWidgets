# Angle Controls

## Controls

- `FluentDial` (include: `Fluent/FluentDial.h`)
- `FluentAngleSelector` (include: `Fluent/FluentAngleSelector.h`)

Demo pages: Angle Controls (`demo/pages/PageAngleControls.cpp`) and Overview. The Angle Controls page starts with an Angle State Matrix for side-by-side tick, compact, and disabled checks across Dial and AngleSelector.

## FluentDial

```cpp
#include "Fluent/FluentDial.h"

auto *dial = new Fluent::FluentDial();
dial->setValue(135);
dial->setTicksVisible(true);
dial->setMajorTickStep(45);
```

Purpose: compact circular knob for angle selection, useful for gradient angles, rotation, and direction input.

Angle convention:

- `0°` = left to right (east / 3 o'clock origin)
- values increase clockwise
- the accent arc starts from the 3 o'clock position

Key APIs:

- `setValue(int)` / `value()`: current angle value
- `setTicksVisible(bool)` / `ticksVisible()`: show tick marks
- `setTickStep(int)` / `tickStep()`: minor tick step, default `15°`
- `setMajorTickStep(int)` / `majorTickStep()`: major tick step, default `45°`
- `setPointerVisible(bool)` / `pointerVisible()`: show the pointer
- `hoverLevel()` / `focusLevel()`: read-only animation levels, useful for tests or diagnostics

Implementation notes:

- Custom-painted `QWidget` whose hover/focus animations use `FluentMotionRole::Hover` / `Focus`; disabling global animations or setting the corresponding duration to 0 snaps the feedback to its target state.
- `setValue(int)` normalizes to `0..359`; when the value changes, it emits `valueChanged(int)`.
- Mouse dragging converts cursor position into an angle value; the mouse wheel can be used for fine adjustment.
- Disabled state mutes the accent arc, pointer, ticks, and indicator dot so it does not read as interactive.
- It can draw the outer track, current accent arc, optional ticks / major ticks, optional pointer, indicator dot, and focus ring.

## FluentAngleSelector

```cpp
#include "Fluent/FluentAngleSelector.h"

auto *editor = new Fluent::FluentAngleSelector();
editor->setValue(128);
editor->setLabelText(QStringLiteral("Angle:"));
editor->setSuffix(QStringLiteral("°"));
```

Purpose: integrated angle editor built from `FluentLabel + FluentDial + FluentSpinBox`, exposing a single `valueChanged(int)` semantic.

Key APIs:

- `setValue(int)` / `value()`
- `setRange(int minimum, int maximum)`: default `0..359`
- `setWrapping(bool)` / `wrapping()`: default true
- `setLabelText(const QString&)` / `labelText()`
- `setSuffix(const QString&)` / `suffix()`
- `setDialVisible(bool)` / `dialVisible()`
- `setLabelVisible(bool)` / `labelVisible()`
- `setSpinBoxVisible(bool)` / `spinBoxVisible()`
- `dial()` / `spinBox()` for advanced customization of the internal child widgets

Implementation notes:

- Internally uses `FluentDial + FluentSpinBox + FluentLabel`.
- Dial and spin box stay synchronized, with temporary `blockSignals(true)` guards to avoid signal loops.
- `setValue(int)` emits `valueChanged(int)` when the normalized value actually changes, matching child-widget interaction semantics.
- In wrapping mode, values are normalized back into range; in non-wrapping mode they are clamped.
- Visibility toggles make it easy to create compact variants such as dial-only, spinbox-only, or label-less forms.

Typical use cases:

- gradient angle editing in `FluentColorDialog`
- object / canvas rotation controls
- any workflow that wants both a numeric value and a visual knob
