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


#include "z_zone.h"

#include "v_video.h"
#include "gl_opengl.h"
#include "gl_intern.h"
#include "r_defs.h"
#include "r_bsp.h"
#include "r_draw.h"
#include "r_main.h"
#include "r_things.h"
#include "w_wad.h"
#include "i_system.h"
#include "e6y.h"

#include "gl_opengl.h"
#include "gl_struct.h"

#ifdef USE_SHADERS

GLShader *sh_main = NULL;
static GLShader *active_shader = NULL;

static GLShader* gld_LoadShader(const char *vpname, const char *fpname);

int glsl_Init(void) {
  if (!gl_arb_shader_objects)
    D_Msg(MSG_WARN, "glsl_Init: shaders expects OpenGL 2.0\n");
  else
    sh_main = gld_LoadShader("glvp", "glfp");

  return (sh_main != NULL);
}

static int ReadLump(const char *filename, const char *lumpname, unsigned char **buffer)
{
  FILE *file = NULL;
  int size = 0;
  const unsigned char *data;
  int lump;

  file = fopen(filename, "r");
  if (file)
  {
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    *buffer = malloc(size + 1);
    size = fread(*buffer, 1, size, file);
    if (size > 0)
    {
      (*buffer)[size] = 0;
    }
    fclose(file);
  }
  else
  {
    char name[9];
    char* p;

    strncpy(name, lumpname, 9);
    name[8] = 0;
    for(p = name; *p; p++)
      *p = toupper(*p);

    lump = (W_CheckNumForName)(name, ns_prboom);

    if (lump != -1)
    {
      size = W_LumpLength(lump);
      data = W_CacheLumpNum(lump);
      *buffer = calloc(1, size + 1);
      memcpy (*buffer, data, size);
      (*buffer)[size] = 0;
      W_UnlockLumpNum(lump);
    }
  }

  return size;
}

static GLShader* gld_LoadShader(const char *vpname, const char *fpname)
{
#define buffer_size 2048
  int idx;
  int linked;
  char buffer[buffer_size];
  char *vp_data = NULL;
  char *fp_data = NULL;
  int vp_size, fp_size;
  size_t vp_fnlen, fp_fnlen;
  char *filename = NULL;
  GLShader* shader = NULL;

  vp_fnlen = snprintf(NULL, 0, "%s/shaders/%s.txt", I_DoomExeDir(), vpname);
  fp_fnlen = snprintf(NULL, 0, "%s/shaders/%s.txt", I_DoomExeDir(), fpname);
  filename = malloc(MAX(vp_fnlen, fp_fnlen) + 1);

  sprintf(filename, "%s/shaders/%s.txt", I_DoomExeDir(), vpname);
  vp_size = ReadLump(filename, vpname, (unsigned char **)&vp_data);

  sprintf(filename, "%s/shaders/%s.txt", I_DoomExeDir(), fpname);
  fp_size = ReadLump(filename, fpname, (unsigned char **)&fp_data);
  
  if (vp_data && fp_data)
  {
    shader = calloc(1, sizeof(GLShader));

    shader->hVertProg = GLEXT_glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    shader->hFragProg = GLEXT_glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);	

    GLEXT_glShaderSourceARB(shader->hVertProg, 1, (const GLcharARB **)&vp_data, &vp_size);
    GLEXT_glShaderSourceARB(shader->hFragProg, 1, (const GLcharARB **)&fp_data, &fp_size);

    GLEXT_glCompileShaderARB(shader->hVertProg);
    GLEXT_glCompileShaderARB(shader->hFragProg);

    shader->hShader = GLEXT_glCreateProgramObjectARB();

    GLEXT_glAttachObjectARB(shader->hShader, shader->hVertProg);
    GLEXT_glAttachObjectARB(shader->hShader, shader->hFragProg);

    GLEXT_glLinkProgramARB(shader->hShader);

    GLEXT_glGetInfoLogARB(shader->hShader, buffer_size, NULL, buffer);

    GLEXT_glGetObjectParameterivARB(shader->hShader, GL_OBJECT_LINK_STATUS_ARB, &linked);

    if (linked)
    {
      D_Msg(MSG_INFO,
        "gld_LoadShader: Shader \"%s+%s\" compiled OK: %s\n",
        vpname, fpname, buffer
      );

      shader->lightlevel_index = GLEXT_glGetUniformLocationARB(
        shader->hShader, "lightlevel"
      );

      GLEXT_glUseProgramObjectARB(shader->hShader);

      idx = GLEXT_glGetUniformLocationARB(shader->hShader, "tex");
      GLEXT_glUniform1iARB(idx, 0);

      GLEXT_glUseProgramObjectARB(0);
    }
    else
    {
      D_Msg(MSG_ERROR,
        "gld_LoadShader: Error compiling shader \"%s+%s\": %s\n",
        vpname, fpname, buffer
      );
      free(shader);
      shader = NULL;
    }
  }

  free(filename);
  free(vp_data);
  free(fp_data);

  return shader;
}

void glsl_SetActiveShader(GLShader *shader)
{
  if (gl_lightmode == gl_lightmode_shaders)
  {
    if (shader != active_shader)
    {
      GLEXT_glUseProgramObjectARB((shader ? shader->hShader : 0));
      active_shader = shader;
    }
  }
}

void glsl_SetLightLevel(float lightlevel)
{
  if (sh_main)
  {
    GLEXT_glUniform1fARB(sh_main->lightlevel_index, lightlevel);
  }
}

int glsl_IsActive(void)
{
  return (gl_lightmode == gl_lightmode_shaders && sh_main);
}

#endif // USE_SHADERS

/* vi: set et ts=2 sw=2: */

