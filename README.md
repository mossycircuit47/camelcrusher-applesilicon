# CamelCrusher Rebuilt

Native Apple Silicon reconstruction of CamelCrusher 1.x for macOS 11 or later.
Includes AU, VST3, and standalone targets, the original 345 × 373 skin, 20 factory
presets, automation, MIDI CC learn in VST3, and legacy FXP/FXB import and export.
This is a local reconstructed implementation, not an official Camel Audio release.

## Build

Requires Xcode command-line tools, CMake 3.22+, and JUCE 8 (tested with 8.0.12).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DJUCE_ROOT=/path/to/JUCE
cmake --build build --config Release --target CamelCrusherRebuilt_AU CamelCrusherRebuilt_VST3 CamelCrusherRebuilt_Standalone -j 6
```

Outputs are in `build/CamelCrusherRebuilt_artefacts/Release/`. The original plugin
and its installed data folder are not needed to build or run the reconstruction.
Skin resources and factory data are bundled for this local rebuild.

## Controls

Drag knobs vertically; hold Shift for fine adjustments. Double-click restores the
clear-preset value. Scroll adjusts knobs. Right-click a control for MIDI Learn,
clear mapping, or linear/circular knob mode. MIDI mappings persist in host state.
The AU is an audio effect with host automation; VST3 accepts MIDI CC input.

Use the arrows beside the patch name to change presets; click the name for the
preset menu, factory bank, clear, and FXP/FXB loading and saving. Random changes
only enabled distortion, filter, and compressor sections, as in the original.
The random generator is native and does not reproduce the original PRNG sequence.

## Installation

Copy `CamelCrusher Rebuilt.component` into
`~/Library/Audio/Plug-Ins/Components/` and `CamelCrusher Rebuilt.vst3` into
`~/Library/Audio/Plug-Ins/VST3/`, then restart your music app and rescan plugins.
The standalone app can be copied into Applications.

The rebuild has a separate plugin identity. To migrate presets from the original,
export FXP/FXB files and load them through the rebuilt patch menu.

Audio comparisons against the Intel original covered all factory presets,
parameter changes, isolated effects, bypass, several block sizes, and three
sample rates. Small cross-architecture floating-point differences remain.
