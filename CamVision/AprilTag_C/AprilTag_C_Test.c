#include <opencv2/opencv.hpp>
#include <apriltag.h>
#include <tag36h11.h>
#include <zlib.h>

#define LINE_LENGTH 5
#define CENTER_COLOR cv::Scalar(0, 255, 0)
#define CORNER_COLOR cv::Scalar(255, 0, 255)

void plotPoint(cv::Mat& image, cv::Point2f center, cv::Scalar color) {
    cv::line(image, cv::Point(center.x - LINE_LENGTH, center.y), cv::Point(center.x + LINE_LENGTH, center.y), color, 3);
    cv::line(image, cv::Point(center.x, center.y - LINE_LENGTH), cv::Point(center.x, center.y + LINE_LENGTH), color, 3);
}

void plotText(cv::Mat& image, cv::Point2f center, cv::Scalar color, const std::string& text) {
    cv::putText(image, text, cv::Point(center.x + 4, center.y - 4), cv::FONT_HERSHEY_SIMPLEX, 1, color, 3);
}

int main() {
    // Initialize AprilTag detector
    apriltag_family_t* tf = tag36h11_create();
    apriltag_detector_t* detector = apriltag_detector_create();
    apriltag_detector_add_family(detector, tf);

    // Open camera
    cv::VideoCapture cam(0);
    if (!cam.isOpened()) {
        std::cerr << "Error: Camera not opened!" << std::endl;
        return -1;
    }

    // Main loop
    bool looping = true;
    cv::Mat image, grayimg;
    while (looping) {
        cam >> image;  // Capture frame
        if (image.empty()) {
            std::cerr << "Error: Empty frame!" << std::endl;
            break;
        }

        // Convert to grayscale
        cv::cvtColor(image, grayimg, cv::COLOR_BGR2GRAY);

        // Prepare AprilTag image
        uint8_t* gray_data = grayimg.data;
        image_u8_t im = { .width = grayimg.cols, .height = grayimg.rows, .stride = grayimg.cols, .buf = gray_data };

        // Detect tags
        zarray_t* detections = apriltag_detector_detect(detector, &im);

        if (zarray_size(detections) == 0) {
            std::cout << "Nothing" << std::endl;
        } else {
            // Found some tags, process each one
            for (int i = 0; i < zarray_size(detections); i++) {
                apriltag_detection_t* det;
                zarray_get(detections, i, &det);
                std::cout << "tag_id: " << det->id << ", center: (" << det->p[0][0] << ", " << det->p[0][1] << ")" << std::endl;

                // Plot the center point and tag id
                plotPoint(image, cv::Point2f(det->c[0], det->c[1]), CENTER_COLOR);
                plotText(image, cv::Point2f(det->c[0], det->c[1]), CENTER_COLOR, std::to_string(det->id));

                // Plot corners
                for (int j = 0; j < 4; j++) {
                    plotPoint(image, cv::Point2f(det->p[j][0], det->p[j][1]), CORNER_COLOR);
                }
            }
        }

        // Show the result
        cv::imshow("Result", image);

        // Wait for key press, exit on 'Enter' key
        int key = cv::waitKey(100);
        if (key == 13) {  // Enter key
            looping = false;
        }
    }

    // Cleanup
    cv::destroyAllWindows();
    cv::imwrite("final.png", image);

    // Free AprilTag detector
    apriltag_detector_destroy(detector);
    tag36h11_destroy(tf);

    return 0;
}