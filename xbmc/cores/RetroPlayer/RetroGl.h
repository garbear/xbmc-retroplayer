#ifndef RETRO_GL
#define RETRO_GL

class CRetroGlRenderPicture
{
public:
  CRetroGlRenderPicture(){}
  int texWidth, texHeight;
  GLuint texture[1];
  GLuint depth[1];
};

#endif
