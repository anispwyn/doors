# Animations
- Per-window animation overrides
- Ensure animations and blur work together without visual artifacts
- Custom animation shaders for windows (e.g. open/close animations)
- Send to desktop animation?
- Resize animation still looks buggy
- During desktop switch animation, rounded corner masks are not present or move unexpectedly (looks like their surface is "scaled", they move from the top left corner ntil they reach their final position)
- During desktop switch animation, if the desktop being switched from contains a toplevel with rounded borders + mica (may also happen on just rounded borders or just mica, unsure), its surface is replaced with a downsampled screen capture

# Effects
- Effects per window state (e.g. unfocused, focused, ...)
- Inner glow effects on borders
- Toplevels with `blur=on` do not render toplevels with blur or mica behind it
- On intel+vulkan, `mica=on` flashes occasionally with the content it should have but is otherwise black, `acrylic=on` looks strange (grid of light grey squares) and `blur=on` shows the toplevels' surface blurred instead of what is behind it blurred.
- Some AMD rendering issues, will fix these after intel ones

# Layout
- Better tab grouping (see sway or Hyprland for reference)
- Better scrolling layout handling (see niri for reference)
- Fullscreening XWayland clients while using a fractional scale causes them not to take up the full screen (black on right right and bottom)
- Some toplevels open smaller in height and then take their correct size a frame later
- Layout isn't rearranged on output scale change

# Misc
- Rework the docs to be easier to use
- Improve the README (include video, images, better info)
- Looks like there is a 1px gap between toplevels and borders under certain conditions, likely a rounding error somewhere (observed on zed editor and ghostty on a two column layout with their toplevel on the right)
- Interactive resize is not smooth, especially in tiled layout

# Potential
- Per desktop rules (e.g. floating, master_stack)
- Per layer-surface rules
- Focus grab protocol
- Overview/Expose mode from niri
- Plugin system
- Move animations to fully shader-based
