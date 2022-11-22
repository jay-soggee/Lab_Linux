#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/photo.hpp>

using namespace std;
using namespace cv;

const int xfltr[9] = {
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1
};
const int yfltr[9] = {
    -1, -2, -1,
     0,  0,  0,
     1,  2,  1
};

int main(int argc, char** argv)
{
    VideoCapture cap;
    cap.open("/dev/video0", CAP_V4L2);
    if (!cap.isOpened()) {
        printf("Can't open Camera\n");
        return -1;
    }

    int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);

    VideoWriter video("outcpp.avi", VideoWriter::fourcc('X','2','6','4'), 10, Size(frame_width,frame_height));

    printf("Open Camera\n");
    Mat img;
    int count = 0; int max;
    int i, j;
    float R_val, G_val, B_val;
    float luma_gray;
    int h = frame_width;
    int w = frame_height;
    Mat gray(h, w, CV_8UC1);
    Mat grad, grad_x, grad_y, abs_grad_x, abs_grad_y;

    if (argc > 1) {
        max = int(argv[1]);
    } else {
        max = 50;
    }

    while (count <= max) {
        cap.read(img);
        if (img.empty()) break;

        // grayscale conversion
        for (i = 0; i < h; i++) {
            for (j = 0; j < w; j++) {
                R_val = img.at<Vec3b>(i, j)[2];
                G_val = img.at<Vec3b>(i, j)[1];
                B_val = img.at<Vec3b>(i, j)[0];
                
                luma_gray = (int)(R_val * 0.299 + G_val * 0.587 + B_val * 0.114);

                gray.at<uchar>(i, j) = luma_gray;
            }
        }

        // sobel convolution
        Sobel(gray, grad_x, CV_8U, 1, 0);
        Sobel(gray, grad_y, CV_8U, 0, 1);
        convertScaleAbs(grad_x, abs_grad_x);
        convertScaleAbs(grad_y, abs_grad_y);
        addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

        video.write(grad);
        count++;
    }

    cap.release();
    video.release();
    return 0;
}
