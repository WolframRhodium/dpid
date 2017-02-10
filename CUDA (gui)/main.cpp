// Copyright (c) 2016 Nicolas Weber and Sandra C. Amend / GCC / TU-Darmstadt. All rights reserved. 
// Use of this source code is governed by the BSD 3-Clause license that can be
// found in the LICENSE file.
// Modified by Muonium
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdint>
#include <cctype>

//-------------------------------------------------------------------
// SHARED
//-------------------------------------------------------------------
struct Params {
    uint32_t oWidth;
    uint32_t oHeight;
    uint32_t iWidth;
    uint32_t iHeight;
    float pWidth;
    float pHeight;
    float lambda;
};

//-------------------------------------------------------------------
// HOST
//-------------------------------------------------------------------
void run(const Params& i, const void* hInput, void* hOutput);

//-------------------------------------------------------------------
int main(int argc, char** argv) {
    // check if there are enough arguments
    if((argc < 2) || (argc > 5)) {
		std::cout << "usage: dpid-gui <input-filename> [output-width=128] [output-height=0] [max-lambda=1]\n"
			<< "  <required> [optional]\n\n"
			<< "examples:\n"
			<< "  dpid \"myImage.jpg\"              // downscales using default values\n"
			<< "  dpid \"myImage.jpg\" 256          // downscales to 256px width, keeping aspect ratio\n"
			<< "  dpid \"myImage.jpg\" 0 256        // downscales to 256px height, keeping aspect ratio\n"
			<< "  dpid \"myImage.jpg\" 128 128      // downscales to 128x128px, ignoring aspect ratio\n\n"
			<< "hotkeys in GUI:\n"
			<< "  1: Source image\n"
			<< "  2: Downscale using dpid(lambda=lambda1) (and upscale to original size using nearest neighbour interpolation)\n"
			<< "  3: Downscale using dpid(lambda=lambda2) (and upscale to original size using nearest neighbour interpolation)\n"
			<< "  4: Downscale using cubic interpolation (and upscale to original size using nearest neighbour interpolation)\n" 
			<< "  s: Save current image\n" 
			<< "  esc: Exit\n" << std::endl;
        exit(1);
    }
    
    const char* iName = argv[1];
	int max_lambda = 100; // multiplied by 100

    // read params or set default parameters
    Params i = { 0 };
    if (argc < 3)
        i.oWidth = (uint32_t)128;
    else
        i.oWidth = (uint32_t)std::atoi(argv[2]);
    if (argc < 4)
        i.oHeight = (uint32_t)0;
    else
        i.oHeight = (uint32_t)std::atoi(argv[3]);
	if (argc == 5)
	    max_lambda = (int)std::atoi(argv[4]) * 100;

    // check params
    if(i.oWidth == 0 && i.oHeight == 0) {
        std::cerr << "either width or height has to be non-zero!" << std::endl;
        exit(1);
    }

    // load image
    cv::Mat iImage = cv::imread(iName);
	
    if(!iImage.data)  {
        std::cerr << "unable to read image" << std::endl;
        exit(1);
    }

	// get input image size
    i.iWidth = iImage.cols;
    i.iHeight = iImage.rows;
	cv::Size iSize(i.iWidth, i.iHeight);
	
    // calc width/height according to aspect ratio
    if(i.oWidth == 0)
        i.oWidth = (uint32_t)std::round((i.oHeight / (double)i.iHeight) * i.iWidth);
    if(i.oHeight == 0)
        i.oHeight = (uint32_t)std::round((i.oWidth / (double)i.iWidth) * i.iHeight);
	cv::Size oSize(i.oWidth, i.oHeight);
	
    // alloc output
    cv::Mat oImage1(i.oHeight, i.oWidth, CV_8UC3);
	cv::Mat oImage2(i.oHeight, i.oWidth, CV_8UC3);
	cv::Mat oImage3(i.oHeight, i.oWidth, CV_8UC3);
	cv::Mat display(i.iHeight, i.iHeight, CV_8UC3);

    // calc patch size
    i.pWidth = i.iWidth / (float)i.oWidth;
    i.pHeight = i.iHeight / (float)i.oHeight;

	// create window
	cv::namedWindow("dpid-gui", cv::WINDOW_NORMAL);

	// create track bar
	cv::createTrackbar("lambda1", "dpid-gui", nullptr, max_lambda);
	cv::createTrackbar("lambda2", "dpid-gui", nullptr, max_lambda);

	// set parameter (1:source; 2:downscale using lambda1; 3:downscale using lambda2; 4:bicubic)
	int currentImage = 1;

	// display source image
	cv::imshow("dpid-gui", iImage);

	for (;;)
	{
		char c = (char)cv::waitKey(0);
		
		if (c == '1')
		{
			currentImage = 1;
			cv::imshow("dpid-gui", iImage);
		}
		else if (c == '2')
		{
			currentImage = 2;
			i.lambda = (float)cv::getTrackbarPos("lambda1", "dpid-gui") / 100;
			run(i, iImage.data, oImage1.data);
			cv::resize(oImage1, display, iSize, 0.0, 0.0, cv::INTER_NEAREST);
			cv::imshow("dpid-gui", display);
		}
		else if (c == '3')
		{
			currentImage = 3;
			i.lambda = (float)cv::getTrackbarPos("lambda2", "dpid-gui") / 100;
			run(i, iImage.data, oImage2.data);
			cv::resize(oImage2, display, iSize, 0.0, 0.0, cv::INTER_NEAREST);
			cv::imshow("dpid-gui", display);
		}
		else if (c == '4')
		{
			currentImage = 4;
			cv::resize(iImage, oImage3, oSize, 0.0, 0.0, cv::INTER_CUBIC);
			cv::resize(oImage3, display, iSize, 0.0, 0.0, cv::INTER_NEAREST);
			cv::imshow("dpid-gui", display);
		}
		else if (tolower(c) == 's')
		{
			if (currentImage == 2)
			{
				std::string oName(iName);
				oName = oName + "_" + std::to_string(i.oWidth) + "x" + std::to_string(i.oHeight) + "_" + std::to_string(i.lambda) + ".png";
				cv::imwrite(oName, oImage1);
				std::cout << "Output filename: " << oName << std::endl;
			}
			else if (currentImage == 3)
			{
				std::string oName(iName);
				oName = oName + "_" + std::to_string(i.oWidth) + "x" + std::to_string(i.oHeight) + "_" + std::to_string(i.lambda) + ".png";
				cv::imwrite(oName, oImage2);
				std::cout << "Output filename: " << oName << std::endl;
			}
			else if (currentImage == 4)
			{
				std::string oName(iName);
				oName = oName + "_" + std::to_string(i.oWidth) + "x" + std::to_string(i.oHeight) + "_bicubic.png";
				cv::imwrite(oName, oImage3);
				std::cout << "Output filename: " << oName << std::endl;
			}
		}
		else if (c == 27)
			break;
	}
	
	cv::destroyWindow("dpid-gui");

    return 0;
}
