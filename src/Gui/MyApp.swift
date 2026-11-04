import SwiftUI

@main struct MyApp: App
{
    @NSApplicationDelegateAdaptor(AppDelegate.self) var appDelegate
    @State private var mainControler = MainControler()
    
    var body: some Scene
    {
        WindowGroup
        {
            ContentView(mainControler: mainControler)
            .frame(minWidth: 1100, minHeight: 600)
        }
        .windowToolbarStyle(.unifiedCompact)
        .commands { SidePanelCommands()}
        .commands { LogCommands() }
        .commands { FileCommands(mainControler: mainControler) }
        .commands 
        {
            FileCommands(mainControler: mainControler)
            SimulationCommands()
        }
        //--------------------------------------------------------------------------------------------------------------

        Settings
        {
            SettingsView()
        }
        .commandsRemoved()
    }
}
//---------------------------------------------------------------------------------------------------------------------

struct SidePanelCommands: Commands {
    @AppStorage("showSidePanel") private var isSidePanelVisible = true

    var body: some Commands {
        CommandGroup(after: .sidebar) {
            Toggle("Show Side Panel", isOn: $isSidePanelVisible)
        }
    }
}
//---------------------------------------------------------------------------------------------------------------------

struct LogCommands: Commands {
    @AppStorage("showLog") private var isLogVisible = false

    var body: some Commands {
        CommandGroup(after: .sidebar) {
            Toggle("Show Log", isOn: $isLogVisible)
                .keyboardShortcut("l", modifiers: [.command, .shift])
        }
    }
}
//---------------------------------------------------------------------------------------------------------------------

struct FileCommands: Commands
{
    let mainControler: MainControler

    var body: some Commands
    {
        CommandGroup(replacing: .newItem)
        {
            Button("New Project")
            {
                mainControler.newProject()
            }
            .keyboardShortcut("n", modifiers: [.command])

            Button("Load Project")
            {
                mainControler.loadProject()
            }
            .keyboardShortcut("o", modifiers: [.command])
        }
    }
}
//---------------------------------------------------------------------------------------------------------------------

struct SimulationCommands: Commands
{
    @Environment(\.openSettings) private var openSettings

    var body: some Commands
    {
        CommandMenu("Simulation")
        {
            Button("Settings…")
            {
                openSettings()
            }
        }
    }
}
//---------------------------------------------------------------------------------------------------------------------