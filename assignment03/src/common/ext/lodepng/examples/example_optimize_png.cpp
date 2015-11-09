/*
LodePNG Examples

Copyright (c) 2005-2012 Lode Vandevenne

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

/*
This example saves the PNG with the best compression LodePNG can do, and with
unnecesary chunks removed.

NOTE: This is not as good as a true PNG optimizer like optipng or pngcrush.
*/

//g++ lodepng.cpp example_optimize_png.cpp -ansi -pedantic -Wall -Wextra -O3

#include "lodepng.h"

#include <iostream>

int main(int argc, char *argv[])
{
  std::vector<unsigned char> image;
  unsigned w, h;
  std::vector<unsigned char> buffer;
  unsigned error;
  
  //check if user gave a filename
  if(argc < 3)
  {
    std::cout << "please provide in and out filename" << std::endl;
    return 0;
  }
  
  lodepng::load_file(buffer, argv[1]);
  error = lodepng::decode(image, w, h, buffer);

  if(error)
  {
    std::cout << "decoding error " << error << ": " << lodepng_error_text(error) << std::endl;
    return 0;
  }
  
  std::cout << "Original size: " << (buffer.size() / 1024) << "K" << std::endl;
  buffer.clear();
  
  //Now encode as hard as possible
  
  lodepng::State state;
  state.encoder.zlibsettings.windowsize = 32768; //Slower but better deflate compression
  state.encoder.filter_strategy = LFS_BRUTE_FORCE; //This is quite slow
  state.encoder.add_id = false; //Don't add LodePNG version chunk to save more bytes
  state.encoder.text_compression = 1; //Not needed because we don't add text chunks, but this demonstrates another optimization setting
  
  error = lodepng::encode(buffer, image, w, h, state);

  if(error)
  {
    std::cout << "encoding error " << error << ": " << lodepng_error_text(error) << std::endl;
    return 0;
  }
  
  lodepng::save_file(buffer, argv[2]);
  std::cout << "New size: " << (buffer.size() / 1024) << "K" << std::endl;
}
