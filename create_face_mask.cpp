/**
 * @file create_face_mask.cpp
 * @brief Automatically detects faces in an image and creates a protection mask.
 *
 * This utility uses OpenCV's Haar Cascade classifier to detect faces
 * and creates a white mask over detected face regions. The mask can then
 * be used with seam_carver's --protect option to preserve faces during resizing.
 *
 * Build:
 * g++ -std=c++17 -o create_face_mask create_face_mask.cpp `pkg-config --cflags --libs opencv4`
 *
 * Usage:
 * ./create_face_mask input.jpg output_mask.png
 */

#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_image> <output_mask>" << std::endl;
        std::cerr << "Example: " << argv[0] << " img1.jpeg face_mask.png" << std::endl;
        return -1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = argv[2];

    // Load input image
    cv::Mat image = cv::imread(inputPath);
    if (image.empty()) {
        std::cerr << "Error: Could not load image: " << inputPath << std::endl;
        return -1;
    }

    // Create black mask (same size as image)
    cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);

    // Convert to grayscale for face detection
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    // Load Haar Cascade classifier for face detection
    cv::CascadeClassifier faceCascade;
    
    // Try common paths for the Haar cascade XML file
    std::vector<std::string> cascadePaths = {
        "/opt/homebrew/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
        "/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
        "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
        // Fallback to older OpenCV paths
        "/opt/homebrew/share/OpenCV/haarcascades/haarcascade_frontalface_default.xml",
        "/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_default.xml"
    };

    bool cascadeLoaded = false;
    for (const auto& path : cascadePaths) {
        if (faceCascade.load(path)) {
            std::cout << "Loaded cascade from: " << path << std::endl;
            cascadeLoaded = true;
            break;
        }
    }

    if (!cascadeLoaded) {
        std::cerr << "Error: Could not load Haar cascade classifier." << std::endl;
        std::cerr << "Tried paths:" << std::endl;
        for (const auto& path : cascadePaths) {
            std::cerr << "  - " << path << std::endl;
        }
        std::cerr << "\nPlease install OpenCV with Haar cascades or specify the correct path." << std::endl;
        return -1;
    }

    // Detect faces
    std::vector<cv::Rect> faces;
    faceCascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(30, 30));

    std::cout << "Detected " << faces.size() << " face(s)" << std::endl;

    if (faces.empty()) {
        std::cout << "Warning: No faces detected. Creating empty mask." << std::endl;
    }

    // Draw white rectangles on mask for each detected face
    // Expand the rectangle slightly to protect more area around the face
    for (size_t i = 0; i < faces.size(); ++i) {
        cv::Rect expandedFace = faces[i];
        
        // Expand by 20% on each side
        int expandX = static_cast<int>(faces[i].width * 0.2);
        int expandY = static_cast<int>(faces[i].height * 0.2);
        
        expandedFace.x = std::max(0, faces[i].x - expandX);
        expandedFace.y = std::max(0, faces[i].y - expandY);
        expandedFace.width = std::min(image.cols - expandedFace.x, faces[i].width + 2 * expandX);
        expandedFace.height = std::min(image.rows - expandedFace.y, faces[i].height + 2 * expandY);
        
        cv::rectangle(mask, expandedFace, cv::Scalar(255), -1); // Fill with white
        
        std::cout << "Face " << (i + 1) << " at: [" 
                  << expandedFace.x << ", " << expandedFace.y << ", "
                  << expandedFace.width << ", " << expandedFace.height << "]" << std::endl;
    }

    // Save the mask
    if (!cv::imwrite(outputPath, mask)) {
        std::cerr << "Error: Could not save mask to: " << outputPath << std::endl;
        return -1;
    }

    std::cout << "Face mask saved to: " << outputPath << std::endl;
    std::cout << "\nNow run seam carver with this mask:" << std::endl;
    std::cout << "./seam_carver -i=" << inputPath << " -o=output.jpg -w=<width> -h=<height> --protect=" << outputPath << std::endl;

    return 0;
}
