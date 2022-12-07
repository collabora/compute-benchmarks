#include <stdio.h>
#include <stdlib.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
// #include <GL/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl31.h>


typedef EGLBoolean (* m_eglQueryDevicesEXTProc)(EGLint, EGLDeviceEXT *, EGLint *);
typedef const char *(* m_eglQueryDeviceStringEXT)(EGLDeviceEXT device, EGLint name);
typedef EGLDisplay (* m_eglGetPlatformDisplayEXTProc) (EGLenum, void *, const EGLint *);


int setup_gl(void) {
    m_eglQueryDevicesEXTProc eglQueryDevicesEXT = (m_eglQueryDevicesEXTProc)eglGetProcAddress("eglQueryDevicesEXT");
    m_eglQueryDeviceStringEXT eglQueryDeviceStringEXT = (m_eglQueryDeviceStringEXT)eglGetProcAddress("eglQueryDeviceStringEXT");
    m_eglGetPlatformDisplayEXTProc eglGetPlatformDisplayEXT = (m_eglGetPlatformDisplayEXTProc)eglGetProcAddress("eglGetPlatformDisplayEXT");

    EGLint num_devices;
    if (!eglQueryDevicesEXT(0, NULL, &num_devices)) {
        fprintf(stderr, "eglQueryDevicesEXT failed (0x%x)\n", eglGetError());
        return 1;
    }

    fprintf(stderr, "Found %d devices\n", num_devices);

    EGLDeviceEXT* devices = malloc(sizeof(EGLDeviceEXT) * num_devices);
    if (!eglQueryDevicesEXT(num_devices, devices, &num_devices)) {
        fprintf(stderr, "eglQueryDevicesEXT failed (0x%x)", eglGetError());
        free(devices);
        return 1;
    }

    for (int i=0; i<num_devices; i++) {
        EGLDeviceEXT device = devices[i];
        const char* apis = eglQueryDeviceStringEXT(device, EGL_CLIENT_APIS);
        const char* vendor = eglQueryDeviceStringEXT(device, EGL_VENDOR);
        const char* exts = eglQueryDeviceStringEXT(device, EGL_EXTENSIONS);
        fprintf(stderr, "device %d: %s %s %s\n", i, apis, vendor, exts);

    }
    EGLDeviceEXT device = devices[0];
    free(devices);

    EGLDisplay dpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, device, 0);
    if (dpy == EGL_NO_DISPLAY) {
        fprintf(stderr, "eglGetPlatformDisplayEXT failed (0x%x)\n", eglGetError());
        return 1;
    }

    EGLint major, minor;
    if (!eglInitialize(dpy, &major, &minor)) {
        fprintf(stderr, "eglInitialize failed (0x%x)\n", eglGetError());
        return 1;
    }
    fprintf(stderr, "eglInitialize: %d.%d\n", major, minor);

    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
        EGL_NONE
    };

    EGLint num_configs = 0;
    EGLConfig cfg;
    if (!eglChooseConfig(dpy, config_attribs, &cfg, 1, &num_configs)) {
        fprintf(stderr, "eglChooseConfig failed (0x%x)\n", eglGetError());
        return 1;
    }
    // fprintf(stderr, "eglChooseConfig: %d\n", num_configs);

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        fprintf(stderr, "eglBindAPI failed (0x%x)\n", eglGetError());
        return 1;
    }

    int ctxattribs[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE, };
    EGLContext ctx = eglCreateContext(dpy, cfg, NULL, ctxattribs);
    if (!ctx) {
        fprintf(stderr, "eglCreateContext failed (0x%x)\n", eglGetError());
        return 1;
    }
    static const EGLint surfaceAttribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
    EGLSurface mSurface                  = eglCreatePbufferSurface(dpy, cfg, surfaceAttribs);
    if (mSurface == EGL_NO_SURFACE) {
        fprintf(stderr, "eglCreatePbufferSurface error: %x !!! \n", eglGetError());
        return 1;
    }

    if(!eglMakeCurrent(dpy, mSurface, mSurface, ctx)) {
        fprintf(stderr, "eglMakeCurrent error: %x !!! \n", eglGetError());
        return 1;
    }

    int _major, _minor;
    glGetIntegerv(GL_MAJOR_VERSION, &_major);
    glGetIntegerv(GL_MINOR_VERSION, &_minor);
    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *glver = glGetString(GL_VERSION);
    fprintf(stderr, "egl %d.%d, GLES %d.%d\n", major, minor, _major, _minor);
    fprintf(stderr, "%s %s %s\n", vendor, renderer, glver);

    const char *glexts = glGetString(GL_EXTENSIONS);
    // fprintf(stderr, "GL_EXTENSIONS %s\n", glexts);

    // uint mShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    // fprintf(stderr, "shader: %d\n", mShaderId);
    // mShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    // fprintf(stderr, "shader: %d\n", mShaderId);
    // mShaderId = glCreateShader(GL_VERTEX_SHADER);
    // fprintf(stderr, "shader: %d\n", mShaderId);
    // mShaderId = glCreateShader(GL_COMPUTE_SHADER);
    // fprintf(stderr, "shader: %d\n", mShaderId);
    // mShaderId = glCreateShader(GL_COMPUTE_SHADER);
    // fprintf(stderr, "shader: %d\n", mShaderId);
    return 0;
}
