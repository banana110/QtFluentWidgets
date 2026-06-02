# AnimatedIcon Integration Plan

This document tracks the Lottie / AnimatedIcon integration boundary, shipped behavior, and remaining polish for the visual-quality phase. API details live in [lottie.md](lottie.md) and [buttons.md](buttons.md).

## Shipped

- `FluentLottieWidget` provides Lottie JSON loading, playback control, marker lookup, fallback icons, and whole-frame `tintColor` recoloring.
- `FluentAnimatedIcon` adds WinUI-style state semantics on top of Lottie: `Normal`, `PointerOver`, `Pressed`, `Disabled`, `Selected`, plus `_Start` / `_End` transition markers.
- `FluentAnimatedButton` synchronizes its internal animated icon to the button foreground: Primary uses `onAccent`, neutral buttons use the current text color, and disabled buttons use `disabledText`.
- `FluentCard` can load an animated collapse indicator, and `clearCollapseIndicatorAnimation()` returns to the static chevron.
- `FluentNavigationView` supports animated pane-toggle, back-button, and item icons, with clear APIs that return to static glyphs.
- Public headers expose Qt types only, not rlottie types, so a future rendering backend change should not break application code.

## Design Contract

- Motion must always have a static fallback. If Lottie loading fails, an animation is cleared, or motion is disabled, the control must remain readable.
- Animation color follows the control's semantic foreground color instead of preserving fixed source-asset colors.
- Every playback path respects global motion / reduced-motion settings. With motion disabled, controls snap to the target state frame instead of continuing to play.
- Markers are the semantic contract. Assets should provide state markers and paired transition markers; with single-point markers only, the control jumps to the state frame.
- Integrations should use animated assets only when the motion clearly matches the control meaning. A static glyph is better than a generic animation.

## Next Polish

- Expand the built-in set of small, semantically clear Lottie assets while keeping static glyph fallbacks aligned.
- Keep animated entry points limited to controls where motion improves readability, rather than animating ordinary form controls by default.
- Keep expanding cache stress coverage for high DPI, rapid resize, and theme switching. `lottieTintCacheSurvivesResizeAndThemeChanges` now covers tinted frames not reusing stale sizes or theme colors, and CTest reruns that case with `QT_SCALE_FACTOR=1.5`.
- Consider opt-in integration points beyond NavigationView and Card when suitable assets exist, while keeping the default behavior stable and readable.

## Regression Coverage

The current smoke tests use `QT_QPA_PLATFORM=offscreen` and `QWidget::render()` for visual verification:

- `lottieMetadataAndPlaybackApiFollowDocumentation`
- `lottieFallbackIconUsesTintColor`
- `lottieTintCacheSurvivesResizeAndThemeChanges`
- `animatedIconMarkerResolutionFollowsDocumentation`
- `buttonAnimatedIconsFollowForegroundTokens`
- `cardCollapseIndicatorUsesStateMarkersAndClearFallback`
- `navigationChromeAnimationsClearToStaticIcons`
