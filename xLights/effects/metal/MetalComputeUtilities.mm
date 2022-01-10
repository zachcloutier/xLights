
#include "MetalComputeUtilities.hpp"
#include "../../PixelBuffer.h"
#include "../../RenderBuffer.h"

#include <MetalPerformanceShaders/MetalPerformanceShaders.h>

MetalComputeUtilities MetalComputeUtilities::INSTANCE;


MetalPixelBufferComputeData::MetalPixelBufferComputeData() {
}
MetalPixelBufferComputeData::~MetalPixelBufferComputeData() {
}


MetalRenderBufferComputeData::MetalRenderBufferComputeData(RenderBuffer *rb, MetalPixelBufferComputeData *pbd) : renderBuffer(rb), pixelBufferData(pbd) {
    commandBuffer = nil;
    pixelBuffer = nil;
    pixelTexture = nil;
    pixelBufferSize = 0;
}
MetalRenderBufferComputeData::~MetalRenderBufferComputeData() {
    pixelBufferData = nullptr;
    @autoreleasepool {
        if (commandBuffer != nil) {
            [commandBuffer release];
        }
        if (pixelBuffer != nil) {
            [pixelBuffer release];
        }
        if (pixelTexture != nil) {
            [pixelTexture release];
        }
    }
}
id<MTLCommandBuffer> MetalRenderBufferComputeData::getCommandBuffer() {
    if (commandBuffer == nil) {
        commandBuffer = [[MetalComputeUtilities::INSTANCE.commandQueue commandBuffer] retain];
        NSString* mn = [NSString stringWithUTF8String:renderBuffer->GetModelName().c_str()];
        [commandBuffer setLabel:mn];
    }
    return commandBuffer;
}
id<MTLBuffer> MetalRenderBufferComputeData::getPixelBuffer(bool sendToGPU) {
    if (pixelBufferSize < renderBuffer->GetPixelCount()) {
        int bufferSize = renderBuffer->GetPixelCount() * 4;
        id<MTLBuffer> newBuffer = [[MetalComputeUtilities::INSTANCE.device newBufferWithLength:bufferSize options:MTLResourceStorageModeShared] retain];
        std::string name = renderBuffer->GetModelName() + "PixelBuffer";
        NSString* mn = [NSString stringWithUTF8String:name.c_str()];
        [newBuffer setLabel:mn];
        memcpy(newBuffer.contents, renderBuffer->pixels, pixelBufferSize * 4);
        pixelBufferSize = renderBuffer->pixelVector.size();
        pixelBuffer = newBuffer;
        renderBuffer->pixels = static_cast<xlColor*>(pixelBuffer.contents);
        currentDataLocation = BUFFER;
    }
    if (currentDataLocation == TEXTURE) {
        //use GPU to copy over to Buffer
        NSUInteger bytesPerRow = 4 * renderBuffer->BufferWi;
        NSUInteger bytesPerImage = bytesPerRow * renderBuffer->BufferHt;
        MTLSize size = MTLSizeMake(renderBuffer->BufferWi, renderBuffer->BufferHt, 1);
        id <MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
        [blitCommandEncoder setLabel:@"CopyTextureToBuffer"];
        [blitCommandEncoder copyFromTexture:pixelTexture
                                sourceSlice:0
                                sourceLevel:0
                               sourceOrigin:{0,0,0}
                                 sourceSize:size
                                   toBuffer:pixelBuffer
                          destinationOffset:0
                     destinationBytesPerRow:bytesPerRow
                   destinationBytesPerImage:bytesPerImage];
        [blitCommandEncoder endEncoding];
        currentDataLocation = BUFFER;
    }
    return pixelBuffer;
}
id<MTLTexture> MetalRenderBufferComputeData::getPixelTexture() {
    getPixelBuffer(true);
    if (pixelTexture == nil) {
        @autoreleasepool {
            MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: MTLPixelFormatRGBA8Unorm
                                                                                         width: renderBuffer->BufferWi
                                                                                        height: renderBuffer->BufferHt
                                                                                     mipmapped: NO];
            d.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
            d.storageMode = MTLStorageModePrivate;
            // Create the texture from the device by using the descriptor
            pixelTexture = [[MetalComputeUtilities::INSTANCE.device newTextureWithDescriptor:d] retain];
            
            std::string name = renderBuffer->GetModelName() + "PixelTexture";
            NSString* mn = [NSString stringWithUTF8String:name.c_str()];
            [pixelTexture setLabel:mn];
        }
    }
    if (currentDataLocation == BUFFER) {
        NSUInteger bytesPerRow = 4 * renderBuffer->BufferWi;
        NSUInteger bytesPerImage = bytesPerRow * renderBuffer->BufferHt;
        MTLSize size = MTLSizeMake(renderBuffer->BufferWi, renderBuffer->BufferHt, 1);
        id <MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
        [blitCommandEncoder setLabel:@"CopyBufferToTexture"];
        [blitCommandEncoder copyFromBuffer:pixelBuffer
                              sourceOffset:0
                         sourceBytesPerRow:bytesPerRow
                       sourceBytesPerImage:bytesPerImage
                                sourceSize:size
                                 toTexture:pixelTexture
                          destinationSlice:0
                          destinationLevel:0
                         destinationOrigin:{0,0,0}];
        [blitCommandEncoder endEncoding];
        currentDataLocation = TEXTURE;
    }
    return pixelTexture;
}

void MetalRenderBufferComputeData::commit() {
    if (commandBuffer != nil && !committed) {
        @autoreleasepool {
            if (currentDataLocation == TEXTURE) {
                //use GPU to copy over to Buffer
                NSUInteger bytesPerRow = 4 * renderBuffer->BufferWi;
                NSUInteger bytesPerImage = bytesPerRow * renderBuffer->BufferHt;
                MTLSize size = MTLSizeMake(renderBuffer->BufferWi, renderBuffer->BufferHt, 1);
                id <MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
                [blitCommandEncoder setLabel:@"CopyTextureToBufferForCommit"];

                [blitCommandEncoder copyFromTexture:pixelTexture
                                        sourceSlice:0
                                        sourceLevel:0
                                       sourceOrigin:{0,0,0}
                                         sourceSize:size
                                           toBuffer:pixelBuffer
                                  destinationOffset:0
                             destinationBytesPerRow:bytesPerRow
                           destinationBytesPerImage:bytesPerImage];
                [blitCommandEncoder endEncoding];
                currentDataLocation = BUFFER;
            }
        }
        [commandBuffer commit];
        committed = true;
    }
}


void MetalRenderBufferComputeData::waitForCompletion() {
    if (commandBuffer != nil) {
        @autoreleasepool {
            commit();
            [commandBuffer waitUntilCompleted];
            [commandBuffer release];
            commandBuffer = nil;
            committed = false;
        }
    }
}
bool MetalRenderBufferComputeData::blur(int radius) {
    if ((renderBuffer->BufferHt < (radius * 2)) || (renderBuffer->BufferWi < (radius * 2))) {
        // the performance shaders don't handle edges so if less than the radius, bail to the CPU
        return false;
    }
    @autoreleasepool {
        id<MTLCommandBuffer> commandBuffer = getCommandBuffer();
        getPixelTexture();
        
        /*
        float sigma = radius - 1;
        sigma = std::sqrt(sigma);
        MPSImageGaussianBlur* gblur = [[MPSImageGaussianBlur alloc] initWithDevice:MetalComputeUtilities::INSTANCE.device sigma:sigma];
        */
        //tent blur is closest to what is implemented on the C++/CPU side
        float r = (radius - 1) * 2 - 1;
        MPSImageTent *gblur = [[MPSImageTent alloc] initWithDevice:MetalComputeUtilities::INSTANCE.device
                                                     kernelWidth:r
                                                    kernelHeight:r];
        [gblur setLabel:@"Blur"];

        MPSCopyAllocator myAllocator = ^id <MTLTexture>( MPSKernel * __nonnull filter,
                                                        __nonnull id <MTLCommandBuffer> cmdBuf,
                                                        __nonnull id <MTLTexture> sourceTexture)
        {
            MTLPixelFormat format = sourceTexture.pixelFormat;
            MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: format
                                                                                         width: sourceTexture.width
                                                                                        height: sourceTexture.height
                                                                                     mipmapped: NO];
            [sourceTexture release];
            d.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
            return [cmdBuf.device newTextureWithDescriptor: d];
        };
        [commandBuffer pushDebugGroup:@"Blur"];
        BOOL ok = [gblur encodeToCommandBuffer:commandBuffer
                                inPlaceTexture:&pixelTexture
                         fallbackCopyAllocator:myAllocator];
        [commandBuffer popDebugGroup];
        [gblur release];
        return ok;
    }
}

MetalRenderBufferComputeData *MetalRenderBufferComputeData::getMetalRenderBufferComputeData(RenderBuffer *b) {
    return static_cast<MetalRenderBufferComputeData*>(b->gpuRenderData);
}



MetalComputeUtilities::MetalComputeUtilities() {
    enabled = false;
    device = nil;
    
    NSArray *devices = MTLCopyAllDevices();
    for (id d in devices) {
        if ([d isRemovable]) {
            device = [d retain];
        }
    }
    [devices release];
    if (device == nil) {
        device = MTLCreateSystemDefaultDevice();
    }
    if (device.argumentBuffersSupport == MTLArgumentBuffersTier1) {
        device = nil;
        return;
    }

    NSError *libraryError = NULL;
    NSString *libraryFile = [[NSBundle mainBundle] pathForResource:@"EffectComputeFunctions" ofType:@"metallib"];
    if (!libraryFile) {
        NSLog(@"Library file error");
        return;
    }
    library = [device newLibraryWithFile:libraryFile error:&libraryError];
    if (!library) {
        NSLog(@"Library error: %@", libraryError);
        return;
    }
    [library setLabel:@"EffectComputeFunctionsLibrary"];

    commandQueue = [device newCommandQueue];
    if (!commandQueue) {
        return;
    }
    [commandQueue setLabel:@"MetalEffectCommandQueue"];
    enabled = true;
}
MetalComputeUtilities::~MetalComputeUtilities() {
    @autoreleasepool {
        commandQueue = nil;
        library = nil;
        device = nil;
    }
}

id<MTLComputePipelineState> MetalComputeUtilities::FindComputeFunction(const char *name) {
    NSString *fname = @(name);
    id<MTLFunction> function = [library newFunctionWithName:fname];
    NSError *error = NULL;
    
    MTLComputePipelineDescriptor *desc = [MTLComputePipelineDescriptor new];
    desc.computeFunction = function;
    desc.label = fname;
    
    //id<MTLComputePipelineState> ps = [device newComputePipelineStateWithFunction:function error:&error];
    id<MTLComputePipelineState> ps = [device newComputePipelineStateWithDescriptor:desc
                                                                           options:MTLPipelineOptionNone
                                                                        reflection:nil
                                                                             error:&error];
    [function release];
    [desc release];
    if (!ps) {
        NSLog(@"Library error: %@", error);
    }
    return ps;
}




extern "C" {
bool isMetalComputeSupported() {
    return MetalComputeUtilities::INSTANCE.enabled;
}
}

