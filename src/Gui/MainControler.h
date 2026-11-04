#pragma once

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

NS_ASSUME_NONNULL_BEGIN

@interface MainControler : NSObject

@property (nonatomic, assign, readonly, nullable) id<MTLDevice> metalDevice;
@property (nonatomic, assign, readonly, nullable) id<MTLCommandQueue> commandQueue;

- (instancetype) init;

- (void) start;
- (void) stop;

- (void) setAtomCount:(NSInteger)atomCount;
- (void) setFreeLength:(float)freeLength;

- (void) appendLog:(NSString *)message isError:(BOOL)isError;

- (BOOL) setOutputFolder;
- (void) setRecordTrajectory:(BOOL)record;
- (void) setInitialConfiguration:(BOOL)random;

@end

NS_ASSUME_NONNULL_END