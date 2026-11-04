//
//  SidePanelView.swift
//  Gui
//
//  Created by Emu on 03.11.26.
//

import SwiftUI

struct SidePanelView: View
{
    @State private var count: Int = Int(1e3)
    @State private var run:  Bool = false
    @State private var recordTrajectory: Bool = false
    @State private var initialConfiguration = "random"
    let mainControler: MainControler
    //@State private var mainControler = MainControler()
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

                Text(count.formatted()).frame(width: 80, alignment: .trailing)
            }
            .formStyle(.grouped)
            //----------------------------------------------------------------------------------------------------------

            Picker("Initial Configuration", selection: $initialConfiguration)
            {
                Text("Random configuration").tag("random").font(.body).foregroundColor(.secondary)
                Text("Load configuration from file…").tag("file").font(.body).foregroundColor(.secondary)
            }
            .font(.body).foregroundColor(.secondary)
            .pickerStyle(.radioGroup)
            .onChange(of: initialConfiguration)
            {
                mainControler.setInitialConfiguration(initialConfiguration == "random")
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
            //----------------------------------------------------------------------------------------------------------
        }
        .formStyle(.grouped)
        //--------------------------------------------------------------------------------------------------------------
    }
}
//----------------------------------------------------------------------------------------------------------------------