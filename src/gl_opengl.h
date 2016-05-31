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
/*****************************************************************************/


#ifndef GL_OPENGL_H__
#define GL_OPENGL_H__

//#define USE_OPENGLES2
#define USE_VERTEX_ARRAYS
#define USE_VBO
#define USE_SHADERS

#include <SDL.h>

#ifdef USE_OPENGLES2
#include <GLES2/gl2.h>
#include <GL/glext.h>
#undef USE_GLU_MIPMAP
#undef USE_GLU_TESS
#undef USE_GLU_IMAGESCALE
#else
#include <SDL_opengl.h>
#endif

#if !defined(GL_DEPTH_STENCIL_EXT)
#define GL_DEPTH_STENCIL_EXT              0x84F9
#endif

#define isExtensionSupported(ext) strstr(extensions, ext)

//e6y: OpenGL version
typedef enum {
  OPENGL_VERSION_1_0,
  OPENGL_VERSION_1_1,
  OPENGL_VERSION_1_2,
  OPENGL_VERSION_1_3,
  OPENGL_VERSION_1_4,
  OPENGL_VERSION_1_5,
  OPENGL_VERSION_2_0,
  OPENGL_VERSION_2_1,
} glversion_t;

extern int gl_version;

extern int GLEXT_CLAMP_TO_EDGE;
extern int gl_max_texture_size;

extern SDL_PixelFormat RGBAFormat;

extern GLuint* last_glTexID;

// obsolete?
extern int gl_use_paletted_texture;
extern int gl_use_shared_texture_palette;
extern int gl_paletted_texture;
extern int gl_shared_texture_palette;

extern bool gl_ext_texture_filter_anisotropic;
extern bool gl_arb_texture_non_power_of_two;
extern bool gl_arb_multitexture;
extern bool gl_arb_texture_compression;
extern bool gl_arb_framebuffer_object;
extern bool gl_ext_packed_depth_stencil;
extern bool gl_ext_blend_color;
extern bool gl_use_stencil;
extern bool gl_ext_arb_vertex_buffer_object;
extern bool gl_arb_pixel_buffer_object;
extern bool gl_arb_shader_objects;

// obsolete?
extern PFNGLCOLORTABLEEXTPROC              GLEXT_glColorTableEXT;

extern PFNGLBINDFRAMEBUFFEREXTPROC         GLEXT_glBindFramebufferEXT;
extern PFNGLGENFRAMEBUFFERSEXTPROC         GLEXT_glGenFramebuffersEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC        GLEXT_glGenRenderbuffersEXT;
extern PFNGLBINDRENDERBUFFEREXTPROC        GLEXT_glBindRenderbufferEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC     GLEXT_glRenderbufferStorageEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC GLEXT_glFramebufferRenderbufferEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    GLEXT_glFramebufferTexture2DEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  GLEXT_glCheckFramebufferStatusEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC      GLEXT_glDeleteFramebuffersEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC     GLEXT_glDeleteRenderbuffersEXT;
extern PFNGLGENERATEMIPMAPEXTPROC          GLEXT_glGenerateMipmapEXT;

/* ARB_multitexture command function pointers */
extern PFNGLACTIVETEXTUREARBPROC           GLEXT_glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC     GLEXT_glClientActiveTextureARB;
extern PFNGLMULTITEXCOORD2FARBPROC         GLEXT_glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC        GLEXT_glMultiTexCoord2fvARB;

extern PFNGLBLENDCOLOREXTPROC              GLEXT_glBlendColorEXT;

/* ARB_texture_compression */
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC    GLEXT_glCompressedTexImage2DARB;

/* VBO */
extern PFNGLGENBUFFERSARBPROC              GLEXT_glGenBuffersARB;
extern PFNGLDELETEBUFFERSARBPROC           GLEXT_glDeleteBuffersARB;
extern PFNGLBINDBUFFERARBPROC              GLEXT_glBindBufferARB;
extern PFNGLBUFFERDATAARBPROC              GLEXT_glBufferDataARB;

/* PBO */
extern PFNGLBUFFERSUBDATAARBPROC           GLEXT_glBufferSubDataARB;
extern PFNGLGETBUFFERPARAMETERIVARBPROC    GLEXT_glGetBufferParameterivARB;
extern PFNGLMAPBUFFERARBPROC               GLEXT_glMapBufferARB;
extern PFNGLUNMAPBUFFERARBPROC             GLEXT_glUnmapBufferARB;

/* GL_ARB_shader_objects */
#ifdef USE_SHADERS
extern PFNGLDELETEOBJECTARBPROC            GLEXT_glDeleteObjectARB;
extern PFNGLGETHANDLEARBPROC               GLEXT_glGetHandleARB;
extern PFNGLDETACHOBJECTARBPROC            GLEXT_glDetachObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC      GLEXT_glCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC            GLEXT_glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC           GLEXT_glCompileShaderARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC     GLEXT_glCreateProgramObjectARB;
extern PFNGLATTACHOBJECTARBPROC            GLEXT_glAttachObjectARB;
extern PFNGLLINKPROGRAMARBPROC             GLEXT_glLinkProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC        GLEXT_glUseProgramObjectARB;
extern PFNGLVALIDATEPROGRAMARBPROC         GLEXT_glValidateProgramARB;

extern PFNGLUNIFORM1FARBPROC               GLEXT_glUniform1fARB;
extern PFNGLUNIFORM2FARBPROC               GLEXT_glUniform2fARB;
extern PFNGLUNIFORM1IARBPROC               GLEXT_glUniform1iARB;

extern PFNGLGETOBJECTPARAMETERFVARBPROC     GLEXT_glGetObjectParameterfvARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC     GLEXT_glGetObjectParameterivARB;
extern PFNGLGETINFOLOGARBPROC               GLEXT_glGetInfoLogARB;
extern PFNGLGETATTACHEDOBJECTSARBPROC       GLEXT_glGetAttachedObjectsARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC       GLEXT_glGetUniformLocationARB;
extern PFNGLGETACTIVEUNIFORMARBPROC         GLEXT_glGetActiveUniformARB;
extern PFNGLGETUNIFORMFVARBPROC             GLEXT_glGetUniformfvARB;
#endif
  
void gld_InitOpenGL(bool compatibility_mode);

//states
void gld_EnableTexture2D(GLenum texture, int enable);
void gld_EnableClientCoordArray(GLenum texture, int enable);
void gld_EnableMultisample(int enable);

typedef enum
{
  TMF_MASKBIT = 1,
  TMF_OPAQUEBIT = 2,
  TMF_INVERTBIT = 4,

  TM_MODULATE = 0,
  TM_MASK = TMF_MASKBIT,
  TM_OPAQUE = TMF_OPAQUEBIT,
  TM_INVERT = TMF_INVERTBIT,
  //TM_INVERTMASK = TMF_MASKBIT | TMF_INVERTBIT
  TM_INVERTOPAQUE = TMF_INVERTBIT | TMF_OPAQUEBIT,
} tex_mode_e;
void SetTextureMode(tex_mode_e type);

void gld_Precache(void);

#endif // _GL_OPENGL_H

/* vi: set et ts=2 sw=2: */

