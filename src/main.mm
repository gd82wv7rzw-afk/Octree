
#import "Gui/App/AppDelegate.h"

#import <Cocoa/Cocoa.h>

int main(int argc, const char * argv[]) {

    @autoreleasepool 
    {
        NSApplication *app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        AppDelegate *delegate = [[AppDelegate alloc] init];
        [app setDelegate: delegate];

        // Optional wichtig: Fenster aktiv mache
        [app activateIgnoringOtherApps:YES];
        
        [app run];

        // Setup code that might create autoreleased objects goes here.
        return 0;
    }    
}