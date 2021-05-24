#ifdef _WINDOWS
#include <Common/wgl_wrap.hpp>
#define WGL_NV_gpu_affinity
#else
#include<EGL/egl.h>
#include<EGL/eglext.h>

const EGLint egl_config_attribs[] = {EGL_SURFACE_TYPE,
                                     EGL_PBUFFER_BIT,
                                     EGL_BLUE_SIZE,
                                     8,
                                     EGL_GREEN_SIZE,
                                     8,
                                     EGL_RED_SIZE,
                                     8,
                                     EGL_DEPTH_SIZE,
                                     8,
                                     EGL_RENDERABLE_TYPE,
                                     EGL_OPENGL_BIT,
                                     EGL_NONE};



void EGLCheck(const char *fn) {
    EGLint error = eglGetError();

    if (error != EGL_SUCCESS) {
        throw runtime_error(fn + to_string(error));
    }
}
#endif

class GLHandler{
    public:
    static HDC window_handle;
    static HGLRC gl_context;
};
