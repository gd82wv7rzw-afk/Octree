#pragma once

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

NS_ASSUME_NONNULL_BEGIN

typedef NSNotificationName MainControlerNotification;

FOUNDATION_EXPORT MainControlerNotification const MainControlerParametersDidChangeNotification;

@interface MainControler : NSObject

@property (nonatomic, assign, readonly, nullable) id<MTLDevice> metalDevice;
@property (nonatomic, assign, readonly, nullable) id<MTLCommandQueue> commandQueue;
@property (nonatomic, assign) NSInteger atomCount;
@property (nonatomic, assign) float freeLength;
@property (nonatomic, readonly) BOOL hasValidSetup;

- (instancetype) init;

- (void) generateRandomConfiguration;
- (void) start;
- (void) stop;

- (void) appendLog:(NSString *)message isError:(BOOL)isError;

- (BOOL)newProject;
- (BOOL)loadProject;
- (BOOL)setupTrajectoryRecording;
- (BOOL)selectStartConfiguration;

- (void) setRecordTrajectory:(BOOL)record;
- (void) setInitialConfiguration:(BOOL)random;

@end

NS_ASSUME_NONNULL_END