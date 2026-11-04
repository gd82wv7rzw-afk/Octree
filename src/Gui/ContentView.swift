import SwiftUI

struct ContentView: View
{
    @AppStorage("showSidePanel") private var isSidePanelVisible = true
    @State private var isToolbarVisible = true
    @State private var isFullscreen = false
    @AppStorage("showLog") private var isLogVisible = false
    //------------------------------------------------------------------------------------------------------------------

    let mainControler: MainControler
    //------------------------------------------------------------------------------------------------------------------

    //let cppWrapper: CppWrapper

    var body: some View
    {
        HStack(spacing: 0)
        {
            // 1. Metal View / Log View
            if isLogVisible
            {
                LogView()
                    .frame(minWidth: 800, minHeight: 600)
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
            else
            {
                MetalView(mainControler: mainControler)
                    .frame(minWidth: 800, minHeight: 600)
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
            
            // 2. Festes Seitenpanel (300px)
            if isSidePanelVisible
            {
                SidePanelView(mainControler: mainControler)
                    .frame(width: 300)
                    .transition(.move(edge: .trailing))
            }
        }
        .ignoresSafeArea(isFullscreen ? .all : [])
        .toolbar
        {
            ToolbarItem(placement: .primaryAction)
            {
                Button{ withAnimation {isSidePanelVisible.toggle()}}
                label: { Image(systemName: "sidebar.right")}
                .help("Show or hide sidebar")
            }
        }
        .toolbar(isFullscreen ? .hidden : .visible, for: .windowToolbar)
        .onReceive(
            NotificationCenter.default.publisher(
                for: NSWindow.didEnterFullScreenNotification
            )
        ) { _ in
            isFullscreen = true
            isToolbarVisible = false
        }
        .onReceive(
            NotificationCenter.default.publisher(
                for: NSWindow.didExitFullScreenNotification
            )
        ) { _ in
            isFullscreen = false
            isToolbarVisible = false
        }
    }
}
//---------------------------------------------------------------------------------------------------------------------
