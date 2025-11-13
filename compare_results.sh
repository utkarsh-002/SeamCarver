#!/bin/bash
# compare_results.sh
# Compares seam carving with and without face protection

set -e

if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <input_image> <target_width> <target_height> <output_prefix>"
    echo "Example: $0 img1.jpeg 300 300 result"
    exit 1
fi

INPUT=$1
WIDTH=$2
HEIGHT=$3
PREFIX=$4

echo "=== Seam Carving Comparison ==="
echo "Input: $INPUT"
echo "Target: ${WIDTH}x${HEIGHT}"
echo ""

# Step 1: Create face mask
echo "[1/3] Detecting faces and creating protection mask..."
./create_face_mask "$INPUT" "${PREFIX}_face_mask.png"
echo ""

# Step 2: Resize without protection
echo "[2/4] Resizing WITHOUT face protection..."
./seam_carver -i="$INPUT" -o="${PREFIX}_no_protection.jpg" -w="$WIDTH" --height="$HEIGHT"
echo ""

# Step 3: Resize with protection
echo "[3/4] Resizing WITH face protection..."
./seam_carver -i="$INPUT" -o="${PREFIX}_with_protection.jpg" -w="$WIDTH" --height="$HEIGHT" --protect="${PREFIX}_face_mask.png"
echo ""

# Step 4: Create visual comparison
echo "[4/4] Creating side-by-side visual comparison..."
./visualize_comparison "$INPUT" "${PREFIX}_no_protection.jpg" "${PREFIX}_with_protection.jpg" "${PREFIX}_side_by_side.jpg"
echo ""

echo "=== Results ==="
echo "Original image:       $INPUT"
echo "Face mask:            ${PREFIX}_face_mask.png"
echo "Without protection:   ${PREFIX}_no_protection.jpg"
echo "With protection:      ${PREFIX}_with_protection.jpg"
echo "Visual comparison:    ${PREFIX}_side_by_side.jpg"
echo ""
echo "Compare the results to see how face protection preserves facial features!"
