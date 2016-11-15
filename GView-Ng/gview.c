
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <getopt.h>
#include <string.h>
#include <libgen.h>

#include <wand/magick_wand.h>

#include "fileutil.h"
#include "colors.h"
#include "image.h"
#include "paths.h"

char   *input_file_name  = NULL;
char   *output_file_name = NULL;
size_t  output_width     = 0;
int   exp_scale = 0;
int   animate = 0;

//extern char *path_file_name;
//extern struct Paths paths;

void usage(char *exe)
{

}

void parseargs(int argc, char *argv[])
{
   static struct option long_options[] = {
      { "help",     no_argument,       0, 'h' },
      { "output",   required_argument, 0, 'o' },
      { "width",    required_argument, 0, 'w' },
      { "paths ",   required_argument, 0, 'p' },
      { "colors",   required_argument, 0, 'c' },
      { "animate",  required_argument, 0, 'a' },
      { 0,          0,                 0,  0  } };

  while(1) {
      int option_index;
      int c = getopt_long(argc, argv, "hc:o:w:p:em:a", long_options, &option_index);

      if(c == -1)
         break;

      switch(c) {
         case 'h':
            usage(argv[0]);
         case 'c':
            set_color_theme(optarg);
            break;
         case 'o':
            output_file_name = strdup(optarg);
            break;
         case 'w':
            output_width = atoi(optarg);
            break;
         case 'p':
            //path_file_name = strdup(optarg);
            break;
         case 'a':
            animate = 1;
            break;
      }
   }
   input_file_name = strdup(argv[optind]); 
   if(output_file_name == NULL) {
      size_t n;
      output_file_name = basename(input_file_name);
      n = strlen(output_file_name);
      output_file_name[n-3] = 'p';
      output_file_name[n-2] = 'n';
      output_file_name[n-1] = 'g';
   }
}

int main(int argc, char *argv[])
{
   MagickWand *R;
   parseargs(argc, argv);
   if(ColorScale.count == 0) 
      set_color_theme("rgb");

   MagickWandGenesis();

   R = loadImage();

   if(output_width > 0) {
      size_t width  = MagickGetImageWidth(R)
	   , height = MagickGetImageHeight(R);
      MagickResizeImage(R, output_width, height * output_width / width, LanczosFilter, 1);
   }


   MagickWriteImage(R, output_file_name);
/*
   readPaths();

   if(paths.npaths > 0) {
      if(animate) {
         animatePaths(&R);
      }
      else {
         drawPaths(&R);
         saveRaster(&R, output_file_name);
      }
   }
   else {
      saveRaster(&R, output_file_name);
   }
*/

   DestroyMagickWand(R);
   return 0;
}