import SwiftUI

struct LogView: View
{
    @State private var logMessages: [(message: String, isError: Bool)] = []
    //------------------------------------------------------------------------------------------------------------------

    var body: some View
    {
        ScrollViewReader
        {
            proxy in

            ScrollView
            {
                LazyVStack(alignment: .leading, spacing: 2)
                {
                    ForEach(logMessages.indices, id: \.self)
                    {
                        index in

                        Text(logMessages[index].message)
                            .font(.system(.body, design: .monospaced))
                            .foregroundColor(logMessages[index].isError ? .red : .primary)
                            .frame(maxWidth: .infinity, alignment: .leading)
                            .textSelection(.enabled)
                            .id(index)
                    }
                }
                .padding(8)
            }
            .onChange(of: logMessages.count)
            {
                _, newValue in

                if newValue > 0
                {
                    proxy.scrollTo(newValue - 1, anchor: .bottom)
                }
            }
        }
        .onReceive(
            NotificationCenter.default.publisher(
                for: Notification.Name("MainControlerDidAppendLog")
            )
        )
        {
            notification in

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
}
//----------------------------------------------------------------------------------------------------------------------