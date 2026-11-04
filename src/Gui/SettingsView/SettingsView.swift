import SwiftUI

struct SettingsView: View
{
    var body: some View
    {
        VStack(alignment: .leading)
        {
            Text("Simulation Settings")
                .font(.title2)

            Text("Settings will be added here.")
                .foregroundColor(.secondary)
        }
        .padding()
        .frame(width: 450, height: 300)
    }
}
//----------------------------------------------------------------------------------------------------------------------