# Seam Carver - Content-Aware Image Resizing

A professional C++ implementation of the Seam Carving algorithm with face protection support.

## Table of Contents

- [Features](#features)
- [Building](#building)
  - [Prerequisites](#prerequisites)
  - [Install Dependencies](#install-dependencies-macos)
  - [Compile](#compile)
- [Usage](#usage)
  - [Basic Resizing](#basic-resizing)
  - [Protecting Faces](#protecting-faces-recommended-for-portraits)
  - [Advanced Options](#advanced-options)
- [Comparison & Analysis Tools](#comparison--analysis-tools)
  - [Automated Comparison Script](#automated-comparison-script)
  - [Manual Visual Comparison](#manual-visual-comparison)
  - [Analysis Reports](#analysis-reports)
- [How It Works](#how-it-works)
  - [Seam Carving Algorithm](#seam-carving-algorithm)
  - [Face Protection](#face-protection)
- [Files](#files)
- [Troubleshooting](#troubleshooting)
- [Examples](#examples)
- [Performance](#performance)
- [Algorithm Details](#algorithm-details)
- [Comparison Results](#comparison-results)
- [Command Reference](#command-reference)
- [Credits](#credits)

---

## Features

- **Content-Aware Resizing**: Intelligently removes or adds seams based on image energy
- **Face Protection**: Automatically detect and protect faces from distortion using Haar Cascade
- **Advanced Energy Function**: Uses Sobel gradient magnitude (5x5 kernel) for better edge preservation
- **Automated Comparison**: Compare results with and without face protection
- **Visual Analysis**: Generate side-by-side comparison images
- **Comprehensive Reports**: Automatic generation of technical analysis reports
- **Flexible CLI**: Easy command-line interface for quick resizing

## Building

### Prerequisites

- C++17 compiler (g++ or clang++)
- OpenCV 4.x
- pkg-config

### Install Dependencies (macOS)

```bash
brew install opencv pkg-config
```

### Compile

```bash
# Build all tools
g++ -std=c++17 -o seam_carver seam_carver.cpp `pkg-config --cflags --libs opencv4`
g++ -std=c++17 -o create_face_mask create_face_mask.cpp `pkg-config --cflags --libs opencv4`
g++ -std=c++17 -o visualize_comparison visualize_comparison.cpp `pkg-config --cflags --libs opencv4`

# Or build individually
g++ -std=c++17 -o seam_carver seam_carver.cpp `pkg-config --cflags --libs opencv4`
g++ -std=c++17 -o create_face_mask create_face_mask.cpp `pkg-config --cflags --libs opencv4`
g++ -std=c++17 -o visualize_comparison visualize_comparison.cpp `pkg-config --cflags --libs opencv4`
```

Or use the VS Code build task (Cmd+Shift+B).

## Usage

### Basic Resizing

```bash
# Resize to specific dimensions
./seam_carver -i=input.jpg -o=output.jpg -w=800 -h=600

# Resize width only (keep original height)
./seam_carver -i=input.jpg -o=output.jpg -w=800

# Resize height only (keep original width)
./seam_carver -i=input.jpg -o=output.jpg --height=600
```

### Protecting Faces (Recommended for portraits)

**Step 1: Create a face protection mask**

```bash
./create_face_mask input.jpg face_mask.png
```

**Step 2: Resize with face protection**

```bash
./seam_carver -i=input.jpg -o=output.jpg -w=800 -h=600 --protect=face_mask.png
```

### Advanced Options

```bash
# Show result in a window
./seam_carver -i=input.jpg -o=output.jpg -w=800 --show

# Remove an object (provide removal mask)
./seam_carver -i=input.jpg -o=output.jpg -w=800 --remove=object_mask.png

# Get help
./seam_carver --help
```

## Comparison & Analysis Tools

### Automated Comparison Script

Run a complete comparison that generates results with and without face protection:

```bash
# Run automated comparison
./compare_results.sh input.jpg 300 300 comparison

# This generates:
# - comparison_face_mask.png (detected face regions)
# - comparison_no_protection.jpg (without face protection)
# - comparison_with_protection.jpg (with face protection)
# - comparison_side_by_side.jpg (visual comparison image)
```

### Manual Visual Comparison

Create a side-by-side comparison image manually:

```bash
./visualize_comparison original.jpg without_protection.jpg with_protection.jpg output_comparison.jpg
```

This creates a labeled, three-panel comparison showing:

- Original image (resized for comparison)
- Result without protection
- Result with protection

### Analysis Reports

The project includes comprehensive documentation:

- **COMPARISON_REPORT.md** - Detailed technical analysis with:
  - Methodology and algorithm configuration
  - Face detection results and metrics
  - Visual and quantitative assessment
  - Technical behavior analysis
  - Best practices and recommendations
- **COMPARISON_SUMMARY.md** - Quick reference with:
  - Key statistics and findings
  - File descriptions
  - Reproduction steps
  - Conclusion and recommendations

## How It Works

### Seam Carving Algorithm

1. Calculate energy map using Sobel gradient magnitude
2. Find lowest-energy vertical/horizontal seam using dynamic programming
3. Remove or duplicate seam to shrink/expand image
4. Repeat until target dimensions reached

### Face Protection

- The `create_face_mask` utility uses OpenCV's Haar Cascade classifier to detect faces
- Detected face regions are marked with maximum energy
- The seam carving algorithm avoids removing seams through high-energy (protected) regions
- Face areas are expanded by 20% to protect surrounding context

## Files

### Core Implementation

- `seam_carver.cpp` - Main seam carving implementation with class-based design
- `seam_carver` - Compiled executable

### Utilities

- `create_face_mask.cpp` - Face detection and mask generation utility
- `create_face_mask` - Compiled face detection tool
- `visualize_comparison.cpp` - Side-by-side comparison image generator
- `visualize_comparison` - Compiled visualization tool
- `compare_results.sh` - Automated comparison script (generates all outputs)

### Documentation

- `README.md` - This file (complete project documentation)
- `COMPARISON_REPORT.md` - Detailed technical analysis and findings
- `COMPARISON_SUMMARY.md` - Quick reference for comparison results

### VS Code Configuration

- `.vscode/c_cpp_properties.json` - IntelliSense configuration with OpenCV paths
- `.vscode/tasks.json` - Build tasks for compilation

### Example Outputs (generated)

- `*_face_mask.png` - Binary protection masks
- `*_no_protection.jpg` - Results without face protection
- `*_with_protection.jpg` - Results with face protection
- `*_side_by_side.jpg` - Visual comparison images

## Troubleshooting

### "Cannot find opencv2/opencv.hpp"

Ensure OpenCV is installed and pkg-config can find it:

```bash
pkg-config --exists opencv4 && echo "Found" || echo "Not found"
pkg-config --cflags opencv4
```

If not found, install OpenCV:

```bash
brew install opencv
```

### Face Detection Not Working

The face detection requires Haar Cascade XML files. These are typically installed with OpenCV at:

- `/opt/homebrew/share/opencv4/haarcascades/` (Homebrew Apple Silicon)
- `/usr/local/share/opencv4/haarcascades/` (Homebrew Intel)

Verify the file exists:

```bash
ls /opt/homebrew/share/opencv4/haarcascades/haarcascade_frontalface_default.xml
```

### Distorted Results

- **For images with faces**: Always use the `--protect` option with a face mask
- **For general images**: The improved gradient magnitude energy function should work well
- **For specific objects**: Create a custom protection mask (white areas are protected)
- **For extreme reductions**: Use the comparison script to verify quality

### Build Issues

If you get compilation errors:

```bash
# Verify pkg-config can find OpenCV
pkg-config --modversion opencv4

# Check include paths
pkg-config --cflags opencv4

# Check library paths
pkg-config --libs opencv4

# Rebuild with verbose output
g++ -std=c++17 -v -o seam_carver seam_carver.cpp `pkg-config --cflags --libs opencv4`
```

## Command Reference

### seam_carver

```
Usage: seam_carver [params] input output

Options:
  -?, --help, --usage     Print help message
  -h, --height           Target height (default: original height)
  -w, --width            Target width (default: original width)
  -p, --protect          Path to protection mask (optional)
  -r, --remove           Path to removal mask (optional)
  -s, --show             Show result in window (optional)

  input                  Path to input image (required)
  output                 Path to output image (required)
```

### create_face_mask

```
Usage: create_face_mask <input_image> <output_mask>

Example:
  ./create_face_mask portrait.jpg face_mask.png
```

### visualize_comparison

```
Usage: visualize_comparison <original> <no_protection> <with_protection> <output>

Example:
  ./visualize_comparison img.jpg no_protect.jpg with_protect.jpg comparison.jpg
```

### compare_results.sh

```
Usage: ./compare_results.sh <input_image> <target_width> <target_height> <output_prefix>

Example:
  ./compare_results.sh portrait.jpg 300 300 result

Generates:
  - result_face_mask.png
  - result_no_protection.jpg
  - result_with_protection.jpg
  - result_side_by_side.jpg
```

## Examples

### Quick Start

```bash
# 1. Build all tools
g++ -std=c++17 -o seam_carver seam_carver.cpp `pkg-config --cflags --libs opencv4`
g++ -std=c++17 -o create_face_mask create_face_mask.cpp `pkg-config --cflags --libs opencv4`
g++ -std=c++17 -o visualize_comparison visualize_comparison.cpp `pkg-config --cflags --libs opencv4`

# 2. Run automated comparison (recommended for first-time users)
./compare_results.sh portrait.jpg 300 300 my_result

# 3. View the side-by-side comparison
open my_result_side_by_side.jpg
```

### Manual Workflow

```bash
# Step 1: Detect faces and create protection mask
./create_face_mask portrait.jpg face_mask.png

# Step 2: Resize WITHOUT protection (for comparison)
./seam_carver -i=portrait.jpg -o=no_protection.jpg -w=300 -h=300

# Step 3: Resize WITH protection (recommended)
./seam_carver -i=portrait.jpg -o=protected.jpg -w=300 -h=300 --protect=face_mask.png

# Step 4: Create visual comparison
./visualize_comparison portrait.jpg no_protection.jpg protected.jpg comparison.jpg
```

### Real-World Use Cases

```bash
# Profile picture resize (preserve face)
./create_face_mask selfie.jpg face_mask.png
./seam_carver -i=selfie.jpg -o=profile_pic.jpg -w=400 -h=400 --protect=face_mask.png

# Thumbnail generation
./seam_carver -i=photo.jpg -o=thumbnail.jpg -w=150 -h=150

# Aspect ratio change (16:9 to 4:3)
./seam_carver -i=landscape.jpg -o=standard.jpg -w=1024 --height=768

# Social media resize with face protection
./create_face_mask group_photo.jpg faces.png
./seam_carver -i=group_photo.jpg -o=instagram.jpg -w=1080 -h=1080 --protect=faces.png
```

## Performance

- Processing time depends on:
  - Original image dimensions
  - Number of seams to remove/add
  - Mask complexity
- Typical performance: ~1-2 seconds per 100 seams on modern hardware
- Example: 1200×1600 → 300×300 (2,200 seams) takes ~2-3 seconds

### Performance Tips

- Use moderate reductions (< 50%) for best quality
- For extreme reductions, consider multiple passes or hybrid approaches
- Face detection adds minimal overhead (~0.1-0.5 seconds)
- Seam insertion (expansion) is slower than removal

## Algorithm Details

### Energy Function

```
E(x,y) = √(Gx² + Gy²)
```

Where:

- `Gx` = Sobel gradient in x-direction (5×5 kernel)
- `Gy` = Sobel gradient in y-direction (5×5 kernel)
- Normalized to 0-255 range for better contrast

### Protected Regions

Protected pixels are assigned maximum energy (1e9), ensuring seams avoid them:

- Face regions (detected via Haar Cascade)
- Custom masks (white pixels = protected)
- Expanded by 20% margin for context preservation

### Seam Selection

- Uses dynamic programming for optimal seam finding
- Considers 3 parent pixels (left, center, right)
- Minimizes cumulative energy along path
- O(width × height) time complexity per seam

## Comparison Results

The included comparison analysis demonstrates:

- **75% width reduction** (1200 → 300 pixels)
- **81% height reduction** (1600 → 300 pixels)
- **Without protection**: Severe facial distortion, unusable
- **With protection**: Natural appearance, preserved features

See **COMPARISON_REPORT.md** for detailed analysis.

## Credits

Based on the Seam Carving algorithm by Avidan & Shamir (2007).
Implementation by Utkarsh Sachan with face protection enhancements.

### References

- Avidan, S., & Shamir, A. (2007). "Seam carving for content-aware image resizing." ACM SIGGRAPH 2007.
- Viola, P., & Jones, M. (2001). "Rapid object detection using a boosted cascade of simple features." CVPR 2001.
- OpenCV Documentation: https://docs.opencv.org/

### Technologies Used

- **C++17**: Modern C++ with RAII and class-based design
- **OpenCV 4**: Computer vision library for image processing and face detection
- **Haar Cascade Classifiers**: Face detection using pre-trained models
- **Dynamic Programming**: Optimal seam selection algorithm

## License

This project is provided as-is for educational and research purposes.

## Contributing

Suggestions and improvements are welcome! Key areas for enhancement:

- Additional energy functions (entropy, saliency)
- Multi-face optimization
- GPU acceleration
- Interactive seam visualization
- Support for other object detection methods (DNN-based)

---

**Last Updated:** November 13, 2025
