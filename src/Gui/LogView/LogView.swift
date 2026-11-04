import SwiftUI

struct LogView: View
{
    @State private var logMessages: [(message: String, isError: Bool)] = []
    //------------------------------------------------------------------------------------------------------------------

    private var logText: NSAttributedString
    {
        let result = NSMutableAttributedString()

        for entry in logMessages
        {
            let attributes: [NSAttributedString.Key: Any] = [
                .foregroundColor: entry.isError
                    ? NSColor.systemRed
                    : NSColor.labelColor,
                .font: NSFont.monospacedSystemFont(
                    ofSize: 12,
                    weight: .regular
                )
            ]

            result.append(
                NSAttributedString(
                    string: entry.message + "\n",
                    attributes: attributes
                )
            )
        }

        return result
    }
    //------------------------------------------------------------------------------------------------------------------

    var body: some View
    {
        VStack(spacing: 0)
        {
            LogTextView(text: logText)

            Divider()

            HStack
            {
                Button("Copy All")
                {
                    NSPasteboard.general.clearContents()

                    NSPasteboard.general.setString(
                        logMessages.map(\.message).joined(separator: "\n"),
                        forType: .string
                    )
                }

                Button("Save Log")
                {
                    saveLog()
                }

                Spacer()

                Text("\(logMessages.count) lines")
                    .foregroundStyle(.secondary)
                    .font(.caption)
            }
            .padding(8)
        }
        .onReceive(
            NotificationCenter.default.publisher(
                for: Notification.Name("MainControlerDidAppendLog")
            )
        )
        { notification in

            guard
                let userInfo = notification.userInfo,
                let message = userInfo["message"] as? String,
                let isError = userInfo["isError"] as? Bool
            else
            {
                return
            }

            logMessages.append((message, isError))
        }
    }
    //------------------------------------------------------------------------------------------------------------------

    private func saveLog()
    {
        let panel = NSSavePanel()

        panel.allowedContentTypes = [.plainText]
        panel.nameFieldStringValue = "SolverLog.txt"

        panel.begin
        { response in

            guard response == .OK,
                  let url = panel.url
            else
            {
                return
            }

            let content = logMessages
                .map(\.message)
                .joined(separator: "\n")

            do
            {
                try content.write(
                    to: url,
                    atomically: true,
                    encoding: .utf8
                )
            }
            catch
            {
                NSLog("Could not save log: %@", error.localizedDescription)
            }
        }
    }
    //------------------------------------------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------------------------------------------

struct LogTextView: NSViewRepresentable
{
    let text: NSAttributedString
    //------------------------------------------------------------------------------------------------------------------

    func makeNSView(context: Context) -> NSScrollView
    {
        let scrollView = NSScrollView()

        scrollView.hasVerticalScroller = true
        scrollView.hasHorizontalScroller = false
        scrollView.autohidesScrollers = false
        scrollView.borderType = .noBorder

        let textView = NSTextView()

        textView.isEditable = false
        textView.isSelectable = true
        textView.isRichText = false
        textView.allowsUndo = false

        textView.font = .monospacedSystemFont(
            ofSize: 12,
            weight: .regular
        )

        textView.textContainerInset = NSSize(
            width: 8,
            height: 8
        )

        textView.isVerticallyResizable = true
        textView.isHorizontallyResizable = false

        textView.autoresizingMask = [.width]

        textView.textContainer?.widthTracksTextView = true

        scrollView.documentView = textView

        return scrollView
    }
    //------------------------------------------------------------------------------------------------------------------

    func updateNSView(
        _ scrollView: NSScrollView,
        context: Context
    )
    {
        guard let textView = scrollView.documentView as? NSTextView
        else { return }

        let wasAtBottom =
            scrollView.contentView.bounds.maxY >=
            scrollView.documentView!.bounds.maxY - 30

        textView.textStorage?.setAttributedString(text)

        if wasAtBottom
        {
            textView.scrollToEndOfDocument(nil)
        }
    }
    //------------------------------------------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------------------------------------------
