# Animations
- Per-window animation overrides
- Ensure animations and blur work together without visual artifacts
- Custom animation shaders for windows (e.g. open/close animations)
- Send to desktop animation?
- Tiled resize animation looks strange with toplevels with blur or rounding

# Effects
- Effects per window state (e.g. unfocused, focused, ...)
- Inner glow effects on borders
- Toplevels with `blur=on` do not render toplevels with blur or mica behind it
- On intel+vulkan, `mica=on` flashes occasionally with the content it should have but is otherwise black, `acrylic=on` looks strange (grid of light grey squares) and `blur=on` shows the toplevels' surface blurred instead of what is behind it blurred.
- Some AMD rendering issues, will fix these after intel ones

# Layout
- Better tab grouping (see sway or Hyprland for reference)
- Better scrolling layout handling (see niri for reference)

# Misc
- Rework the docs to be easier to use
- Improve the README (include video, images, better info)
- Looks like there is a 1px gap between toplevels and borders under certain conditions, likely a rounding error somewhere (observed on zed editor and ghostty on a two column layout with their toplevel on the right)

# Potential
- Per desktop rules (e.g. floating, master_stack)
- Per layer-surface rules
- Focus grab protocol
- Overview/Expose mode from niri
- Plugin system
- Move animations to fully shader-based
