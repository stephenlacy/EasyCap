#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <gcrypt.h>

struct header {
   uint8_t  magic[2];     // 0c 94
   uint8_t  version[2];   // cc00 / ce00 / d900 / ?
   uint8_t  fill1[40];    // 40 times 0xff
   uint8_t  magic2[2];    // 0c 94
   uint8_t  something[2]; // ?  / 7206 / 0c03 / ?
   uint8_t  fill2[48];    // 48 times 0xff
};

int main(int argc, char **argv) {
   if (argc < 2) {
      printf("Usage: extractor SmiUsbGrabberXX.sys");
      exit(0);
   }

   int infile = open(argv[1], O_RDONLY);
   struct stat stat;
   fstat(infile,&stat);
   unsigned long full_size = stat.st_size;

   printf("Loading file `%s` (%ld bytes)\n", argv[1], full_size);


   int8_t * filestart = (int8_t*)mmap(NULL, full_size, PROT_READ, MAP_PRIVATE, infile, 0);

   unsigned long i = 0;
   while (i < (full_size-sizeof(struct header))) {
      struct header * p = (struct header*)(filestart + i);

      // check that the structure is valid
      int valid = 1;
      if (p->magic[0] != 0x0c || p->magic[1] != 0x94) {
         valid = 0;
      }
      if (valid) {
         if (p->magic2[0] != 0x0c || p->magic2[1] != 0x94) {
            valid = 0;
         }
      }
      if (valid) {
         for (int k=0;k<40; k++) {
            if (p->fill1[k] != 0xff) {
               valid = 0;
               break;
            }
         }
      }
      if (valid) {
         for (int k=0;k <48; k++) {
            if (p->fill2[k] != 0xff) {
               valid = 0;
               break;
            }
         }
      }

      if (!valid) {
         i += 4;
         continue;
      }

      // look for ending: at least 26 times 0xff
      uint8_t * body = (uint8_t*)(filestart + i + sizeof(struct header));
      unsigned int body_size = 0;
      int count_ff = 0;
      while (count_ff < 26) {
         if (body[body_size] == 0xff) {
            count_ff += 1;
         }
         else {
            count_ff = 0;
         }
         body_size += 1;
      }
      // read remaining 0xff's
      while (body[body_size] == 0xff) {
         body_size += 1;
      }

      unsigned int size = sizeof(struct header)+body_size;

      char outname[1000];
      sprintf(outname, "somagic_firmware_%lx.bin", i);
      FILE * outfile = fopen(outname, "w");
      fwrite(p, size, 1, outfile);
      fclose(outfile);

      unsigned char digest[4];
      gcry_md_hash_buffer(GCRY_MD_CRC32, digest, p, size);

      printf("Found firmware at offset 0x%lx (%d bytes): %s\n", i, size, outname);
      printf("  - magic   : %02x %02x / %02x %02x \n", p->version[0], p->version[1], p->something[0], p->something[1]);
      printf("  - checksum: %02x %02x %02x %02x\n", digest[0], digest[1], digest[2], digest[3]);

      // skip header and body
      i += sizeof(struct header)+body_size;

      // align to 32-bit boundaries
      while (i % 4 != 0) i++;
   }

   close(infile);
}
