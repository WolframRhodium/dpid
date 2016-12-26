# Description
Modified version of ["Rapid, Detail-Preserving Image Downscaling"](http://www.gcc.tu-darmstadt.de/home/proj/dpid/)

# Usage
## CUDA:
    dpid <input-filename> [output-width=128] [output-height=0] [lambda=1.0] (<required> [optional])

## MATLAB:
    dpid(<input-filename>[, output-width=128][, output-height=0][, lambda=1.0]);

# Examples (CUDA)
    dpid "myImage.jpg"              // downscales using default values
    dpid "myImage.jpg" 256          // downscales to 256px width, keeping aspect ratio
    dpid "myImage.jpg" 0 256        // downscales to 256px height, keeping aspect ratio
    dpid "myImage.jpg" 128 0 0.5    // downscales to 128px width, keeping aspect ratio, using lamdba=0.5
    dpid "myImage.jpg" 128 128      // downscales to 128x128px, ignoring aspect ratio
