
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <petsc.h>

#include "habitat.h"
#include "util.h"

static char *get_file_handle(const char *filename, long *fsz)
{
   int     fd;
   void   *handle;

   if(!file_exists(filename)) {
      fprintf(stderr, "%s does not exist\n", filename);
      exit(1);
   }
   *fsz = file_size(filename);
   fd = open(filename, O_RDONLY);
   assert(fd > 0);
   handle = mmap(NULL, *fsz, PROT_READ, MAP_PRIVATE, fd, 0);
   if(handle == MAP_FAILED) {
      fprintf(stderr, "Error mapping %s to memory", filename);
      exit(1);
   }
   close(fd);  /* man page says this is safe */
   return (char *)handle;
}

static void allocate_cells(struct ResistanceGrid *R)
{
   int i;
   struct RCell *data;
   PetscMalloc(sizeof(struct RCell) * R->nrows * R->ncols, &data);
   PetscMalloc(sizeof(struct RCell *) * R->nrows, &R->cells);
   for(i = 0; i < R->nrows; i++) {
      R->cells[i] = &data[i * R->ncols];
   }
}

static char *parse_header(struct ResistanceGrid *R, char *grid)
{
   char key[32];
   int  bytes;

   while(isalpha(grid[0])) {
       sscanf(grid, "%s%n", key, &bytes);
       grid += bytes;
   
       if(streq(key,"ncols"))
         sscanf(grid, "%d%n", &R->ncols, &bytes);
       else if(streq(key,"nrows"))
         sscanf(grid, "%d%n", &R->nrows, &bytes);
       else if(streq(key,"xllcorner"))
         sscanf(grid, "%lf%n", &R->xllcorner, &bytes);
       else if(streq(key,"yllcorner"))
         sscanf(grid, "%lf%n", &R->yllcorner, &bytes);
       else if(streq(key,"cellsize"))
         sscanf(grid, "%lf%n", &R->cellsize, &bytes);
       else if(streq(key,"NODATA_value"))
         sscanf(grid, "%lf%n", &R->NODATA_value, &bytes);
       grid += bytes;

       while(isspace(grid[0]))
          ++grid;
   }
   message("(rows,cols) = (%d,%d)\n", R->nrows, R->ncols);
   return grid;
}

void parse_habitat_file(struct ResistanceGrid *R, const char *habitat_file)
{
   long   fsz;
   char  *habitat;
   char  *p, *q;
   int    i, j;

   habitat = get_file_handle(habitat_file, &fsz);
   p = parse_header(R, habitat);
   allocate_cells(R);

   R->cell_count = 0;
   for(i = 0; i < R->nrows; i++) {
      for(j = 0; j < R->ncols; j++) {
         R->cells[i][j].value = strtod(p, &q);
         // if(R->cells[i][j].value != R->NODATA_value && R->cells[i][j].value != 0.) {
         if(R->cells[i][j].value > 0) {
            R->cells[i][j].index = R->cell_count++;
         }
         else {
            R->cells[i][j].index = -1;
         }
         p = q;
      }
   }
   munmap(habitat, fsz);
}

void free_habitat(struct ResistanceGrid *R)
{
   PetscFree(R->cells[0]);
   PetscFree(R->cells);
}
