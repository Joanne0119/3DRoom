#pragma once

#define BUFFER_OFFSET(offset) ((GLvoid*)(offset))
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 800

#define VTX_OFFSET 0		
#define COLOR_OFFSET 3		
#define NORMAL_OFFSET 6		
#define TEXCOORD_OFFSET 9	

#define SAFE_DELETE(p) { if(p) { delete (p); (p) = nullptr; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p); (p) = nullptr; } }


#ifndef M_PI
#define M_PI  3.1415926f
#endif

#ifndef M_PI_2
#define M_PI_2  1.5707963f
#endif