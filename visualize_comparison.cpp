/**
 * @file visualize_comparison.cpp
 * @brief Creates a side-by-side comparison image showing original, unprotected, and protected results
 *
 * Build:
 * g++ -std=c++17 -o visualize_comparison visualize_comparison.cpp `pkg-config --cflags --libs opencv4`
 *
 * Usage:
 * ./visualize_comparison <original> <no_protection> <with_protection> <output>
 */

#include <iostream>
#include <opencv2/opencv.hpp>

void addLabel(cv::Mat& img, const std::string& text, const cv::Scalar& color = cv::Scalar(255, 255, 255)) {
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 0.8;
    int thickness = 2;
    int baseline = 0;
    
    cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
    cv::Point textOrg((img.cols - textSize.width) / 2, 30);
    
    // Add background rectangle for better readability
    cv::rectangle(img, 
                  cv::Point(textOrg.x - 5, textOrg.y - textSize.height - 5),
                  cv::Point(textOrg.x + textSize.width + 5, textOrg.y + baseline + 5),
                  cv::Scalar(0, 0, 0),
                  -1);
    
    cv::putText(img, text, textOrg, fontFace, fontScale, color, thickness);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <original> <no_protection> <with_protection> <output>" << std::endl;
        std::cerr << "Example: " << argv[0] << " img1.jpeg comparison_no_protection.jpg comparison_with_protection.jpg side_by_side.jpg" << std::endl;
        return -1;
    }

    std::string originalPath = argv[1];
    std::string noProtectionPath = argv[2];
    std::string withProtectionPath = argv[3];
    std::string outputPath = argv[4];

    // Load images
    cv::Mat original = cv::imread(originalPath);
    cv::Mat noProtection = cv::imread(noProtectionPath);
    cv::Mat withProtection = cv::imread(withProtectionPath);

    if (original.empty() || noProtection.empty() || withProtection.empty()) {
        std::cerr << "Error: Could not load one or more images" << std::endl;
        return -1;
    }

    // Resize original to match output dimensions for fair comparison
    cv::Mat originalResized;
    cv::resize(original, originalResized, noProtection.size());

    // Add labels
    addLabel(originalResized, "Original (Resized)", cv::Scalar(100, 255, 100));
    addLabel(noProtection, "Without Protection", cv::Scalar(100, 100, 255));
    addLabel(withProtection, "With Protection", cv::Scalar(100, 255, 100));

    // Create comparison image
    int padding = 20;
    int totalWidth = originalResized.cols + noProtection.cols + withProtection.cols + 4 * padding;
    int maxHeight = std::max({originalResized.rows, noProtection.rows, withProtection.rows}) + 2 * padding;
    
    cv::Mat comparison(maxHeight, totalWidth, originalResized.type(), cv::Scalar(50, 50, 50));

    // Copy images into comparison
    int xOffset = padding;
    originalResized.copyTo(comparison(cv::Rect(xOffset, padding, originalResized.cols, originalResized.rows)));
    
    xOffset += originalResized.cols + padding;
    noProtection.copyTo(comparison(cv::Rect(xOffset, padding, noProtection.cols, noProtection.rows)));
    
    xOffset += noProtection.cols + padding;
    withProtection.copyTo(comparison(cv::Rect(xOffset, padding, withProtection.cols, withProtection.rows)));

    // Add title
    std::string title = "Seam Carving Comparison: Face Protection Impact";
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 1.2;
    int thickness = 2;
    int baseline = 0;
    
    cv::Size textSize = cv::getTextSize(title, fontFace, fontScale, thickness, &baseline);
    cv::Point textOrg((comparison.cols - textSize.width) / 2, padding / 2 + textSize.height / 2);
    
    cv::putText(comparison, title, textOrg, fontFace, fontScale, cv::Scalar(255, 255, 255), thickness);

    // Add dimensions info at bottom
    std::stringstream ss;
    ss << "Original: " << original.cols << "x" << original.rows 
       << "  |  Resized: " << noProtection.cols << "x" << noProtection.rows
       << "  |  Reduction: " << std::fixed << std::setprecision(1) 
       << (100.0 * (1.0 - (double)noProtection.cols / original.cols)) << "% width, "
       << (100.0 * (1.0 - (double)noProtection.rows / original.rows)) << "% height";
    
    std::string info = ss.str();
    fontScale = 0.6;
    thickness = 1;
    textSize = cv::getTextSize(info, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline);
    textOrg = cv::Point((comparison.cols - textSize.width) / 2, comparison.rows - padding / 3);
    
    cv::putText(comparison, info, textOrg, cv::FONT_HERSHEY_SIMPLEX, fontScale, cv::Scalar(200, 200, 200), thickness);

    // Save result
    if (!cv::imwrite(outputPath, comparison)) {
        std::cerr << "Error: Could not save comparison image" << std::endl;
        return -1;
    }

    std::cout << "Side-by-side comparison saved to: " << outputPath << std::endl;
    std::cout << "Dimensions: " << comparison.cols << "x" << comparison.rows << std::endl;

    return 0;
}
