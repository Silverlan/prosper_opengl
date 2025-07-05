// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PROSPER_OPENGL_DEFINITIONS_HPP__
#define __PROSPER_OPENGL_DEFINITIONS_HPP__

#include <prosper_definitions.hpp>

#ifdef SHPROSPER_OPENGL_STATIC
#define DLLPROSPER_GL
#elif SHPROSPER_OPENGL_DLL
#ifdef __linux__
#define DLLPROSPER_GL __attribute__((visibility("default")))
#else
#define DLLPROSPER_GL __declspec(dllexport)
#endif
#else
#ifdef __linux__
#define DLLPROSPER_GL
#else
#define DLLPROSPER_GL __declspec(dllimport)
#endif
#endif

// Note: These have to match the definitions of the OpenGL headers
using GLuint = unsigned int;
using GLint = int;
using GLenum = unsigned int;
using GLboolean = unsigned char;
#define GL_INVALID_VALUE 0x0501
struct __GLsync;
using GLsync = __GLsync *;

#endif
