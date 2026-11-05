//
//  SidePanelView.swift
//  Gui
//
//  Created by Emu on 03.11.26.
//

import SwiftUI

struct SidePanelView: View
{
    @State private var atomCount: Int
    @State private var run:  Bool = false
    @State private var recordTrajectory: Bool = false    
    @State private var hasValidSetup = false
    let mainControler: MainControler
    //@State private var mainControler = MainControler()
    //------------------------------------------------------------------------------------------------------------------

    init(mainControler: MainControler)
    {
        self.mainControler = mainControler
        _atomCount = State(initialValue: Int(mainControler.atomCount))
    }
    //------------------------------------------------------------------------------------------------------------------

    var body: some View
    {
        Form
        {
            // nuber of atoms
            HStack
            {
                Text("Number of Atoms:").font(.body).foregroundColor(.secondary)
                Spacer()

                Text(atomCount.formatted(IntegerFormatStyle<Int>(locale: Locale(identifier: "en_US_POSIX"))))
            }
            .formStyle(.grouped)
            .onReceive(
                NotificationCenter.default.publisher(
                    for: .MainControlerParametersDidChange,
                    object: mainControler
                )
            )
            {
                _ in atomCount = Int(mainControler.atomCount)
            }
            //----------------------------------------------------------------------------------------------------------

            HStack
            {
                Toggle(
                    "Record Trajectory:",
                    isOn: Binding(
                        get: { recordTrajectory },
                        set: { newValue in
    
                            if newValue 
                            {
                                guard mainControler.setupTrajectoryRecording() 
                                else {return}
                            }
    
                            recordTrajectory = newValue
                            mainControler.setRecordTrajectory(newValue)
                        }
                    )
                )
                .font(.body)
                .foregroundColor(.secondary)
                .toggleStyle(.checkbox) 
   
                Spacer()
   
                Button
                {
                    if run
                    {
                        mainControler.stop()
                        run = false
                    }
                    else
                    {
                        mainControler.start()
                        run = true
                    }
                }
                label:
                {
                    Text(run ? "Stop" : "Run").frame(maxWidth: .infinity)
                }
                .buttonStyle(.borderedProminent)
                .controlSize(.regular)
                .frame(width: 80, alignment: .trailing)
                //.frame(width: 80)                
            }
            .formStyle(.grouped)
            .disabled(!hasValidSetup)
            .onReceive(NotificationCenter.default.publisher(for: Notification.Name("MainControlerSetupDidChange")))
            {
                 _ in
                hasValidSetup = mainControler.hasValidSetup
            }
            //----------------------------------------------------------------------------------------------------------
        }
        .formStyle(.grouped)
        //--------------------------------------------------------------------------------------------------------------
    }
}
//----------------------------------------------------------------------------------------------------------------------
