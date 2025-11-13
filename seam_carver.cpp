/**
 * @file seam_carver.cpp
 * @author Utkarsh Sachan
 * @brief A professional, class-based C++ implementation of the Seam Carving algorithm.
 *
 * This tool resizes an image using content-aware seam carving. It supports
 * both seam removal (shrinking) and seam insertion (expanding) for width and height.
 *
 * It also includes advanced features:
 * 1. Sobel Energy Function: Uses Sobel gradients for robust energy calculation.
 * 2. Protection Masking: Allows a user to provide a mask to protect areas
 * (e.g., faces) from being carved. Protected pixels are given max energy.
 * 3. Removal Masking: Allows a user to provide a mask to target areas
 * (e.g., an object) for removal. Targeted pixels are given min energy.
 *
 * This project uses modern C++ practices:
 * - Encapsulated in a `SeamCarver` class.
 * - No global variables.
 * - Uses std::vector for all dynamic arrays (RAII, no manual malloc/free).
 * - Uses cv::Mat for all image and energy map storage.
 * - Provides a clean command-line interface (CLI) via cv::CommandLineParser.
 *
 * ---
 *
 * Build Command:
 * g++ -std=c++17 -o seam_carver seam_carver.cpp `pkg-config --cflags --libs opencv4`
 *
 * ---
 *
 * Example Usage:
 *
 * 1. Shrink image width and height:
 * ./seam_carver -i=input.jpg -o=output.jpg -w=500 -h=300
 *
 * 2. Expand image width:
 * ./seam_carver -i=input.jpg -o=output.jpg -w=800
 *
 * 3. Shrink, but protect a face:
 * ./seam_carver -i=face.jpg -o=shrunk.jpg -w=400 --protect=face_mask.png
 *
 * 4. Remove an object from a scene:
 * ./seam_carver -i=scene.jpg -o=removed.jpg -w=500 --remove=object_mask.png
 *
 */

#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <opencv2/opencv.hpp>

// Use high-precision constants for energy modification
const double MAX_ENERGY = 1e9;
const double MIN_ENERGY = -1e9;

/**
 * @class SeamCarver
 * @brief Encapsulates all logic and data for the seam carving algorithm.
 */
class SeamCarver {
public:
    /**
     * @brief Loads the image and optional masks.
     * @param imagePath Path to the input image.
     * @param protectMaskPath Path to the (optional) protection mask.
     * @param removeMaskPath Path to the (optional) removal mask.
     */
    SeamCarver(const std::string& imagePath, const std::string& protectMaskPath, const std::string& removeMaskPath) {
        m_image = cv::imread(imagePath);
        if (m_image.empty()) {
            throw std::runtime_error("Could not load input image: " + imagePath);
        }

        // Load optional masks
        if (!protectMaskPath.empty()) {
            m_protectionMask = cv::imread(protectMaskPath, cv::IMREAD_GRAYSCALE);
            if (m_protectionMask.empty()) {
                std::cerr << "Warning: Could not load protection mask: " << protectMaskPath << std::endl;
            } else if (m_protectionMask.size() != m_image.size()) {
                std::cerr << "Warning: Protection mask dimensions do not match image. Resizing mask." << std::endl;
                cv::resize(m_protectionMask, m_protectionMask, m_image.size());
            }
        }

        if (!removeMaskPath.empty()) {
            m_removalMask = cv::imread(removeMaskPath, cv::IMREAD_GRAYSCALE);
            if (m_removalMask.empty()) {
                std::cerr << "Warning: Could not load removal mask: " << removeMaskPath << std::endl;
            } else if (m_removalMask.size() != m_image.size()) {
                std::cerr << "Warning: Removal mask dimensions do not match image. Resizing mask." << std::endl;
                cv::resize(m_removalMask, m_removalMask, m_image.size());
            }
        }

        std::cout << "Image loaded: " << m_image.cols << "x" << m_image.rows << std::endl;
    }

    /**
     * @brief Resizes the image to the target dimensions.
     * @param newWidth The target width.
     * @param newHeight The target height.
     */
    void resize(int newWidth, int newHeight) {
        if (newWidth < 0 || newHeight < 0) {
            throw std::invalid_argument("New dimensions must be non-negative.");
        }

        int currentWidth = m_image.cols;
        int currentHeight = m_image.rows;

        // --- 1. Width Resizing ---
        int deltaCols = newWidth - currentWidth;
        if (deltaCols < 0) {
            std::cout << "Reducing width by " << -deltaCols << " pixels..." << std::endl;
            for (int i = 0; i < -deltaCols; ++i) {
                calculateEnergy();
                std::vector<int> seam = findVerticalSeam();
                removeVerticalSeam(seam);
            }
        } else if (deltaCols > 0) {
            std::cout << "Expanding width by " << deltaCols << " pixels..." << std::endl;
            // For expansion, we find all seams at once on the original image
            // to avoid repeatedly adding seams in the same low-energy area.
            cv::Mat originalImage = m_image.clone();
            std::vector<std::vector<int>> seams;
            for (int i = 0; i < deltaCols; ++i) {
                calculateEnergy();
                std::vector<int> seam = findVerticalSeam();
                seams.push_back(seam);
                // Temporarily remove seam to find the *next* best seam
                removeVerticalSeam(seam);
            }
            // Restore original image and add all found seams
            m_image = originalImage;
            addVerticalSeams(seams);
        }

        // --- 2. Height Resizing ---
        int deltaRows = newHeight - currentHeight;
        if (deltaRows < 0) {
            std::cout << "Reducing height by " << -deltaRows << " pixels..." << std::endl;
            for (int i = 0; i < -deltaRows; ++i) {
                calculateEnergy();
                std::vector<int> seam = findHorizontalSeam();
                removeHorizontalSeam(seam);
            }
        } else if (deltaRows > 0) {
            std::cout << "Expanding height by " << deltaRows << " pixels..." << std::endl;
            cv::Mat originalImage = m_image.clone();
            std::vector<std::vector<int>> seams;
            for (int i = 0; i < deltaRows; ++i) {
                calculateEnergy();
                std::vector<int> seam = findHorizontalSeam();
                seams.push_back(seam);
                removeHorizontalSeam(seam);
            }
            m_image = originalImage;
            addHorizontalSeams(seams);
        }

        std::cout << "Resize complete. New dimensions: " << m_image.cols << "x" << m_image.rows << std::endl;
    }

    /**
     * @brief Saves the processed image to a file.
     * @param outputPath Path to save the new image.
     */
    void saveImage(const std::string& outputPath) {
        if (!cv::imwrite(outputPath, m_image)) {
            throw std::runtime_error("Failed to save image to: " + outputPath);
        }
        std::cout << "Image saved successfully to: " << outputPath << std::endl;
    }

    /**
     * @brief Displays the current image in a window.
     * @param windowName The name for the display window.
     */
    void showImage(const std::string& windowName) {
        cv::imshow(windowName, m_image);
        std::cout << "Press any key to close the image window..." << std::endl;
        cv::waitKey(0);
    }

private:
    cv::Mat m_image;
    cv::Mat m_energyMap;
    cv::Mat m_protectionMask;
    cv::Mat m_removalMask;

    /**
     * @brief Calculates the energy map using Sobel filters and applies masks.
     */
    void calculateEnergy() {
        cv::Mat gray, grad_x, grad_y, abs_grad_x, abs_grad_y;

        // 1. Convert to grayscale
        cv::cvtColor(m_image, gray, cv::COLOR_BGR2GRAY);

        // 2. Apply Sobel filters with stronger kernel for better edge detection
        cv::Sobel(gray, grad_x, CV_64F, 1, 0, 5);
        cv::Sobel(gray, grad_y, CV_64F, 0, 1, 5);

        // 3. Compute gradient magnitude: E = sqrt(grad_x^2 + grad_y^2)
        cv::Mat grad_x_sq, grad_y_sq;
        cv::multiply(grad_x, grad_x, grad_x_sq);
        cv::multiply(grad_y, grad_y, grad_y_sq);
        cv::sqrt(grad_x_sq + grad_y_sq, m_energyMap);

        // Normalize to 0-255 range for better contrast
        cv::normalize(m_energyMap, m_energyMap, 0, 255, cv::NORM_MINMAX);

        // 4. Apply masks
        if (!m_protectionMask.empty()) {
            for (int r = 0; r < m_image.rows; ++r) {
                for (int c = 0; c < m_image.cols; ++c) {
                    // If mask pixel is non-zero (white), apply max energy
                    if (m_protectionMask.at<uchar>(r, c) > 0) {
                        m_energyMap.at<double>(r, c) = MAX_ENERGY;
                    }
                }
            }
        }
        
        if (!m_removalMask.empty()) {
            for (int r = 0; r < m_image.rows; ++r) {
                for (int c = 0; c < m_image.cols; ++c) {
                    // If mask pixel is non-zero (white), apply min energy
                    if (m_removalMask.at<uchar>(r, c) > 0) {
                        m_energyMap.at<double>(r, c) = MIN_ENERGY;
                    }
                }
            }
        }
    }

    /**
     * @brief Finds the lowest-energy vertical seam using dynamic programming.
     * @return A vector of column indices, one for each row.
     */
    std::vector<int> findVerticalSeam() {
        int rows = m_image.rows;
        int cols = m_image.cols;
        std::vector<int> seam(rows);

        // DP cost matrix
        cv::Mat dpCost = cv::Mat(rows, cols, CV_64F);

        // Parent pointers to reconstruct the path
        cv::Mat parent = cv::Mat(rows, cols, CV_32S);

        // 1. Initialize first row
        m_energyMap.row(0).copyTo(dpCost.row(0));

        // 2. Fill DP table
        for (int r = 1; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                double left = (c > 0) ? dpCost.at<double>(r - 1, c - 1) : std::numeric_limits<double>::max();
                double middle = dpCost.at<double>(r - 1, c);
                double right = (c < cols - 1) ? dpCost.at<double>(r - 1, c + 1) : std::numeric_limits<double>::max();

                double minVal = middle;
                int minIdx = c;

                if (left < minVal) {
                    minVal = left;
                    minIdx = c - 1;
                }
                if (right < minVal) {
                    minVal = right;
                    minIdx = c + 1;
                }

                dpCost.at<double>(r, c) = m_energyMap.at<double>(r, c) + minVal;
                parent.at<int>(r, c) = minIdx;
            }
        }

        // 3. Find minimum cost in the last row
        double minVal = std::numeric_limits<double>::max();
        int minIdx = 0;
        for (int c = 0; c < cols; ++c) {
            if (dpCost.at<double>(rows - 1, c) < minVal) {
                minVal = dpCost.at<double>(rows - 1, c);
                minIdx = c;
            }
        }

        // 4. Backtrack to find the seam
        seam[rows - 1] = minIdx;
        for (int r = rows - 2; r >= 0; --r) {
            seam[r] = parent.at<int>(r + 1, seam[r + 1]);
        }

        return seam;
    }

    /**
     * @brief Removes a vertical seam from the image.
     * @param seam The seam to remove (vector of column indices).
     */
    void removeVerticalSeam(const std::vector<int>& seam) {
        int rows = m_image.rows;
        int cols = m_image.cols;

        cv::Mat newImage(rows, cols - 1, m_image.type());

        for (int r = 0; r < rows; ++r) {
            int seamCol = seam[r];
            for (int c = 0; c < cols - 1; ++c) {
                if (c < seamCol) {
                    newImage.at<cv::Vec3b>(r, c) = m_image.at<cv::Vec3b>(r, c);
                } else {
                    newImage.at<cv::Vec3b>(r, c) = m_image.at<cv::Vec3b>(r, c + 1);
                }
            }
        }
        m_image = newImage;

        // Also update masks if they exist
        if (!m_protectionMask.empty()) {
            cv::Mat newMask(rows, cols - 1, m_protectionMask.type());
            for (int r = 0; r < rows; ++r) {
                int seamCol = seam[r];
                for (int c = 0; c < cols - 1; ++c) {
                    newMask.at<uchar>(r, c) = (c < seamCol) ? m_protectionMask.at<uchar>(r, c) : m_protectionMask.at<uchar>(r, c + 1);
                }
            }
            m_protectionMask = newMask;
        }

        if (!m_removalMask.empty()) {
            cv::Mat newMask(rows, cols - 1, m_removalMask.type());
            for (int r = 0; r < rows; ++r) {
                int seamCol = seam[r];
                for (int c = 0; c < cols - 1; ++c) {
                    newMask.at<uchar>(r, c) = (c < seamCol) ? m_removalMask.at<uchar>(r, c) : m_removalMask.at<uchar>(r, c + 1);
                }
            }
            m_removalMask = newMask;
        }
    }

    /**
     * @brief Adds multiple vertical seams to the image.
     * @param seams A vector of seams to add.
     */
    void addVerticalSeams(std::vector<std::vector<int>>& seams) {
        int rows = m_image.rows;
        int cols = m_image.cols;
        int numSeams = seams.size();

        cv::Mat newImage(rows, cols + numSeams, m_image.type());

        for (int r = 0; r < rows; ++r) {
            // Sort seam indices for this row to process them from left to right
            std::vector<int> rowSeamIndices;
            for(const auto& seam : seams) {
                rowSeamIndices.push_back(seam[r]);
            }
            std::sort(rowSeamIndices.begin(), rowSeamIndices.end());

            int newCol = 0;
            int oldCol = 0;
            int seamIdx = 0;

            while (oldCol < cols) {
                // Copy original pixel
                newImage.at<cv::Vec3b>(r, newCol) = m_image.at<cv::Vec3b>(r, oldCol);

                // If this is a seam pixel, add a new pixel
                if (seamIdx < numSeams && oldCol == rowSeamIndices[seamIdx]) {
                    newCol++;
                    cv::Vec3b newPixel;
                    if (oldCol < cols - 1) {
                        // Average with right neighbor
                        newPixel = (m_image.at<cv::Vec3b>(r, oldCol) / 2) + (m_image.at<cv::Vec3b>(r, oldCol + 1) / 2);
                    } else {
                        // At edge, just duplicate
                        newPixel = m_image.at<cv::Vec3b>(r, oldCol);
                    }
                    newImage.at<cv::Vec3b>(r, newCol) = newPixel;
                    seamIdx++;
                }
                newCol++;
                oldCol++;
            }
        }
        m_image = newImage;
        // Note: We don't expand masks as the semantics are unclear.
        // We assume expansion adds "neutral" content.
    }

    /**
     * @brief Finds the lowest-energy horizontal seam.
     * @return A vector of row indices, one for each column.
     */
    std::vector<int> findHorizontalSeam() {
        // --- Transpose Method ---
        // 1. Transpose the image and energy map
        cv::Mat originalImage = m_image;
        cv::Mat originalEnergy = m_energyMap;
        cv::Mat originalProtect = m_protectionMask;
        cv::Mat originalRemove = m_removalMask;

        m_image = m_image.t();
        m_energyMap = m_energyMap.t();
        if(!m_protectionMask.empty()) m_protectionMask = m_protectionMask.t();
        if(!m_removalMask.empty()) m_removalMask = m_removalMask.t();
        
        // 2. Find a *vertical* seam on the transposed data
        std::vector<int> seam = findVerticalSeam();

        // 3. Restore original (non-transposed) data
        m_image = originalImage;
        m_energyMap = originalEnergy;
        m_protectionMask = originalProtect;
        m_removalMask = originalRemove;

        return seam;
    }

    /**
     * @brief Removes a horizontal seam from the image.
     * @param seam The seam to remove (vector of row indices).
     */
    void removeHorizontalSeam(const std::vector<int>& seam) {
        int rows = m_image.rows;
        int cols = m_image.cols;

        cv::Mat newImage(rows - 1, cols, m_image.type());

        for (int c = 0; c < cols; ++c) {
            int seamRow = seam[c];
            for (int r = 0; r < rows - 1; ++r) {
                if (r < seamRow) {
                    newImage.at<cv::Vec3b>(r, c) = m_image.at<cv::Vec3b>(r, c);
                } else {
                    newImage.at<cv::Vec3b>(r, c) = m_image.at<cv::Vec3b>(r + 1, c);
                }
            }
        }
        m_image = newImage;

        // Also update masks if they exist
        if (!m_protectionMask.empty()) {
            cv::Mat newMask(rows - 1, cols, m_protectionMask.type());
            for (int c = 0; c < cols; ++c) {
                int seamRow = seam[c];
                for (int r = 0; r < rows - 1; ++r) {
                    newMask.at<uchar>(r, c) = (r < seamRow) ? m_protectionMask.at<uchar>(r, c) : m_protectionMask.at<uchar>(r + 1, c);
                }
            }
            m_protectionMask = newMask;
        }

        if (!m_removalMask.empty()) {
            cv::Mat newMask(rows - 1, cols, m_removalMask.type());
            for (int c = 0; c < cols; ++c) {
                int seamRow = seam[c];
                for (int r = 0; r < rows - 1; ++r) {
                    newMask.at<uchar>(r, c) = (r < seamRow) ? m_removalMask.at<uchar>(r, c) : m_removalMask.at<uchar>(r + 1, c);
                }
            }
            m_removalMask = newMask;
        }
    }

    /**
     * @brief Adds multiple horizontal seams to the image.
     * @param seams A vector of seams to add.
     */
    void addHorizontalSeams(std::vector<std::vector<int>>& seams) {
        // --- Transpose Method ---
        // 1. Transpose the image
        m_image = m_image.t();

        // 2. Transpose the seams
        // (This is tricky. We need to convert horizontal seams to vertical seams on the transposed image)
        int rows = m_image.rows; // Original cols
        int cols = m_image.cols; // Original rows
        int numSeams = seams.size();
        
        std::vector<std::vector<int>> transposedSeams(numSeams, std::vector<int>(rows));
        for(int i = 0; i < numSeams; ++i) {
            for(int j = 0; j < rows; ++j) {
                transposedSeams[i][j] = seams[i][j];
            }
        }

        // 3. Call the *vertical* add function
        addVerticalSeams(transposedSeams);

        // 4. Transpose the result back
        m_image = m_image.t();
    }
};


// ---
// Main function: Handles Command-Line Interface (CLI)
// ---
const char* keys =
    "{help usage ? | | print this message }"
    "{ @input i       |   | path to input image (required) }"
    "{ @output o      |   | path to output image (required) }"
    "{ width w        | -1 | target width (default: original width) }"
    "{ height h       | -1 | target height (default: original height) }"
    "{ protect p      |   | (optional) path to protection mask }"
    "{ remove r       |   | (optional) path to removal mask }"
    "{ show s         |   | (optional) show final image in a window }";

int main(int argc, char* argv[]) {
    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("Seam Carving v2.0");

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::string inputPath = parser.get<std::string>("@input");
    std::string outputPath = parser.get<std::string>("@output");
    int targetWidth = parser.get<int>("width");
    int targetHeight = parser.get<int>("height");
    std::string protectPath = parser.get<std::string>("protect");
    std::string removePath = parser.get<std::string>("remove");
    bool showResult = parser.has("show");

    if (inputPath.empty() || outputPath.empty()) {
        std::cerr << "Error: Input and Output paths are required." << std::endl;
        parser.printMessage();
        return -1;
    }

    try {
        // 1. Initialize SeamCarver
        SeamCarver carver(inputPath, protectPath, removePath);

        // 2. Get original dimensions if not specified
        cv::Mat tempImg = cv::imread(inputPath);
        if (targetWidth == -1) {
            targetWidth = tempImg.cols;
        }
        if (targetHeight == -1) {
            targetHeight = tempImg.rows;
        }
        tempImg.release();

        // 3. Perform resize
        carver.resize(targetWidth, targetHeight);

        // 4. Save result
        carver.saveImage(outputPath);

        // 5. Optionally show result
        if (showResult) {
            carver.showImage("Seam Carving Result");
        }

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}