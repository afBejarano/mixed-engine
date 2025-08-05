# Mixed Engine

A graphics programming and game development engine built with Vulkan.

## Dependencies

This project uses the [bootstrapping](https://github.com/corporateshark/bootstrapping) dependency management system to handle external libraries.

### Required Dependencies

- **Vulkan SDK** - Graphics API
- **Python 3** - For bootstrapping script
- **Git** - For cloning repositories
- **CMake 3.30+** - Build system

### External Libraries (Managed by Bootstrap)

- **GLM** - Mathematics library
- **GLFW** - Window management
- **spdlog** - Logging library
- **Dear ImGui** - Immediate mode GUI
- **tiny_obj_loader** - OBJ file loader
- **stb_image** - Image loading
- **RapidXML** - XML parsing
- **yaml-cpp** - YAML parsing and generation

## Quick Setup

**Option 1: Automated Setup (Recommended)**
```bash
git clone <repository-url>
cd mixed-engine
./setup.sh
```

**Option 2: Manual Setup**
```bash
git clone <repository-url>
cd mixed-engine
cd deps && python3 bootstrap.py && cd ..
python3 scripts/copy_headers.py
mkdir build && cd build
cmake .. -G "Unix Makefiles"
make
```

## Project Structure

```
mixed-engine/
├── assets/           # Game assets (models, textures, scenes)
├── cmake/           # CMake modules
├── deps/            # Dependency management
│   ├── bootstrap.py # Bootstrap script (from corporateshark/bootstrapping)
│   └── bootstrap.json # Dependency configuration
├── external/        # External dependencies
│   ├── src/        # Bootstrapped library sources
│   ├── archives/   # Downloaded archives
│   └── *.h         # Header-only libraries
├── scripts/         # Build and utility scripts
│   ├── setup.sh    # Setup script
│   ├── build.sh    # Build script
│   ├── run.sh      # Run script
│   ├── clean.sh    # Clean script
│   └── copy_headers.py # Header copying script
├── src/            # Engine source code
├── shaders/        # GLSL shaders
├── setup.sh        # Main setup script
└── README.md       # This file
```

**Note:** The `external/src/` directory contains bootstrapped dependencies and is not committed to git.

## Build Commands

### Setup (First time only)
```bash
./setup.sh
```

### Build
```bash
./scripts/build.sh
# or
cd build && make
```

### Run
```bash
./scripts/run.sh
# or
cd build && ./MixedEngine
```

### Clean
```bash
./scripts/clean.sh
```

## Adding New Dependencies

To add a new dependency:

1. Edit `deps/bootstrap.json` and add the library configuration
2. Run `cd deps && python3 bootstrap.py && cd ..` to download the new dependency
3. Update `CMakeLists.txt` to include the new library
4. If it's a header-only library, update `scripts/copy_headers.py`

## Building

The project uses CMake as the build system. After running the setup script, you can build normally with CMake.

## Troubleshooting

If you encounter issues with the bootstrap script:

1. Make sure you have Python 3 installed
2. Ensure Git is available in your PATH
3. Check that the repository URLs in `deps/bootstrap.json` are correct
4. Try running `./scripts/clean.sh` followed by `./setup.sh` to force re-download

## License

[Add your license information here] 