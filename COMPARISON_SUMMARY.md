# Comparison Results Summary

## Overview
This directory contains a complete comparison analysis of seam carving with and without face protection.

## Generated Files

### Images
- **img1.jpeg** - Original input image (1200x1600 pixels)
- **comparison_face_mask.png** - Binary protection mask (white = protected face region)
- **comparison_no_protection.jpg** - Result without face protection (300x300)
- **comparison_with_protection.jpg** - Result with face protection (300x300)
- **comparison_side_by_side.jpg** - Visual comparison showing all three images

### Documentation
- **COMPARISON_REPORT.md** - Detailed technical analysis and findings
- **README.md** - Complete project documentation

## Quick Visual Comparison

Open `comparison_side_by_side.jpg` to see:
- Left: Original image (resized to 300x300 for comparison)
- Middle: Seam carved WITHOUT face protection (distorted)
- Right: Seam carved WITH face protection (preserved)

## Key Statistics

| Metric | Value |
|--------|-------|
| Original Size | 1200 × 1600 pixels |
| Target Size | 300 × 300 pixels |
| Width Reduction | 75% (900 pixels removed) |
| Height Reduction | 81.25% (1300 pixels removed) |
| Total Seams Removed | 2,200 seams |
| Faces Detected | 1 |
| Protected Area | 256 × 256 pixels (expanded) |
| Processing Time | ~2-3 seconds per version |

## Findings

### ❌ Without Face Protection
- **Face distortion:** Severe compression and unnatural appearance
- **Facial features:** Eyes, nose, mouth positions altered
- **Aspect ratio:** Face width-to-height ratio severely changed
- **Overall quality:** Unusable for portrait purposes

### ✅ With Face Protection
- **Face preservation:** Features remain recognizable and proportional
- **Natural appearance:** Face looks normal despite aggressive resize
- **Background compression:** Non-face areas absorb the reduction
- **Overall quality:** Suitable for profile pictures, thumbnails, etc.

## Reproduction Steps

To recreate this comparison:

```bash
# 1. Create face detection mask
./create_face_mask img1.jpeg comparison_face_mask.png

# 2. Resize without protection
./seam_carver -i=img1.jpeg -o=comparison_no_protection.jpg -w=300 -h=300

# 3. Resize with protection
./seam_carver -i=img1.jpeg -o=comparison_with_protection.jpg -w=300 -h=300 \
  --protect=comparison_face_mask.png

# 4. Generate visual comparison
./visualize_comparison img1.jpeg comparison_no_protection.jpg \
  comparison_with_protection.jpg comparison_side_by_side.jpg
```

Or simply run the automated script:
```bash
./compare_results.sh img1.jpeg 300 300 comparison
```

## Conclusion

**Face protection is essential for seam carving on portrait images.**

The comparison clearly demonstrates that without protection, the seam carving algorithm treats facial features as low-importance content, leading to severe distortion. With face protection enabled, the face remains natural and recognizable even with extreme size reductions (75%+ in both dimensions).

## Recommendations

For ANY image containing people:
1. ✅ Always run face detection first
2. ✅ Use the `--protect` flag with the generated mask
3. ✅ Verify results visually
4. ✅ Keep reductions under 70% for best quality

For more details, see **COMPARISON_REPORT.md**.
