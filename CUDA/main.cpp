// Copyright (c) 2016 Nicolas Weber and Sandra C. Amend / GCC / TU-Darmstadt. All rights reserved. 
// Use of this source code is governed by the BSD 3-Clause license that can be
// found in the LICENSE file.
// Modified by Muonium
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdint>

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
        std::cout << "usage: dpid <input-filename> [output-width=128] [output-height=0] [lambda=1.0]\n"
                  << "  <required> [optional]\n\n"
                  << "examples:\n"
                  << "  dpid \"myImage.jpg\"              // downscales using default values\n"
                  << "  dpid \"myImage.jpg\" 256          // downscales to 256px width, keeping aspect ratio\n"
                  << "  dpid \"myImage.jpg\" 0 256        // downscales to 256px height, keeping aspect ratio\n"
                  << "  dpid \"myImage.jpg\" 128 0 0.5    // downscales to 128px width, keeping aspect ratio, using lamdba=0.5\n"
                  << "  dpid \"myImage.jpg\" 128 128      // downscales to 128x128px, ignoring aspect ratio" << std::endl;
        exit(1);
    }
    
    const char* iName = argv[1];

    // read params or set default parameters
    Params i = { 0 };
    if (argc < 3)
        i.oWidth = (uint32_t) 128;
    else
        i.oWidth = (uint32_t) std::atoi(argv[2]);
    if (argc < 4)
        i.oHeight = (uint32_t) 0;
    else
        i.oHeight = (uint32_t) std::atoi(argv[3]);
    if (argc < 5)
        i.lambda = 1.0f;
    else
        i.lambda = (float) std::atof(argv[4]);

    std::string oName(iName);

    // check params
    if(i.oWidth == 0 && i.oHeight == 0) {
        std::cerr << "either width or height has to be non-zero!" << std::endl;
        exit(1);
    }

    // load image
    cv::Mat iImage	= cv::imread(iName);
	
    if(!iImage.data)  {
    	std::cerr << "unable to read image" << std::endl;
    	exit(1);
    }

    i.iWidth = iImage.cols;
    i.iHeight = iImage.rows;
	
    // calc width/height according to aspect ratio
    if(i.oWidth == 0)
        i.oWidth = (uint32_t)std::round((i.oHeight / (double)i.iHeight) * i.iWidth);

    if(i.oHeight == 0)
        i.oHeight = (uint32_t)std::round((i.oWidth / (double)i.iWidth) * i.iHeight);

	oName = oName + "_" + std::to_string(i.oWidth) + "x" + std::to_string(i.oHeight) + "_" + std::to_string(i.lambda) + ".png";
	
    // alloc output
    cv::Mat oImage(i.oHeight, i.oWidth, CV_8UC3);

    // calc patch size
    i.pWidth = i.iWidth / (float)i.oWidth;
    i.pHeight = i.iHeight / (float)i.oHeight;

    // run cuda
    run(i, iImage.data, oImage.data);
	
    // write image
    cv::imwrite(oName, oImage);

	std::cout << "Output filename: " << oName << std::endl;
	
    return 0;
}
