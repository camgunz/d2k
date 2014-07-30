/*****************************************************************************/
/* D2K: A Doom Source Port for the 21st Century                              */
/*                                                                           */
/* Copyright (C) 2014: See COPYRIGHT file                                    */
/*                                                                           */
/* This file is part of D2K.                                                 */
/*                                                                           */
/* D2K is free software: you can redistribute it and/or modify it under the  */
/* terms of the GNU General Public License as published by the Free Software */
/* Foundation, either version 2 of the License, or (at your option) any      */
/* later version.                                                            */
/*                                                                           */
/* D2K is distributed in the hope that it will be useful, but WITHOUT ANY    */
/* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS */
/* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more    */
/* details.                                                                  */
/*                                                                           */
/* You should have received a copy of the GNU General Public License along   */
/* with D2K.  If not, see <http://www.gnu.org/licenses/>.                    */
/*                                                                           */
/* vi: set et ts=2 sw=2:                                                     */
/*                                                                           */
/*****************************************************************************/

#include "z_zone.h"

#include "gl_opengl.h"

#include <SDL.h>

#include "gl_opengl.h"
#include "gl_intern.h"

#include "i_main.h"
#include "lprintf.h"

dboolean gl_use_FBO = false;

#ifdef USE_FBO_TECHNIQUE
GLuint glSceneImageFBOTexID = 0;
GLuint glDepthBufferFBOTexID = 0;
GLuint glSceneImageTextureFBOTexID = 0;
int SceneInTexture = false;
static dboolean gld_CreateScreenSizeFBO(void);
#endif

//e6y: motion bloor
int gl_motionblur;
int gl_use_motionblur = false;
motion_blur_params_t motion_blur;

#ifdef USE_FBO_TECHNIQUE

void gld_InitMotionBlur(void);

void gld_InitFBO(void)
{
  gld_FreeScreenSizeFBO();

  gl_use_motionblur = gl_arb_framebuffer_object && gl_motionblur && gl_ext_blend_color;

  gl_use_FBO = (gl_arb_framebuffer_object) && (gl_version >= OPENGL_VERSION_1_3) &&
    (gl_use_motionblur || !gl_boom_colormaps || gl_has_hires);

  if (gl_use_FBO)
  {
    if (gld_CreateScreenSizeFBO())
    {
      // motion blur setup
      gld_InitMotionBlur();
    }
    else
    {
      gld_FreeScreenSizeFBO();
      gl_use_FBO = false;
      gl_arb_framebuffer_object = false;
    }
  }
}

static dboolean gld_CreateScreenSizeFBO(void)
{
  int status = 0;
  GLenum internalFormat;
  dboolean attach_stencil = gl_ext_packed_depth_stencil;// && (gl_has_hires || gl_use_motionblur);

  if (!gl_arb_framebuffer_object)
    return false;

  GLEXT_glGenFramebuffersEXT(1, &glSceneImageFBOTexID);
  GLEXT_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, glSceneImageFBOTexID);

  GLEXT_glGenRenderbuffersEXT(1, &glDepthBufferFBOTexID);
  GLEXT_glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, glDepthBufferFBOTexID);

  internalFormat = (attach_stencil ? GL_DEPTH_STENCIL_EXT : GL_DEPTH_COMPONENT);
  GLEXT_glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internalFormat, SCREENWIDTH, SCREENHEIGHT);
  
  // attach a renderbuffer to depth attachment point
  GLEXT_glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, glDepthBufferFBOTexID);

  if (attach_stencil)
  {
    // attach a renderbuffer to stencil attachment point
    GLEXT_glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, glDepthBufferFBOTexID);
  }
  
  glGenTextures(1, &glSceneImageTextureFBOTexID);
  glBindTexture(GL_TEXTURE_2D, glSceneImageTextureFBOTexID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCREENWIDTH, SCREENHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  
  // e6y
  // Some ATI’s drivers have a bug whereby adding the depth renderbuffer
  // and then a texture causes the application to crash.
  // This should be kept in mind when doing any FBO related work and
  // tested for as it is possible it could be fixed in a future driver revision
  // thus rendering the problem non-existent.
  PRBOOM_TRY(EXEPTION_glFramebufferTexture2DEXT)
  {
    GLEXT_glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, glSceneImageTextureFBOTexID, 0);
    status = GLEXT_glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  }
  PRBOOM_EXCEPT(EXEPTION_glFramebufferTexture2DEXT)

  if (status == GL_FRAMEBUFFER_COMPLETE_EXT)
  {
    GLEXT_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  }
  else
  {
    lprintf(LO_ERROR, "gld_CreateScreenSizeFBO: Cannot create framebuffer object (error code: %d)\n", status);
  }

  return (status == GL_FRAMEBUFFER_COMPLETE_EXT);
}

void gld_FreeScreenSizeFBO(void)
{
  if (!gl_arb_framebuffer_object)
    return;

  GLEXT_glDeleteFramebuffersEXT(1, &glSceneImageFBOTexID);
  glSceneImageFBOTexID = 0;

  GLEXT_glDeleteRenderbuffersEXT(1, &glDepthBufferFBOTexID);
  glDepthBufferFBOTexID = 0;

  glDeleteTextures(1, &glSceneImageTextureFBOTexID);
  glSceneImageTextureFBOTexID = 0;
}

void gld_InitMotionBlur(void)
{
  if (gl_use_motionblur)
  {
    float f;

    sscanf(motion_blur.str_min_speed, "%f", &f);
    motion_blur.minspeed_pow2 = f * f;
    
    sscanf(motion_blur.str_min_angle, "%f", &f);
    motion_blur.minangle = (int)(f * 65536.0f / 360.0f);

    sscanf(motion_blur.str_att_a, "%f", &motion_blur.att_a);
    sscanf(motion_blur.str_att_b, "%f", &motion_blur.att_b);
    sscanf(motion_blur.str_att_c, "%f", &motion_blur.att_c);
  }
}
#endif
