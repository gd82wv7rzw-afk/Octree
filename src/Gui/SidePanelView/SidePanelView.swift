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
    @State private var sphereResolution = 2
    @State private var sphereScale = 1.0
    @State private var delta_t: Double = 0.1
    @State private var temperature: Double = 1.0
    @State private var lightColor: Color = .white
    @State private var materialColor: Color = .red
    @State private var lightIntensity = 1.0
    @State private var ambient = 0.2
    @State private var diffuse = 0.8
    @State private var specular = 0.5
    @State private var shininess = 32.0
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

            // start / stop / record
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

            // dt
            HStack(alignment: .center)
            {
                Text("Δt:")
                    .font(.body)
                    .foregroundColor(.secondary)

                VStack
                {
                    Text(String(format: "%.4f", delta_t))
                        .monospacedDigit()
                        .frame(maxWidth: .infinity, alignment: .trailing)

                    Slider(value: $delta_t, in: 1e-2...1)
                        .frame(maxWidth: .infinity)
                }
            }
            .onChange(of: delta_t)
            {
                mainControler.setDt(delta_t)
            }
            //----------------------------------------------------------------------------------------------------------

            // temperature
            HStack(alignment: .center)
            {
                HStack(spacing: 4)
                {
                    Text("T:")
                        .font(.body)
                        .foregroundColor(.secondary)
 
                    Button
                    {
                        temperature = 1.0
                    }
                    label:
                    {
                        Image(systemName: "arrow.counterclockwise")
                    }
                    .buttonStyle(.borderless)
                    .help("Reset to 1.0")
                }
                .fixedSize()
                .frame(width: 20, alignment: .leading)
                
                VStack
                {
                    Text(String(format: "%.4f", temperature))
                        .monospacedDigit()
                        .frame(maxWidth: .infinity, alignment: .trailing)

                    Slider(value: $temperature, in: 0.9...1.1)
                        .frame(maxWidth: .infinity)
                }
            }
            .onChange(of: temperature)
            {
                mainControler.setTemperature(temperature)
            }
            //----------------------------------------------------------------------------------------------------------

            // Light & Color
            Section
            {
                VStack
                {
                    ColorPicker("Light color:", selection: $lightColor)
                    ColorPicker("Material color:", selection: $materialColor)
                    lightSlider("Intensity:", value: $lightIntensity, range: 0...2)
                    lightSlider("Ambient:", value: $ambient, range: 0...1)
                    lightSlider("Diffuse:", value: $diffuse, range: 0...1)
                    lightSlider("Specular:", value: $specular, range: 0...1)
                    lightSlider("Shininess:", value: $shininess, range: 1...128)
                }
                .font(.body)
                .foregroundColor(.secondary)    
                //------------------------------------------------------------------------------------------------------ 
            }
            //----------------------------------------------------------------------------------------------------------

            // resolution and scale
            HStack
            {
                VStack(alignment: .leading)
                {
                    Text("Resolution:")
                        .font(.body)
                        .foregroundColor(.secondary)

                    Stepper(value: $sphereResolution, in: 0...5)
                    {
                        Text("\(sphereResolution)")
                    }
                }

                Divider()
   
                VStack(alignment: .leading)
                {
                    Text("Scale:")
                        .font(.body)
                        .foregroundColor(.secondary)

                    Stepper(value: $sphereScale, in: 0.1...3.0, step: 0.1)
                    {
                        Text(String(format: "%.1f", sphereScale))
                    }
                }
            }
            //----------------------------------------------------------------------------------------------------------
        }
        .formStyle(.grouped)
        //--------------------------------------------------------------------------------------------------------------
    }
    //------------------------------------------------------------------------------------------------------------------
    private func lightSlider
    (
        _ title: String,
        value: Binding<Double>,
        range: ClosedRange<Double>
    ) -> some View
    {
        HStack
        {
            Text(title)
                .foregroundColor(.secondary)
                .frame(width: 75, alignment: .leading)
 
            VStack
            {
                Text(String(format: "%.2f", value.wrappedValue))
                    .monospacedDigit()
                    .frame(maxWidth: .infinity, alignment: .trailing)
 
                Slider(value: value, in: range)
            }
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------
