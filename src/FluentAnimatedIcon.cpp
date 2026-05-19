#include "Fluent/FluentAnimatedIcon.h"

#include <QEvent>
#include <QMouseEvent>

namespace Fluent {

namespace {

QString normalizedState(QString state)
{
    state = state.trimmed();
    return state.isEmpty() ? QStringLiteral("Normal") : state;
}

bool isFrameName(const QString &state, int *frame)
{
    bool ok = false;
    const int value = state.toInt(&ok);
    if (ok && frame) {
        *frame = value;
    }
    return ok;
}

} // namespace

FluentAnimatedIcon::FluentAnimatedIcon(QWidget *parent)
    : FluentLottieWidget(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    setFallbackIconSize(QSize(28, 28));
}

QString FluentAnimatedIcon::state() const
{
    return m_state;
}

void FluentAnimatedIcon::setState(const QString &state)
{
    setState(state, true);
}

void FluentAnimatedIcon::setState(const QString &state, bool animated)
{
    const QString next = normalizedState(state);
    const QString previous = m_state;

    if (previous == next) {
        return;
    }

    m_state = next;
    emit stateChanged(m_state);

    if (!isLoaded()) {
        return;
    }

    if (animated && playResolvedTransition(previous, next)) {
        return;
    }

    pause();
    setCurrentFrame(resolvedStateFrame(previous, next));
}

bool FluentAnimatedIcon::isInteractive() const
{
    return m_interactive;
}

void FluentAnimatedIcon::setInteractive(bool interactive)
{
    m_interactive = interactive;
}

void FluentAnimatedIcon::enterEvent(FluentEnterEvent *event)
{
    FluentLottieWidget::enterEvent(event);
    if (m_interactive && isEnabled()) {
        setState(QStringLiteral("PointerOver"), true);
    }
}

void FluentAnimatedIcon::leaveEvent(QEvent *event)
{
    FluentLottieWidget::leaveEvent(event);
    if (m_interactive && isEnabled()) {
        setState(QStringLiteral("Normal"), true);
    }
}

void FluentAnimatedIcon::mousePressEvent(QMouseEvent *event)
{
    if (m_interactive && isEnabled() && event->button() == Qt::LeftButton) {
        setState(QStringLiteral("Pressed"), true);
    }
    FluentLottieWidget::mousePressEvent(event);
}

void FluentAnimatedIcon::mouseReleaseEvent(QMouseEvent *event)
{
    FluentLottieWidget::mouseReleaseEvent(event);
    if (m_interactive && isEnabled() && event->button() == Qt::LeftButton) {
        setState(rect().contains(mousePositionF(event).toPoint())
                     ? QStringLiteral("PointerOver")
                     : QStringLiteral("Normal"),
                 true);
    }
}

void FluentAnimatedIcon::changeEvent(QEvent *event)
{
    FluentLottieWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        setState(isEnabled() ? QStringLiteral("Normal") : QStringLiteral("Disabled"), true);
    }
}

int FluentAnimatedIcon::resolvedStateFrame(const QString &previousState, const QString &nextState) const
{
    const QString transition = previousState + QStringLiteral("To") + nextState;

    const QString transitionEnd = transition + QStringLiteral("_End");
    if (hasMarker(transitionEnd)) {
        return markerFrame(transitionEnd);
    }

    const QString transitionStart = transition + QStringLiteral("_Start");
    if (hasMarker(transitionStart)) {
        return markerFrame(transitionStart);
    }

    if (hasMarker(transition)) {
        return markerFrame(transition);
    }

    if (hasMarker(nextState)) {
        return markerFrame(nextState);
    }

    const QString suffix = QStringLiteral("To") + nextState + QStringLiteral("_End");
    const QStringList names = markerNames();
    for (const QString &name : names) {
        if (name.endsWith(suffix, Qt::CaseInsensitive)) {
            return markerFrame(name);
        }
    }

    int frame = 0;
    if (isFrameName(nextState, &frame)) {
        return frame;
    }

    return 0;
}

bool FluentAnimatedIcon::playResolvedTransition(const QString &previousState, const QString &nextState)
{
    const QString transition = previousState + QStringLiteral("To") + nextState;
    const QString transitionStart = transition + QStringLiteral("_Start");
    const QString transitionEnd = transition + QStringLiteral("_End");

    if (hasMarker(transitionStart) && hasMarker(transitionEnd)) {
        const int start = markerFrame(transitionStart);
        const int end = markerFrame(transitionEnd);
        if (start != end) {
            playSegment(start, end);
            return true;
        }
    }

    if (hasMarker(transition)) {
        playMarker(transition);
        return true;
    }

    return false;
}

} // namespace Fluent
