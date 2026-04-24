How To Remove The Shadow Around macOS Window Captures
=====================================================

The macOS `Screenshot` application has the ability to capture a screenshot of a
specific window, either by selecting the `Capture Selected Window` option
directly from within the `Screenshot` application or by pressing
`Command`+`Shift`+`4` followed by pressing `Space`. Unfortunately, these
captures produce an image with an ugly shadow effect around the selected
window:

{{Image src="/blog/2026-02-19-how-to-set-macos-dock-size-from-the-cli/window-capture-with-shadow.png"}}
A window capture of Firefox on the Space Jam (1996) website. The window capture
has a shadow effect around all sides of the window.
{{/Image}}

It is somewhat difficult to see, because the resulting PNG file has a
transparent background. The shadow effect is easier to see if we place the
image on a white background.

<div style="background-color: white;">
{{Image src="/blog/2026-02-19-how-to-set-macos-dock-size-from-the-cli/window-capture-with-shadow.png"}}
The same window capture of Firefox on the Space Jam (1996) website as above,
now displaying the image on a white background <div> element to more clearly
show the shadow effect.
{{/Image}}
</div>

To disable this shadow effect, open a terminal and execute the following:

```sh
defaults write com.apple.screencapture "disable-shadow" -bool "true"
```

After executing that command, window captures should not longer have the shadow
effect:

{{Image src="/blog/2026-02-19-how-to-set-macos-dock-size-from-the-cli/window-capture-without-shadow.png"}}
Another window capture of Firefox on the Space Jam (1996) website. This image
does not have the same shadow around the edge of the window that was seen in
the previous two images.
{{/Image}}

As a bonus, disabling the shadow effect will also remove of the huge sections
of empty transparent space around the window! Ahh, much nicer looking. 😊

To re-enable the shadow effect, open a terminal and execute the following:

```sh
defaults write com.apple.screencapture "disable-shadow" -bool "false"
```
