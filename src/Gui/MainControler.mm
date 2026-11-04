#import "MainControler.h"
#include "Simulation/MetalGravity.h"
#include "Gui/GuiParameter.h"

#include <Metal/Metal.hpp>
#include <algorithm>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <cstdio>
#include <string>

#import <AppKit/AppKit.h>

static void readLogPipe(int fd, MainControler *controller, BOOL isError)
{
    char buffer[1024];
    std::string pending;

    ssize_t bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
    {
        pending.append(buffer, bytesRead);

        size_t pos;

        while ((pos = pending.find('\n')) != std::string::npos)
        {
            std::string line = pending.substr(0, pos);
            pending.erase(0, pos + 1);

            NSString *message = [[NSString alloc]
                initWithBytes:line.data()
                length:line.size()
                encoding:NSUTF8StringEncoding];

            if (message)
            {
                [controller appendLog:message isError:isError];
                [message release];
            }
        }
    }

    if (!pending.empty())
    {
        NSString *message = [[NSString alloc]
            initWithBytes:pending.data()
            length:pending.size()
            encoding:NSUTF8StringEncoding];

        if (message)
        {
            [controller appendLog:message isError:isError];
            [message release];
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

@implementation MainControler 
{
    id<MTLDevice> _metalDevice;
    MTL::Device* _cppDevice;
    
    id<MTLCommandQueue> _metalCommandQueue;
    MTL::CommandQueue* _commandQueue;

    std::atomic_bool _isRunning;
    std::thread _worker;
    Gui::GuiParameter _parameters;

    int stdoutPipe[2];
    int stderrPipe[2];

    int savedStdout;
    int savedStderr;

    std::thread stdoutThread;
    std::thread stderrThread;

    std::atomic<bool> isLogging;

    std::unique_ptr<Simulation::MetalGravity> _metalSimulation;
}
//----------------------------------------------------------------------------------------------------------------------

@synthesize metalDevice = _metalDevice;
@synthesize commandQueue = _metalCommandQueue;
//----------------------------------------------------------------------------------------------------------------------

- (instancetype)init 
{
    self = [super init];

    if (self == nil) { return self; }

    NSString *savedPath = [[NSUserDefaults standardUserDefaults] stringForKey:@"OutputFolder"];

    BOOL isDirectory = NO;

    if (savedPath != nil && 
        [[NSFileManager defaultManager] fileExistsAtPath:savedPath isDirectory:&isDirectory] &&
        isDirectory)
    {
        _parameters.outputFolder = savedPath.UTF8String;
    }
    else
    {
        _parameters.outputFolder = [NSHomeDirectory()stringByAppendingPathComponent:@"Desktop"].UTF8String;
    }

    _cppDevice = MTL::CreateSystemDefaultDevice();
    if (_cppDevice == nullptr) 
    { 
        NSLog(@"Metal device unavailable");
        return self; 
    }

    // Es ist dasselbe Metal-Device, einmal als Metal-cpp und einmal für Swift/MTKView.
    _metalDevice = (__bridge id<MTLDevice>)_cppDevice;

    _commandQueue = _cppDevice->newCommandQueue();
    if (_commandQueue == nullptr)
    {
        NSLog(@"Metal command queue unavailable");
        return self;
    }

    _metalCommandQueue = (__bridge id<MTLCommandQueue>)_commandQueue;

    // we start with no running simulation
    _isRunning.store(false);

    isLogging.store(false);

    stdoutPipe[0] = stdoutPipe[1] = -1;
    stderrPipe[0] = stderrPipe[1] = -1;

    savedStdout = -1;
    savedStderr = -1;

    [self startLogCapture];

    // simulation
    //printf("natoms: %i\t free length: %f\n", _parameters.nAtoms, _parameters.freeLength);
    _metalSimulation = std::make_unique<Simulation::MetalGravity>(_cppDevice, _commandQueue, _parameters.nAtoms, _parameters.freeLength);

    return self;
}
//----------------------------------------------------------------------------------------------------------------------

- (void)start
{
    // Hier später die C++-Simulation mit atomCount starten.
    // TODO take the parameter from the gui
    //
    if (_worker.joinable()) return;

    auto* simulation = _metalSimulation.get();

    _worker = std::thread([simulation]()
    {
        printf("Worker started\n");
        fflush(stdout);

        simulation->generateRandomConfiguration();
 
        simulation->start();
 
        printf("Worker stopped\n");
        fflush(stdout);
    });

    printf("MainControler: Start\n");
}
//----------------------------------------------------------------------------------------------------------------------

- (void)stop 
{
    // Hier später die C++-Simulation stoppen.
    _metalSimulation->stop();
    
    if (_worker.joinable())
    {
        _worker.join();
    }

    printf("MainControler: Stop\n");
}
//----------------------------------------------------------------------------------------------------------------------

- (void)dealloc 
{
    [self stopLogCapture];

    _metalSimulation.reset();

    if (_commandQueue != nullptr) 
    {
        _commandQueue->release();
    }

    if (_cppDevice != nullptr) 
    {
        _cppDevice->release();
    }

    [super dealloc];
}
//----------------------------------------------------------------------------------------------------------------------

- (void) setAtomCount:(NSInteger)atomCount
{
    _parameters.nAtoms = static_cast<int>(atomCount);
    printf("Atom Count: %d\n", _parameters.nAtoms);
}
//----------------------------------------------------------------------------------------------------------------------

- (void) setFreeLength:(float)freeLength
{
    _parameters.freeLength = freeLength;
}
//----------------------------------------------------------------------------------------------------------------------

- (void)appendLog:(NSString *)message isError:(BOOL)isError
{
    NSString *messageCopy = [message copy];

    dispatch_async(dispatch_get_main_queue(), ^
    {
        [[NSNotificationCenter defaultCenter]
            postNotificationName:@"MainControlerDidAppendLog"
            object:nil
            userInfo:@{
                @"message": messageCopy,
                @"isError": @(isError)
            }];

        [messageCopy release];
    });
}
//----------------------------------------------------------------------------------------------------------------------

- (void)startLogCapture
{
    if (isLogging.load()) return;

    fflush(stdout);
    fflush(stderr);

    if (pipe(stdoutPipe) == -1)
    {
        [self cleanupLogCapture];
        return;
    }

    if (pipe(stderrPipe) == -1)
    {
        [self cleanupLogCapture];
        return;
    }

    savedStdout = dup(STDOUT_FILENO);
    savedStderr = dup(STDERR_FILENO);

    if (savedStdout == -1 || savedStderr == -1)
    {
        [self cleanupLogCapture];
        return;
    }

    if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1)
    {
        [self cleanupLogCapture];
        return;
    }

    if (dup2(stderrPipe[1], STDERR_FILENO) == -1)
    {
        dup2(savedStdout, STDOUT_FILENO);
        [self cleanupLogCapture];
        return;
    }

    close(stdoutPipe[1]);
    close(stderrPipe[1]);

    stdoutPipe[1] = -1;
    stderrPipe[1] = -1;

    setvbuf(stdout, nullptr, _IOLBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    isLogging.store(true);

    stdoutThread = std::thread(readLogPipe, stdoutPipe[0], self, NO);

    stderrThread = std::thread(readLogPipe, stderrPipe[0], self, YES);
}
//----------------------------------------------------------------------------------------------------------------------

- (void)stopLogCapture
{
    if (!isLogging.load()) return;

    fflush(stdout);
    fflush(stderr);

    dup2(savedStdout, STDOUT_FILENO);
    dup2(savedStderr, STDERR_FILENO);

    close(savedStdout);
    close(savedStderr);

    savedStdout = -1;
    savedStderr = -1;

    stdoutThread.join();
    stderrThread.join();

    close(stdoutPipe[0]);
    close(stderrPipe[0]);

    stdoutPipe[0] = -1;
    stderrPipe[0] = -1;

    isLogging.store(false);
}
//----------------------------------------------------------------------------------------------------------------------

- (void)cleanupLogCapture
{
    for (int i = 0; i < 2; ++i)
    {
        if (stdoutPipe[i] != -1)
        {
            close(stdoutPipe[i]);
            stdoutPipe[i] = -1;
        }

        if (stderrPipe[i] != -1)
        {
            close(stderrPipe[i]);
            stderrPipe[i] = -1;
        }
    }

    if (savedStdout != -1)
    {
        close(savedStdout);
        savedStdout = -1;
    }

    if (savedStderr != -1)
    {
        close(savedStderr);
        savedStderr = -1;
    }
}
//----------------------------------------------------------------------------------------------------------------------

- (BOOL)setOutputFolder
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];

    panel.canChooseFiles = NO;
    panel.canChooseDirectories = YES;
    panel.canCreateDirectories = YES;
    panel.allowsMultipleSelection = NO;

    panel.directoryURL = [NSURL fileURLWithPath: [NSString stringWithUTF8String:_parameters.outputFolder.c_str()]];

    if ([panel runModal] == NSModalResponseOK)
    {
        _parameters.outputFolder = panel.URL.path.UTF8String;

        [[NSUserDefaults standardUserDefaults]
            setObject:panel.URL.path
            forKey:@"OutputFolder"];

        printf("Output folder: %s\n", _parameters.outputFolder.c_str());

        return YES;
    }

    return NO;
}
//----------------------------------------------------------------------------------------------------------------------

- (void)setRecordTrajectory:(BOOL)record
{
    _parameters.recordTrajectory = record;
}
//----------------------------------------------------------------------------------------------------------------------

- (void)setInitialConfiguration:(BOOL)random
{
    _parameters.randomConfiguration = random;
    printf("Random configuration: %s\n", _parameters.randomConfiguration ? "YES" : "NO");
}
//----------------------------------------------------------------------------------------------------------------------
@end