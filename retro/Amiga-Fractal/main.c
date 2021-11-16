#include <stdio.h>
#include <time.h>
#include <exec/types.h>
#include <intuition/intuition.h>

#define MAX_ITER 20
#define IMAGE_WIDTH 128
#define IMAGE_HEIGHT 128

struct GfxBase* GfxBase = 0;
struct IntuitionBase *IntuitionBase = 0;

struct Window        *FirstWindow;
struct NewWindow     FirstNewWindow = {
  160,50,
  320,200,
  0, 1,
  CLOSEWINDOW,
  WINDOWDEPTH | WINDOWSIZING | WINDOWDRAG | 
  WINDOWCLOSE | GIMMEZEROZERO | SMART_REFRESH,
  NULL,
  NULL,
  (UBYTE*)"Fractal",
  NULL,
  NULL,
  100,50,
  640,256,
  WBENCHSCREEN
};
struct RastPort *firstRastPort;

USHORT firstImageData[IMAGE_HEIGHT*(IMAGE_WIDTH+15)/16];

struct Image firstImage = {
  0, 0,
  IMAGE_WIDTH, IMAGE_HEIGHT,
  1,
  firstImageData,
  2,0,
  NULL
};

void endProgram(int);

void openIntuition() {
  if (!(IntuitionBase = (struct IntuitionBase*)
        OpenLibrary("intuition.library", NULL))) {
    printf("Could not open intuition library\n");
    endProgram(1);
  }

  if (!(GfxBase = (struct GfxBase*)
        OpenLibrary("graphics.library", NULL))) {
    printf("Could not open gfxbase library\n");
    endProgram(1);
  }

}

void closeIntuition() {
  if (IntuitionBase) CloseLibrary(IntuitionBase);
}

void openWindow() {
  if (!(FirstWindow = (struct Window*)
      OpenWindow(&FirstNewWindow))) {
    printf("Could not open window\n");
    endProgram(2);
  }
  
  firstRastPort = FirstWindow->RPort;
}

void closeWindow() {
  CloseWindow(FirstWindow);
}

void endProgram(int code) {
  closeWindow();
  closeIntuition();
  exit(code); 
}

int computeFractalPixel(int x, int y) {
  FLOAT cr = -2.1f + x*2.6f/IMAGE_WIDTH;
  FLOAT ci = -1.3f + y*2.6f/IMAGE_HEIGHT;
  int iter = 0;
  FLOAT zr=0.0f, zi=0.0f;
  FLOAT tr=0.0f;
  
  while (iter < MAX_ITER && zr*zr+zi*zi < 4) {
    tr = zr*zr - zi*zi + cr;
	zi = zr*zi + zi*zr + ci;
    zr = tr;
    ++iter;
  }
  return iter < MAX_ITER ? 0 : 1;
}

void computeFractal() {
  int xb=0;
  int xs=0;
  int y=0;

  for (y=0;y<IMAGE_HEIGHT;++y) {
    for (xb=0;xb<IMAGE_WIDTH/16;++xb) {
      for (xs=0;xs<16;++xs) {
        int val = computeFractalPixel(xb*16+xs,y);
        firstImageData[y*((IMAGE_WIDTH+15)/16)+xb] |= val << (15-xs);
      }
    }
  }
}

void computeFractalFast() {
  int xb=0;
  int xs=0;
  int y=0;
  
  float dr = 2.6f/IMAGE_WIDTH;
  float di = 2.6f/IMAGE_HEIGHT;
  int blockCount = IMAGE_WIDTH/16;
  
  float ci = -1.3f;
  for (y=0;y<IMAGE_HEIGHT;++y) {  
    float cr = -2.1f;
    for (xb=0;xb<blockCount;++xb) {
      for (xs=0;xs<16;++xs) {
        int iter = 0;
        float zr=0.0f, zi=0.0f;
        float r2=0.0f, i2=0.0f;
        cr += dr;

        while (r2+i2 < 4 && iter < MAX_ITER) {
          zi = (zr+zr)*zi + ci;
          zr = r2 - i2 + cr;
          r2 = zr * zr;
          i2 = zi * zi;
          ++iter;
        }
        firstImageData[y*((IMAGE_WIDTH+15)/16)+xb] |= (iter == MAX_ITER) << (15-xs);
      }
    }
    ci += di;
  }
}

int main(int argc, char** argv) {
  struct IntuiMessage *message;
  ULONG messageClass;
  USHORT code;
  time_t startTime, endTime;
  float duration;
    
  openIntuition();
  openWindow();

  startTime = time(NULL);

  computeFractalFast();
  
  endTime = time(NULL);   
  duration = difftime(endTime, startTime);
  printf("Computation took %3.1f sec.\n",duration);

  DrawImage(firstRastPort, &firstImage, 0, 0); 

  FOREVER {
    if (message = (struct IntuiMessage *) 
      GetMsg(FirstWindow->UserPort)) {
      messageClass = message->Class;      
      ReplyMsg(message);
      switch (messageClass) {
        case CLOSEWINDOW : 
          endProgram(0);
          break;
      }
    }
  }
  
  endProgram(0);
}
