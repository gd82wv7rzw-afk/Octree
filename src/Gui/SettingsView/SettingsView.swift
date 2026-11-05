
import SwiftUI

struct SettingsView: View
{
    @State private var initialConfiguration = "random"
    @State private var atomCountInput = Int()
    @State private var startConfigURL: URL?
    @State private var meanFreeDistance = Float()
    @State private var applySettings = false

    @Environment(\.dismiss) private var dismiss


    //@FocusState private var atomCountIsFocused: Bool

    let mainControler: MainControler

    private let minAtoms = 1
    private let maxAtoms = Int(1e6)
    //------------------------------------------------------------------------------------------------------------------

    init(mainControler: MainControler)
    {
        self.mainControler = mainControler
        _atomCountInput = State(initialValue: Int(mainControler.atomCount))
        _meanFreeDistance = State(initialValue: Float(mainControler.freeLength))
    }
    //------------------------------------------------------------------------------------------------------------------

    var body: some View
    {
        VStack(alignment: .leading)
        {
            //Text("Simulation Settings").font(.title2)

            //Spacer()

            Picker("Initial Configuration", 
                    selection: Binding(
                        get: { initialConfiguration },
                        set: 
                        { 
                            newValue in
                            initialConfiguration = newValue

                            if newValue == "file" {mainControler.selectStartConfiguration()}
                        }
                    )
                )
            {
                Text("Random configuration").tag("random")
                Text("Load configuration from file…").tag("file")
            }
            .font(.body)
            .foregroundColor(.secondary)
            .pickerStyle(.radioGroup)
            .onChange(of: initialConfiguration)
            {
                mainControler.setInitialConfiguration(initialConfiguration == "random")
            }  

            Divider().padding(.vertical, 8)
            //Spacer()

            if initialConfiguration == "random"
            {
                HStack
                {
                    Text("Number of Atoms:")
                        .font(.body)
                        .foregroundColor(.secondary)

                    Spacer()

                    TextField("Atoms", value: $atomCountInput, format: .number.grouping(.never))
                        .frame(width: 80)
                        .multilineTextAlignment(.trailing)
                        //.focused($atomCountIsFocused)
                        .onDisappear()
                        {
                            commitAtomCount()
                        }
                }

                Spacer()

                HStack
                {
                    Text("Mean Free Distance:")
                        .font(.body)
                        .foregroundColor(.secondary)

                    Spacer()

                    TextField("meanFreeDistance", value: $meanFreeDistance, format: .number.locale(Locale(identifier: "en_US_POSIX")))
                        .frame(width: 80)
                        .multilineTextAlignment(.trailing)
                        //.focused($meanFreeDistance)
                        .onDisappear()
                        {
                            commitFreeLength()
                        }
                }
            }

            Spacer()

            HStack
            {
                Spacer()
   
                Button("Cancel")
                {
                    print("Settings cancelled")
                    applySettings = false
                    dismiss()
                }
   
                Button("Apply & Initialize")
                {
                    print("Apply & Initialized")
                    applySettings = true
                    dismiss()
                }
                .buttonStyle(.borderedProminent)
            }          
        }
        .padding()
        .frame(width: 450, height: 300)
        .onDisappear
        {
            if applySettings
            {
                commitAtomCount()
                commitFreeLength()

                applySettings = false
                print("Initialize Simulator")

                if initialConfiguration == "random"
                {
                    mainControler.generateRandomConfiguration()
                }
                else
                {
                    mainControler.readInputConfiguration();
                }
            }
        }
    }
    //------------------------------------------------------------------------------------------------------------------

    private func commitAtomCount()
    {
        let correctedValue = min(max(atomCountInput, minAtoms), maxAtoms)

        mainControler.atomCount = correctedValue
        atomCountInput = mainControler.atomCount
    }
    //------------------------------------------------------------------------------------------------------------------

    private func commitFreeLength()
    {
        let correctedValue = max(meanFreeDistance, 1.0)

        mainControler.freeLength = correctedValue
        meanFreeDistance = mainControler.freeLength
    }
    //------------------------------------------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------------------------------------------
