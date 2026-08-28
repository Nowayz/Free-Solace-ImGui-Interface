# Solace

An offline interface showcase built with C++20 and Dear ImGui. Solace runs either as a
native Windows application using DirectX 11 or as a browser SPA compiled to WebAssembly and
rendered with WebGL2.
The sign-in screens and provider buttons are local demonstrations; Solace makes no network
requests and does not collect or submit credentials.

[![Windows build](https://github.com/poncippg-spec/Free-Solace-ImGui-Interface/actions/workflows/build.yml/badge.svg)](https://github.com/poncippg-spec/Free-Solace-ImGui-Interface/actions/workflows/build.yml)
[Download the latest release](https://github.com/poncippg-spec/Free-Solace-ImGui-Interface/releases/latest)

![Solace interface](docs/img/menu.png)

## Preview

### YouTube demo

[![Watch the Solace demo on YouTube](https://img.youtube.com/vi/R2SeWPtzg3g/maxresdefault.jpg)](https://www.youtube.com/watch?v=R2SeWPtzg3g)

[Watch on YouTube](https://www.youtube.com/watch?v=R2SeWPtzg3g)

### Sign-up screens

<p align="center">
  <img src="docs/media/signup-light.png" alt="Solace sign-up screen in light mode" width="49%">
  <img src="docs/media/signup-dark.png" alt="Solace sign-up screen in dark mode" width="49%">
</p>

The light capture uses the Pondot credit profile; the dark capture demonstrates the
swappable placeholder profile.

### Interface recording

[![Watch the Solace interface recording](docs/media/video-preview.jpg)](https://github.com/poncippg-spec/Free-Solace-ImGui-Interface/releases/download/v0.1.0/solace-demo.mp4)

[Watch the full 35-second recording](https://github.com/poncippg-spec/Free-Solace-ImGui-Interface/releases/download/v0.1.0/solace-demo.mp4)

## Features

- Frameless Win32 window
- Responsive browser shell with a WebGL2-only canvas
- WebAssembly build powered by Emscripten, SDL2, and OpenGL ES 3
- Sign-up and sign-in screens
- Animated sidebar and page transitions
- Reusable inputs, menus, tabs, switches, and sliders
- Light and dark themes
- DPI-aware text and layout

## Download

Download `Solace-0.1.0-win64.zip` from the
[latest release](https://github.com/poncippg-spec/Free-Solace-ImGui-Interface/releases/latest), extract
the complete archive, and run `Solace.exe`. The release includes the required assets,
licenses, and a SHA-256 checksum file.

The executable is currently unsigned, so Windows may display an unknown-publisher warning.
Build from source if you prefer not to run the packaged binary.

## Build for Windows

Requirements:

- Windows 10 or 11
- Visual Studio 2022
- Desktop development with C++

From PowerShell:

```powershell
git clone https://github.com/poncippg-spec/Free-Solace-ImGui-Interface.git
cd Free-Solace-ImGui-Interface
.\scripts\build.ps1 -Configuration Release
```

The executable is written to:

```text
Release/Solace.exe
```

For a debug build:

```powershell
.\scripts\build.ps1 -Configuration Debug
```

You can also open `Solace.sln` in Visual Studio.

## Build for the web

Requirements:

- Emscripten 6.0.8 or newer
- CMake 3.24 or GNU Make

Activate an Emscripten SDK, then run:

```bash
./scripts/build-web.sh
```

The static SPA is written to:

```text
build/web/dist/index.html
```

Serve that directory over HTTP; browsers do not allow the `.wasm` and `.data` files to load
reliably from a `file://` URL. For example:

```bash
python3 -m http.server 8080 --directory build/web/dist
```

Then open `http://localhost:8080`. The web target requires WebGL2 and intentionally has no
WebGL1 fallback. The checked-in Pages workflow builds pull requests and publishes `main`.

### Browser architecture

- `src/runtime/web_app.cpp` owns the SDL2, OpenGL ES 3, and Emscripten frame loop.
- `src/assets/*_web.cpp` uploads the same bundled images as WebGL textures.
- DirectX-only shader effects use browser-safe ImGui drawing fallbacks.
- The browser shell provides responsive scaling, loading and error states, fullscreen mode,
  touch-safe canvas behavior, and persisted light/dark preference.
- All authentication screens remain local demonstrations. No credentials or form data leave
  the browser.

## Controls

- Drag an empty part of the window to move it.
- Press `Ctrl+B` to collapse the sidebar.
- Press `F` to open search.
- Press `Escape` to close the active panel or exit.
- In the browser, use the corner button to enter fullscreen. `Escape` exits fullscreen through
  the browser as usual.

## Notes

Keep the `assets` directory with the executable when packaging the interface. Media
redistribution and provider-mark terms are documented in `THIRD_PARTY_NOTICES.md`.

## Project layout

```text
src/          application source
assets/       runtime images
scripts/      build and verification tools
thirdparty/   bundled dependencies
web/          SPA shell
```

## Credits

Created by [Pondot](https://github.com/poncippg-spec).

Third-party licenses and asset notes are listed in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

MIT License. See [LICENSE](LICENSE).
