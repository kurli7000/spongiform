#define BYTE  unsigned char
#define DWORD  unsigned int
#define LONG  int
#define UINT  unsigned int
#define WORD  unsigned short int
 
#define LPSTR  char*
#define BI_RGB        0L

#pragma pack(1)
typedef struct tagBITMAPFILEHEADER {
  WORD    bfType;
  DWORD   bfSize;
  WORD    bfReserved1;
  WORD    bfReserved2;
  DWORD   bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;
 
typedef struct tagBITMAPINFOHEADER{
  DWORD  biSize;
  LONG   biWidth;
  LONG   biHeight;
  WORD   biPlanes;
  WORD   biBitCount;
  DWORD  biCompression;
  DWORD  biSizeImage;
  LONG   biXPelsPerMeter;
  LONG   biYPelsPerMeter;
  DWORD  biClrUsed;
  DWORD  biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
 
typedef struct tagRGBQUAD {
  BYTE    rgbBlue;
  BYTE    rgbGreen;
  BYTE    rgbRed;
  BYTE    rgbReserved;
} RGBQUAD;
#pragma pack() 


void writeBmp(const char* filename, int w, int h)
{
  FILE *f;
  BITMAPINFOHEADER infoHeader;
  BITMAPFILEHEADER fileHeader;
  unsigned char* img = malloc(w * h * 3);
  int i;
  
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  memset(img, 0, w*h*3);
  glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, img);

  f = fopen(filename, "wb");

  // Define whats going in the infoHeader.
  infoHeader.biSize = sizeof(BITMAPINFOHEADER);
  infoHeader.biPlanes        = 1;
  infoHeader.biBitCount      = 24;
  infoHeader.biCompression   = BI_RGB;
  infoHeader.biSizeImage     = w*h*3;
  infoHeader.biXPelsPerMeter = 0;
  infoHeader.biYPelsPerMeter = 0;
  infoHeader.biClrUsed       = 0;
  infoHeader.biClrImportant  = 0;
  infoHeader.biWidth         = w;
  infoHeader.biHeight        = h;

  // Define whats going in the file header.
  fileHeader.bfSize       = sizeof(BITMAPFILEHEADER);
  fileHeader.bfType       = 0x4D42;
  fileHeader.bfReserved1  = 0;
  fileHeader.bfReserved2  = 0;
  fileHeader.bfOffBits    = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

  for (i = 0; i < infoHeader.biSizeImage; i += 3) {
    unsigned char swap;

    swap = img[i + 0];
    img[i + 0] = img[i + 2];
    img[i + 2]=swap;
  }

  fwrite(&fileHeader, 1, sizeof(BITMAPFILEHEADER), f);
  fwrite(&infoHeader, 1, sizeof(BITMAPINFOHEADER), f);
  fwrite(img, 1, infoHeader.biSizeImage, f);
  fclose(f);

  free(img);
}

