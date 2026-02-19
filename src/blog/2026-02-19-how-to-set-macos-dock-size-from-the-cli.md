How To Set The macOS Dock Size From The CLI
===========================================

Open a terminal and execute the following, replacing 64 with your desired dock
size:

```sh
defaults write com.apple.dock tilesize -int 64; killall Dock
```

To lock this dock size, execute the following:

```sh
defaults write com.apple.dock size-immutable -bool yes
```

And to unlock this dock size, execute the following:

```sh
defaults write com.apple.dock size-immutable -bool no
```

All together:

```sh
defaults write com.apple.dock size-immutable -bool no
defaults write com.apple.dock tilesize -int 64
killall Dock
defaults write com.apple.dock size-immutable -bool yes
```
