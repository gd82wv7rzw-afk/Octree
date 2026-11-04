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

        view.clearColor = MTLClearColor(
            red: 0.0,
            green: 0.0,
            blue: 0.0,
            alpha: 1.0
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
        let coordinator = Coordinator()
        coordinator.commandQueue = mainControler.commandQueue
        return coordinator
    }
    //------------------------------------------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------------------------------------------

class Coordinator: NSObject, MTKViewDelegate
{
    var commandQueue: MTLCommandQueue?

    func draw(in view: MTKView)
    {
        guard let drawable = view.currentDrawable,
              let descriptor = view.currentRenderPassDescriptor,
              let commandBuffer = commandQueue?.makeCommandBuffer(),
              let encoder = commandBuffer.makeRenderCommandEncoder(descriptor: descriptor)
        else { return }

        encoder.endEncoding()
        commandBuffer.present(drawable)
        commandBuffer.commit()
    }

    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize)
    {
    }
}
//----------------------------------------------------------------------------------------------------------------------