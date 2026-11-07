import SwiftUI
import MetalKit

struct MetalView: NSViewRepresentable
{
    let mainControler: MainControler

    func makeNSView(context: Context) -> MTKView
    {
        let view = MTKView()

        if let device = mainControler.metalDevice
        {
            view.device = device
        }

        mainControler.setPixelFormat(view.colorPixelFormat)

        view.clearColor = MTLClearColor(
            red:    0.0,
            green:  0.0,
            blue:   0.0,
            alpha:  1.0
        )

        view.delegate = context.coordinator
        view.isPaused = false

        return view
    }
    //------------------------------------------------------------------------------------------------------------------

    func updateNSView(_ view: MTKView, context: Context)
    {
    }
    //------------------------------------------------------------------------------------------------------------------

    func makeCoordinator() -> Coordinator
    {
        Coordinator(mainControler: mainControler)
    }
    //------------------------------------------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------------------------------------------

class Coordinator: NSObject, MTKViewDelegate
{
    let mainControler: MainControler

    init(mainControler: MainControler)
    {
        self.mainControler = mainControler
    }

    func draw(in view: MTKView)
    {
        mainControler.draw(in: view)
    }

    func mtkView(
        _ view: MTKView,
        drawableSizeWillChange size: CGSize
    )
    {
        mainControler.drawableSizeWillChange(size)
    }
}
//----------------------------------------------------------------------------------------------------------------------