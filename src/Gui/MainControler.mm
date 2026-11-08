#import "MainControler.h"

#include "Simulation/MetalGravity.h"
#include "Gui/GuiParameter.h"
#include "Rendering/Renderer.h"

#include <Metal/Metal.hpp>
#include <algorithm>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <cerrno>

#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
//#import <MetalKit/MetalKit.h>

MainControlerNotification const MainControlerParametersDidChangeNotification = @"MainControlerParametersDidChangeNotification";

static bool writeAll(int fd, const char* data, size_t size)
{
    size_t written = 0;

    while (written < size)
    {
        ssize_t result = write(
            fd,
            data + written,
            size - written
        );

        if (result > 0)
        {
            written += static_cast<size_t>(result);
        }
        else if (result == -1 && errno == EINTR)
        {
            continue;
        }
        else
        {
            return false;
        }
    }

    return true;
}
//----------------------------------------------------------------------------------------------------------------------

static void readLogPipe(
    int fd,
    int consoleFd,
    MainControler* controller,
    BOOL isError)
{
    char buffer[4096];
    std::string pending;

    ssize_t bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
    {
        // 1. Zur ursprünglichen Konsole schreiben
        writeAll(
            consoleFd,
            buffer,
            static_cast<size_t>(bytesRead)
        );

        // 2. Für die GUI sammeln
        pending.append(
            buffer,
            static_cast<size_t>(bytesRead)
        );

        size_t pos;

        while ((pos = pending.find('\n')) != std::string::npos)
        {
            std::string line = pending.substr(0, pos);
            pending.erase(0, pos + 1);

            NSString* message = [[NSString alloc]
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

    // Letzten Rest ohne Zeilenumbruch ausgeben
    if (!pending.empty())
    {
        NSString* message = [[NSString alloc]
            initWithBytes:pending.data()
            length:pending.size()
            encoding:NSUTF8StringEncoding];

        if (message)
        {
            [controller appendLog:message isError:isError];
            [message release];
        }
    }

    close(fd);
}
//----------------------------------------------------------------------------------------------------------------------

@implementation MainControler 
{
    id<MTLDevice> _metalDevice;
    MTL::Device* _cppDevice;
    
    id<MTLCommandQueue> _metalCommandQueue;
    MTL::CommandQueue* _commandQueue;

    std::unique_ptr<Rendering::Renderer> _renderer;

    std::atomic_bool _isRunning;
    std::thread _worker;
    Gui::GuiParameter _parameters;

    std::atomic_bool _hasValidSetup; 

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

    // set directories
    NSString *savedPath = [[NSUserDefaults standardUserDefaults]
    stringForKey:@"ProjectFolder"];

    BOOL isDirectory = NO;

    if (savedPath != nil &&
        [[NSFileManager defaultManager] fileExistsAtPath:savedPath isDirectory:&isDirectory] &&
        isDirectory)
    {
        _parameters.fileNames.outputFolder = savedPath.UTF8String;
    }
    else
    {
        NSString *root = [NSHomeDirectory()
        stringByAppendingPathComponent:@"Desktop"];

        _parameters.fileNames.defaultRootFolder = root.UTF8String;

        NSString *project = [NSString stringWithUTF8String:_parameters.fileNames.defaultProjectName.c_str()];

        _parameters.fileNames.outputFolder = [[root stringByAppendingPathComponent:project] UTF8String];
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

    // at the beginning we dusn't hav a valid setup
    _hasValidSetup.store(false);

    stdoutPipe[0] = stdoutPipe[1] = -1;
    stderrPipe[0] = stderrPipe[1] = -1;

    savedStdout = -1;
    savedStderr = -1;

    [self startLogCapture];

    // simulation
    _metalSimulation = std::make_unique<Simulation::MetalGravity>(_cppDevice, _commandQueue);

    // renderer
    _renderer = std::make_unique<Rendering::Renderer>(_cppDevice, _commandQueue);

    return self;
}
//----------------------------------------------------------------------------------------------------------------------

- (void)generateRandomConfiguration
{
    // if the thread is working we go back
    if (_isRunning.load()) return;

    if (_worker.joinable()) _worker.join();

    auto* simulation = _metalSimulation.get();
    int nAtoms = [self atomCount];
    float freeLength = [self freeLength];
    auto* running = &_isRunning;
    auto* isValid = &_hasValidSetup;
    MainControler* controller = self;

    running->store(true);

    _worker = std::thread([simulation, nAtoms, freeLength, running, isValid, controller]()
    {
        printf("Configuration generation started\n");
        fflush(stdout);

        simulation->generateRandomConfiguration(nAtoms, freeLength);
        isValid->store(true);

        dispatch_async(dispatch_get_main_queue(), ^
        {
            [[NSNotificationCenter defaultCenter]
            postNotificationName:@"MainControlerSetupDidChange"
            object:controller];
        });

        printf("Configuration generation finished\n");
        fflush(stdout);

        running->store(false);
    });
}
//----------------------------------------------------------------------------------------------------------------------

- (void)readInputConfiguration
{
    // if the thread is working we go back
    if (_isRunning.load()) return;

    if (_worker.joinable()) _worker.join();

    auto* simulation = _metalSimulation.get();
    const char* fileName = _parameters.fileNames.startConfig.c_str();
 
    auto* running = &_isRunning;
    auto* isValid = &_hasValidSetup;
    MainControler* controller = self;

    running->store(true);

    _worker = std::thread([simulation, fileName, running, isValid, controller]()
    {
        printf("Configuration reading started\n");
        fflush(stdout);

        isValid->store(simulation->readFile(fileName));

        dispatch_async(dispatch_get_main_queue(), ^
        {
            [[NSNotificationCenter defaultCenter]
            postNotificationName:@"MainControlerSetupDidChange"
            object:controller];
        });

        printf("Configuration reading finished\n");
        fflush(stdout);

        running->store(false);
    });
}
//----------------------------------------------------------------------------------------------------------------------

- (void)start
{
    // If the thread is working, we return.
    if (_isRunning.load()) return;

    if (_worker.joinable()) _worker.join();

    auto* simulation = _metalSimulation.get();
    auto* running = &_isRunning;
    auto* param = &_parameters;

    running->store(true);

    _worker = std::thread([simulation, running, param]()
    {
        printf("Worker started\n");
        fflush(stdout);

        //simulation->generateRandomConfiguration();
 
        simulation->start(param);
 
        printf("Worker stopped\n");
        fflush(stdout);

        running->store(false);
    });

    printf("MainControler: Start\n");
}
//----------------------------------------------------------------------------------------------------------------------

- (void)stop 
{
    // Stop the simulation.
    _metalSimulation->stop();

    // Wait for the worker to finish.
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
    _parameters.simulationSettings.nAtoms = static_cast<int>(atomCount);
    [[NSNotificationCenter defaultCenter] postNotificationName:MainControlerParametersDidChangeNotification object:self];
    printf("Atom Count: %d\n", _parameters.simulationSettings.nAtoms);
}
//----------------------------------------------------------------------------------------------------------------------

- (NSInteger)atomCount
{
    return static_cast<NSInteger>(_parameters.simulationSettings.nAtoms);
}
//----------------------------------------------------------------------------------------------------------------------

- (void) setFreeLength:(float)freeLength
{
    _parameters.simulationSettings.meanFreeDistance = freeLength;
    printf("freeLength: %f\n", _parameters.simulationSettings.meanFreeDistance);
}
//----------------------------------------------------------------------------------------------------------------------

- (float)freeLength
{
    return _parameters.simulationSettings.meanFreeDistance;
}
//----------------------------------------------------------------------------------------------------------------------

- (BOOL)hasValidSetup
{
    return _hasValidSetup.load();
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

    stdoutThread = std::thread(
        readLogPipe,
        stdoutPipe[0],
        savedStdout,
        self,
        NO
    );

    stderrThread = std::thread(
        readLogPipe,
        stderrPipe[0],
        savedStderr,
        self,
        YES
    );
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

- (BOOL)loadProject
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];

    panel.canChooseFiles = NO;
    panel.canChooseDirectories = YES;
    panel.canCreateDirectories = NO;
    panel.allowsMultipleSelection = NO;

    panel.directoryURL = [NSURL fileURLWithPath: [NSString stringWithUTF8String: _parameters.fileNames.defaultRootFolder.c_str()]];

    if ([panel runModal] == NSModalResponseOK)
    {
        _parameters.fileNames.outputFolder = panel.URL.path.UTF8String;

        [[NSUserDefaults standardUserDefaults] setObject:panel.URL.path forKey:@"ProjectFolder"];

        printf("Project folder: %s\n", _parameters.fileNames.outputFolder.c_str());

        return YES;
    }

    return NO;
}
//----------------------------------------------------------------------------------------------------------------------

- (BOOL)newProject
{
    // Ask for project name
    NSAlert *nameAlert = [NSAlert new];

    nameAlert.messageText = @"New Project";
    nameAlert.informativeText = @"Enter a name for the new project.";

    [nameAlert addButtonWithTitle:@"Continue"];
    [nameAlert addButtonWithTitle:@"Cancel"];

    NSTextField *nameField = [[NSTextField alloc] initWithFrame: NSMakeRect(0, 0, 280, 24)];

    nameField.stringValue = [NSString stringWithUTF8String: _parameters.fileNames.defaultProjectName.c_str()];

    nameAlert.accessoryView = nameField;

    if ([nameAlert runModal] != NSAlertFirstButtonReturn)
    {
        [nameField release];
        [nameAlert release];
        return NO;
    }

    NSString *projectName = [nameField.stringValue stringByTrimmingCharactersInSet: [NSCharacterSet whitespaceAndNewlineCharacterSet]];

    [nameField release];
    [nameAlert release];

    if (projectName.length == 0) return NO;

    // Select parent folder
    NSOpenPanel *panel = [NSOpenPanel openPanel];

    panel.title = @"New Project";
    panel.message = @"Choose the parent folder for the new project.";
    panel.prompt = @"Choose Folder";

    panel.canChooseFiles = NO;
    panel.canChooseDirectories = YES;
    panel.canCreateDirectories = YES;
    panel.allowsMultipleSelection = NO;

    panel.directoryURL = [NSURL fileURLWithPath: [NSString stringWithUTF8String: _parameters.fileNames.defaultRootFolder.c_str()]];

    if ([panel runModal] != NSModalResponseOK) return NO;

    NSString *rootPath         = panel.URL.path;
    //NSString *projectName      = [NSString stringWithUTF8String: _parameters.fileNames.defaultProjectName.c_str()];
    NSString *projectPath      = [rootPath stringByAppendingPathComponent:projectName];
    NSFileManager *fileManager = [NSFileManager defaultManager];

    BOOL isDirectory = NO;

    if ([fileManager fileExistsAtPath:projectPath isDirectory:&isDirectory])
    {
        if (!isDirectory)
        {
            NSAlert *alert = [NSAlert new];
            alert.messageText = @"Project cannot be created";
            alert.informativeText = @"A file with the project name already exists.";
            alert.alertStyle = NSAlertStyleWarning;
            [alert runModal];
            [alert release];

            return NO;
        }

        NSAlert *alert = [NSAlert new];

        alert.messageText = @"Project already exists";
        alert.informativeText = [NSString stringWithFormat: @"The following folder and all its contents will be deleted:\n\n%@", projectPath];

        alert.alertStyle = NSAlertStyleWarning;

        [alert addButtonWithTitle:@"Overwrite"];
        [alert addButtonWithTitle:@"Cancel"];

        BOOL overwrite = ([alert runModal] == NSAlertFirstButtonReturn);

        [alert release];

        if (!overwrite) return NO;

        NSError *error = nil;

        if (![fileManager removeItemAtPath:projectPath error:&error])
        {
            NSLog(@"Could not remove project folder: %@", error);
            return NO;
        }
    }

    NSError *error = nil;

    if (![fileManager createDirectoryAtPath:projectPath 
                withIntermediateDirectories:YES
                                 attributes:nil
                                      error:&error])
    {
        NSLog(@"Could not create project folder: %@", error);
        return NO;
    }

    // Update active project
    _parameters.fileNames.defaultRootFolder = rootPath.UTF8String;
    _parameters.fileNames.defaultProjectName = projectName.UTF8String;
    _parameters.fileNames.outputFolder = projectPath.UTF8String;

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

    [defaults setObject:rootPath forKey:@"ProjectRootFolder"];
    [defaults setObject:projectPath forKey:@"ProjectFolder"];

    printf("New project: %s\n", _parameters.fileNames.outputFolder.c_str());

    return YES;
}
//----------------------------------------------------------------------------------------------------------------------

- (BOOL)setupTrajectoryRecording
{
    NSFileManager *fileManager = [NSFileManager defaultManager];

    // Default trajectory folder
    NSString *defaultPath = [NSString stringWithUTF8String: _parameters.fileNames.trajectoryFolder.c_str()];

    // Choose trajectory folder
    //NSOpenPanel *panel = [NSOpenPanel openPanel];
    NSSavePanel *panel = [NSSavePanel savePanel];

    panel.title = @"Trajectory Recording";
    panel.message = @"Choose the folder for trajectory data.";
    panel.prompt = @"Use Folder";

    //panel.canChooseFiles = NO;
    //panel.canChooseDirectories = YES;
    panel.canCreateDirectories = YES;

    panel.directoryURL = [NSURL fileURLWithPath: [defaultPath stringByDeletingLastPathComponent]];

    panel.nameFieldStringValue = [defaultPath lastPathComponent];

    if ([panel runModal] != NSModalResponseOK) return NO;

    NSString *trajectoryPath = panel.URL.path;

    // Prevent using the project folder or one of its parent folders
    NSString *projectPath = [NSString stringWithUTF8String: _parameters.fileNames.outputFolder.c_str()];

    NSString *normalizedProjectPath = projectPath.stringByStandardizingPath;
    NSString *normalizedTrajectoryPath = trajectoryPath.stringByStandardizingPath;

    if ([normalizedTrajectoryPath isEqualToString:normalizedProjectPath] ||
        [normalizedProjectPath hasPrefix: [normalizedTrajectoryPath stringByAppendingString:@"/"]])
    {
        NSAlert *alert = [NSAlert new];
 
        alert.messageText = @"Invalid trajectory folder";
        alert.informativeText =
            @"The trajectory folder must be inside the project folder, not the project folder itself or one of its parent folders.";
 
        [alert runModal];
        [alert release];
 
        return NO;
    }

    // Ask for recording frequency
    NSAlert *frequencyAlert = [NSAlert new];

    frequencyAlert.messageText = @"Trajectory Recording";
    frequencyAlert.informativeText = @"Record every N-th simulation step.";

    [frequencyAlert addButtonWithTitle:@"Continue"];
    [frequencyAlert addButtonWithTitle:@"Cancel"];

    NSTextField *frequencyField = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 100, 24)];

    frequencyField.stringValue = [NSString stringWithFormat:@"%d", _parameters.trajectoryFrequency];

    frequencyAlert.accessoryView = frequencyField;

    if ([frequencyAlert runModal] != NSAlertFirstButtonReturn)
    {
        [frequencyField release];
        [frequencyAlert release];
        return NO;
    }

    NSScanner *scanner = [NSScanner scannerWithString: frequencyField.stringValue];

    NSInteger frequency = 0;

    BOOL validFrequency =
        [scanner scanInteger:&frequency] &&
        scanner.isAtEnd &&
        frequency > 0;

    [frequencyField release];
    [frequencyAlert release];

    if (!validFrequency)
    {
        NSAlert *alert = [NSAlert new];
        alert.messageText = @"Invalid frequency";
        alert.informativeText = @"Please enter a positive integer.";
        [alert runModal];
        [alert release];
        return NO;
    }

    // Check existing trajectory folder
    BOOL isDirectory = NO;

    if ([fileManager fileExistsAtPath:trajectoryPath isDirectory:&isDirectory])
    {
        if (!isDirectory) return NO;

        NSAlert *alert = [NSAlert new];

        alert.messageText = @"Trajectory folder already exists";
        alert.informativeText = [NSString stringWithFormat: @"The following folder and all its contents will be deleted:\n\n%@", trajectoryPath];

        alert.alertStyle = NSAlertStyleWarning;

        [alert addButtonWithTitle:@"Delete and Continue"];
        [alert addButtonWithTitle:@"Cancel"];

        BOOL confirmed = ([alert runModal] == NSAlertFirstButtonReturn);

        [alert release];

        if (!confirmed) return NO;

        NSError *error = nil;

        if (![fileManager removeItemAtPath:trajectoryPath error:&error])
        {
            NSLog(@"Could not remove trajectory folder: %@", error);
            return NO;
        }
    }

    // Create empty trajectory folder
    NSError *error = nil;

    if (![fileManager createDirectoryAtPath:trajectoryPath
                withIntermediateDirectories:YES
                                 attributes:nil
                                      error:&error])
    {
        NSLog(@"Could not create trajectory folder: %@", error);
        return NO;
    }

    // Update parameters
    _parameters.fileNames.trajectoryFolder = trajectoryPath.UTF8String;
    _parameters.trajectoryFrequency = static_cast<int>(frequency);

    printf("Trajectory folder: %s\n", _parameters.fileNames.trajectoryFolder.c_str());
    printf("Trajectory frequency: %d\n", _parameters.trajectoryFrequency);

    return YES;
}
//----------------------------------------------------------------------------------------------------------------------

- (void)setRecordTrajectory:(BOOL)record
{
    _parameters.recordTrajectory = record;
}
//----------------------------------------------------------------------------------------------------------------------

- (void)setInitialConfiguration:(BOOL)random
{
    _parameters.simulationSettings.randomConfiguration = random;
    printf("Random configuration: %s\n", _parameters.simulationSettings.randomConfiguration ? "YES" : "NO");
}
//----------------------------------------------------------------------------------------------------------------------

- (BOOL)selectStartConfiguration
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];

    panel.title = @"Select Initial Configuration";
    panel.prompt = @"Select";
    panel.canChooseFiles = YES;
    panel.canChooseDirectories = NO;
    panel.allowsMultipleSelection = NO;

    panel.allowedContentTypes = @[ [UTType typeWithFilenameExtension:@"xyz"]];

    // Start in the default configuration folder
    NSString *defaultPath = [NSString stringWithUTF8String: _parameters.fileNames.startConfig.c_str()];

    panel.directoryURL = [NSURL fileURLWithPath: [defaultPath stringByDeletingLastPathComponent]];

    panel.nameFieldStringValue = [defaultPath lastPathComponent];

    // Dialog öffnen
    NSModalResponse response = [panel runModal];

    if (response != NSModalResponseOK) return NO;

    _parameters.fileNames.startConfig = std::string(panel.URL.path.UTF8String);

    return YES;
}
//----------------------------------------------------------------------------------------------------------------------

- (void)setPixelFormat:(MTLPixelFormat)pixelFormat
{
    _renderer->setPixelFormat(static_cast<MTL::PixelFormat>(pixelFormat));
}
//----------------------------------------------------------------------------------------------------------------------

- (void)drawInView:(MTKView*)view
{
    if (!_renderer) return;

    id<CAMetalDrawable> drawable = view.currentDrawable;
    MTLRenderPassDescriptor* renderPass = view.currentRenderPassDescriptor;

    if (!drawable || !renderPass) return;

    _renderer->updateFrame
    (
        (__bridge CA::MetalDrawable*)drawable,
        (__bridge MTL::RenderPassDescriptor*)renderPass,
        _metalSimulation->getOutput()
    );

    _metalSimulation->renderingIsDone();
}
//----------------------------------------------------------------------------------------------------------------------

- (void)drawableSizeWillChange:(CGSize)size
{
    if (!_renderer) return;

    _renderer->resize
    (
        static_cast<float>(size.width),
        static_cast<float>(size.height)
    );
}
//----------------------------------------------------------------------------------------------------------------------

- (void)mouseDragged:(float)dx :(float)dy :(BOOL)controlPressed
{
    _renderer->mouseDragged(dx, dy, controlPressed);
}
//----------------------------------------------------------------------------------------------------------------------

@end