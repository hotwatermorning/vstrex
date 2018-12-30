#import <Foundation/Foundation.h>
#include <AppKit/NSOpenGL.h>
#include "./PluginEditor.h"

void setOpenGLContextSurfaceOpacityToZero(OpenGLContext *context)
{
    NSOpenGLContext* nscontext = (NSOpenGLContext*)context->getRawContext();
    
    GLint aValue = 0;
    
    [nscontext setValues:&aValue forParameter:NSOpenGLCPSurfaceOpacity];
}
