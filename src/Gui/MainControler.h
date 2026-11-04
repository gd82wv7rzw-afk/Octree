#pragma once

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

NS_ASSUME_NONNULL_BEGIN

@interface MainControler : NSObject

@property (nonatomic, assign, readonly, nullable) id<MTLDevice> metalDevice;
@property (nonatomic, assign, readonly, nullable) id<MTLCommandQueue> commandQueue;
@property (nonatomic, assign) NSInteger atomCount;
@property (nonatomic, assign) float freeLength;

- (instancetype) init;

- (void) start;
- (void) stop;

- (void) appendLog:(NSString *)message isError:(BOOL)isError;

- (BOOL)newProject;
- (BOOL)loadProject;
- (BOOL)setupTrajectoryRecording;

- (void) setRecordTrajectory:(BOOL)record;
- (void) setInitialConfiguration:(BOOL)random;

@end

NS_ASSUME_NONNULL_END