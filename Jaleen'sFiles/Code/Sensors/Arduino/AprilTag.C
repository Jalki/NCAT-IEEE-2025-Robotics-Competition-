#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d_c.h>
#include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>

typedef struct {
    CvCapture *camera;
    CvMat *objp;
    CvMat *gray;
    CvMat *mtx;
    CvMat *distort;
    CvMat *newcameramtx;
    CvMat **rvecs;
    CvMat **tvecs;
    CvPoint3D32f *corners;
    int h, w;
    CvTermCriteria criteria;
    CvSize board_size;
    CvMat *mapx;
    CvMat *mapy;
    CvRect roi;
} CameraCalibration;

void initCameraCalibration(CameraCalibration *calib) {
    calib->camera = cvCreateCameraCapture(0);

    calib->criteria = cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.001);

    calib->objp = cvCreateMat(6 * 7, 3, CV_32FC1);
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            cvmSet(calib->objp, i * 7 + j, 0, j);
            cvmSet(calib->objp, i * 7 + j, 1, i);
        }
    }

    calib->board_size = cvSize(7, 6);
    calib->rvecs = (CvMat **)malloc(sizeof(CvMat *) * 1);
    calib->tvecs = (CvMat **)malloc(sizeof(CvMat *) * 1);
}

void captureChessboard(CameraCalibration *calib) {
    IplImage *img = cvQueryFrame(calib->camera);
    calib->gray = cvCreateMat(img->height, img->width, CV_8UC1);
    cvCvtColor(img, calib->gray, CV_BGR2GRAY);

    int corners_found = cvFindChessboardCorners(calib->gray, calib->board_size, calib->corners);

    if (corners_found) {
        cvFindCornerSubPix(calib->gray, calib->corners, calib->board_size.width * calib->board_size.height, cvSize(11, 11), cvSize(-1, -1), calib->criteria);
    }

    cvDrawChessboardCorners(img, calib->board_size, calib->corners, corners_found);
    cvShowImage("img", img);
    cvWaitKey(1000);
    cvDestroyAllWindows();
}

void calibrate(CameraCalibration *calib) {
    CvMat *objpoints = cvCreateMat(1, calib->board_size.width * calib->board_size.height, CV_32FC3);
    CvMat *imgpoints = cvCreateMat(1, calib->board_size.width * calib->board_size.height, CV_32FC2);

    cvCalibrateCamera2(objpoints, imgpoints, calib->gray->cols, calib->gray->rows, calib->mtx, calib->distort, calib->rvecs, calib->tvecs, 0);

    printf("mtx:\n");
    cvmPrint(calib->mtx);
    printf("distort:\n");
    cvmPrint(calib->distort);

    cvSave("calibration_save.xml", calib->mtx, "camera_matrix", NULL);
    cvSave("calibration_save.xml", calib->distort, "dist_coeffs", NULL);
}

void undistort(CameraCalibration *calib) {
    IplImage *img = cvQueryFrame(calib->camera);
    IplImage *undistorted = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3);

    cvUndistort2(img, undistorted, calib->mtx, calib->distort);
    cvShowImage("undistorted", undistorted);
    cvWaitKey(1000);
    cvDestroyAllWindows();
    cvReleaseImage(&undistorted);
}

void remapping(CameraCalibration *calib) {
    IplImage *img = cvQueryFrame(calib->camera);
    IplImage *dst = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3);

    cvInitUndistortRectifyMap(calib->mtx, calib->distort, NULL, calib->newcameramtx, calib->mapx, calib->mapy);
    cvRemap(img, dst, calib->mapx, calib->mapy, CV_INTER_LINEAR, cvScalarAll(0));

    cvShowImage("Remapping", dst);
    cvWaitKey(0);
    cvDestroyAllWindows();
    cvReleaseImage(&dst);
}

void projectionError(CameraCalibration *calib) {
    double total_error = 0;
    int total_points = 0;
    CvMat *imgpoints2 = cvCreateMat(1, calib->board_size.width * calib->board_size.height, CV_32FC2);

    for (int i = 0; i < 1; i++) {
        cvProjectPoints2(calib->objp, calib->rvecs[i], calib->tvecs[i], calib->mtx, calib->distort, imgpoints2);

        double error = cvNorm(imgpoints2, calib->gray, CV_L2) / (calib->board_size.width * calib->board_size.height);
        total_error += error;
        total_points += (calib->board_size.width * calib->board_size.height);
    }

    printf("Total error: %f\n", total_error / total_points);
    cvReleaseMat(&imgpoints2);
}

int main() {
    CameraCalibration calibObj;
    initCameraCalibration(&calibObj);
    captureChessboard(&calibObj);
    calibrate(&calibObj);
    undistort(&calibObj);
    remapping(&calibObj);
    projectionError(&calibObj);

    cvReleaseCapture(&calibObj.camera);
    cvReleaseMat(&calibObj.objp);
    cvReleaseMat(&calibObj.gray);
    cvReleaseMat(&calibObj.mtx);
    cvReleaseMat(&calibObj.distort);
    cvReleaseMat(&calibObj.newcameramtx);
    free(calibObj.rvecs);
    free(calibObj.tvecs);

    return 0;
}
