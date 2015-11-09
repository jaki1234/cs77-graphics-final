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

//g++ lodepng.cpp example_png_info.cpp -ansi -pedantic -Wall -Wextra -lSDL -O3

/*
This sample shows a lot of information in the console about a PNG file,
including color type, text chunks, the names of all chunks in the image,
etc...
*/


#include "lodepng.h"
#include <iostream>

int main(int argc, char *argv[]) /*list the chunks*/
{
  if(argc < 2)
  {
    std::cout << "Please provide a filename to preview" << std::endl;
    return 0;
  }
  const char* filename = argv[1];
  
  std::vector<unsigned char> buffer;
  std::vector<unsigned char> image;
  unsigned w, h;
  
  lodepng::load_file(buffer, filename); //load the image file with given filename
  
  lodepng::State state;
  unsigned error = lodepng::decode(image, w, h, state, buffer);
  
  if(error)
  {
    std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    return 0;
  }
  
  LodePNGInfo& info = state.info_png;
  LodePNGColorMode& color = state.info_png.color;
  
  std::cout << "Width: " << w << std::endl;
  std::cout << "Height: " << h << std::endl;
  std::cout << "Num pixels: " << w * h << std::endl;
  std::cout << "Compression method: " << info.compression_method << std::endl;
  std::cout << "Filter method: " << info.filter_method << std::endl;
  std::cout << "Interlace method: " << info.interlace_method << std::endl;
  std::cout << "Color type: " << color.colortype << std::endl;
  std::cout << "Bit depth: " << color.bitdepth << std::endl;
  std::cout << "Bits per pixel: " << lodepng_get_bpp(&color) << std::endl;
  std::cout << "Channels per pixel: " << lodepng_get_channels(&color) << std::endl;
  std::cout << "Is greyscale type: " << lodepng_is_greyscale_type(&color) << std::endl;
  std::cout << "Can have alpha: " << lodepng_can_have_alpha(&color) << std::endl;
  std::cout << "Palette size: " << color.palettesize << std::endl;
  std::cout << "Has color key: " << color.key_defined << std::endl;
  if(color.key_defined)
  {
    std::cout << "Color key r: " << color.key_r << std::endl;
    std::cout << "Color key g: " << color.key_g << std::endl;
    std::cout << "Color key b: " << color.key_b << std::endl;
  }
  std::cout << "Texts: " << info.text_num << std::endl;
  for(size_t i = 0; i < info.text_num; i++)
  {
    std::cout << "Text: " << info.text_keys[i] << ": " << info.text_strings[i] << std::endl << std::endl;
  }
  std::cout << "International texts: " << info.itext_num << std::endl;
  for(size_t i = 0; i < info.itext_num; i++)
  {
    std::cout << "Text: "
              << info.itext_keys[i] << ", "
              << info.itext_langtags[i] << ", "
              << info.itext_transkeys[i] << ": "
              << info.itext_strings[i] << std::endl << std::endl;
  }
  std::cout << "Time defined: " << info.time_defined << std::endl;
  if(info.time_defined)
  {
    LodePNGTime& time = info.time;
    std::cout << "year: " << time.year << std::endl;
    std::cout << "month: " << time.month << std::endl;
    std::cout << "day: " << time.day << std::endl;
    std::cout << "hour: " << time.hour << std::endl;
    std::cout << "minute: " << time.minute << std::endl;
    std::cout << "second: " << time.second << std::endl;
  }
  std::cout << "Physics defined: " << info.phys_defined << std::endl;
  if(info.phys_defined)
  {
    std::cout << "physics X: " << info.phys_x << std::endl;
    std::cout << "physics Y: " << info.phys_y << std::endl;
    std::cout << "physics unit: " << info.phys_unit << std::endl;
  }
  if(w > 0 && h > 0)
  {
    std::cout << "Top left pixel color:"
              << " r: " << (int)image[0]
              << " g: " << (int)image[1]
              << " b: " << (int)image[2]
              << " a: " << (int)image[3]
              << std::endl;
  }
  
  
  // Listing chunks is based on the original file, not the decoded png info.
  unsigned char *chunk, *begin, *end;
  end = &buffer.back() + 1;
  begin = chunk = &buffer.front() + 8;

  std::cout << std::endl << "Chunks:" << std::endl;
  std::cout << "  type : length" << std::endl;
  std::cout << "  ---- : ------" << std::endl;
  while(chunk + 8 < end && chunk >= begin)
  {
    char type[5];
    lodepng_chunk_type(type, chunk);
    if(std::string(type).size() != 4)
    {
      std::cout << "this is probably not a PNG" << std::endl;
      return 0;
    }
    std::cout << "  " << type << " : " << lodepng_chunk_length(chunk) << std::endl;
    chunk = lodepng_chunk_next(chunk);
  }
  
  //Show ASCII art preview of the image
  if(w > 0 && h > 0)
  {
    std::cout << std::endl << "ASCII Art Preview: " << std::endl;
    unsigned w2 = 40;
    if(w < w2) w2 = w;
    unsigned h2 = h * w2 / w;
    h2 = (h2 * 2) / 3; //compensate for non-square characters in terminal
    if(h2 > (w2 * 2)) h2 = w2 * 2; //avoid too large output
    
    std::cout << '+';
    for(unsigned x = 0; x < w2; x++) std::cout << '-';
    std::cout << '+' << std::endl;
    for(unsigned y = 0; y < h2; y++)
    {
      std::cout << "|";
      for(unsigned x = 0; x < w2; x++)
      {
        unsigned x2 = x * w / w2;
        unsigned y2 = y * h / h2;
        int r = image[y2 * w * 4 + x2 * 4 + 0];
        int g = image[y2 * w * 4 + x2 * 4 + 1];
        int b = image[y2 * w * 4 + x2 * 4 + 2];
        int a = image[y2 * w * 4 + x2 * 4 + 3];
        int lightness = ((r + g + b) / 3) * a / 255;
        int min = (r < g && r < b) ? r : (g < b ? g : b);
        int max = (r > g && r > b) ? r : (g > b ? g : b);
        int saturation = max - min;
        int letter = 'i'; //i for grey, or r,y,g,c,b,m for colors
        if(saturation > 32)
        {
          int h = lightness >= (min + max) / 2;
          if(h) letter = (min == r ? 'c' : (min == g ? 'm' : 'y'));
          else letter = (max == r ? 'r' : (max == g ? 'g' : 'b'));
        }
        int symbol = ' ';
        if(lightness > 224) symbol = '@';
        else if(lightness > 128) symbol = letter - 32;
        else if(lightness > 32) symbol = letter;
        else if(lightness > 16) symbol = '.';
        std::cout << (char)symbol;
      }
      std::cout << "|";
      std::cout << std::endl;
    }
    std::cout << '+';
    for(unsigned x = 0; x < w2; x++) std::cout << '-';
    std::cout << '+' << std::endl;
  }
}
