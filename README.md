# VU Mister NAM (VST3 starter project)

A JUCE-based Windows VST3/Standalone starter with analog-style input/output VU meters, Input/Output gain, Mix, Bypass, and `.nam` file selection.

## Important capability note

This starter **does not yet run the NAM neural network**. It checks that a selected `.nam` file is JSON with the expected basic `architecture` and `weights` fields, remembers its path in plugin state, and displays a status message. Audio processing currently implements gain/mix and meter display only. Do not treat it as a completed NAM amp modeler.

For actual NAM sound, integrate a supported inference backend such as [NeuralAmpModelerCore](https://github.com/sdatkinson/NeuralAmpModelerCore) or [NeuralAudio](https://github.com/mikeoliphant/NeuralAudio), following that project's current API and licenses. Model loading must happen off the real-time audio thread; swap prepared model instances safely.

## Build locally (Windows 10/11)

Prerequisites:
- Visual Studio 2022 with **Desktop development with C++**
- Windows 10/11 SDK
- Git
- CMake 3.22+ (or use the CMake bundled with Visual Studio)

Open **x64 Native Tools Command Prompt for VS 2022**, then:

```bat
git clone https://github.com/YOUR-USERNAME/VUMisterNAM.git
cd VUMisterNAM
cmake -S . -B build -A x64
cmake --build build --config Release --parallel
```

CMake downloads JUCE 8.0.6 automatically during configure, so the first build needs internet access.

## Find the VST3

Look under:

```text
build\VUMisterNAM_artefacts\Release\VST3\VU Mister NAM.vst3
```

The exact intermediate folder layout can vary slightly with CMake/JUCE versions. Copy the `.vst3` bundle to:

```text
C:\Program Files\Common Files\VST3\
```

Restart/rescan plugins in your DAW. Alternatively, use the generated Standalone app for a basic smoke test.

## Build with GitHub Actions

1. Create a new GitHub repository named `VUMisterNAM`.
2. Upload all files from this project, preserving folder structure.
3. Open **Actions** and enable workflows if GitHub asks.
4. Push a commit to `main`/`master`, or use **Actions → Build Windows VST3 → Run workflow**.
5. Open the completed workflow run and download `VUMisterNAM-Windows-Release` under **Artifacts**.
6. Extract the archive and install the `.vst3` bundle.

The workflow artifact is a build artifact, not a signed installer. Windows may show security warnings for unsigned binaries.

## UI controls

- **INPUT**: input gain in dB.
- **OUTPUT**: output gain in dB.
- **MIX**: dry/wet blend for the starter gain stage.
- **BYPASS**: bypasses the gain stage.
- **LOAD .NAM**: selects a `.nam` file and validates its basic JSON envelope.

## Troubleshooting

- **CMake cannot find a compiler**: install Visual Studio 2022 C++ workload and reopen the x64 Native Tools prompt.
- **JUCE download fails**: check internet/proxy settings and retry configure.
- **No plugin in DAW**: verify the `.vst3` bundle was copied as a folder, restart the DAW, and rescan VST3 plugins.
- **GitHub Actions artifact missing**: inspect the CMake build log first; do not ignore compile errors.
- **NAM file shows validated but does not change tone**: expected in this starter; actual NAM inference is not integrated yet.

## Licenses

JUCE is distributed under its own license terms. Review JUCE licensing for your intended use. The NAM Core/NeuralAudio backend has its own license and dependency requirements; comply with all dependency notices when integrating it.
