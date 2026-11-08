import SwiftUI
import MetalKit

struct MetalView: NSViewRepresentable
{
    let mainControler: MainControler

    func makeNSView(context: Context) -> MTKView
    {
        let view = InteractiveMTKView()

        if let device = mainControler.metalDevice
        {
            view.device = device
        }

        mainControler.setPixelFormat(view.colorPixelFormat)

        view.onMouseDragged = 
        { 
            dx, dy, controlPressed in
            mainControler.mouseDragged(dx, dy, controlPressed)
        }
        
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

class InteractiveMTKView: MTKView
{
    var onMouseDragged: ((Float, Float, Bool) -> Void)?

    override func mouseDragged(with event: NSEvent)
    {
        let controlPressed = event.modifierFlags.contains(.control)

        onMouseDragged?(Float(event.deltaX), Float(event.deltaY), controlPressed)
    }
}
//----------------------------------------------------------------------------------------------------------------------