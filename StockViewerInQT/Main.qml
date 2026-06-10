import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

ApplicationWindow {
    visible: true
    width: 1200
    height: 1200
    title: "S&P 500 Stock Viewer"

    minimumWidth: 1200
    maximumWidth: 1200
    minimumHeight: 1200
    maximumHeight: 1200

    // -- THEME ----------------------------------------------------------------
    readonly property color bg:        "#000000"
    readonly property color panel:     "#0a0a0a"
    readonly property color panelAlt:  "#0d0d0d"
    readonly property color borderCol: "#1a1a1a"
    readonly property color cyan:      "#00ffff"
    readonly property color cyanDim:   "#00cccc"
    readonly property color cyanFade:  "#003333"
    readonly property color hoverCol:  "#001a1a"
    readonly property color redCol:    "#ff4444"
    readonly property color greenCol:  "#00ff88"
    readonly property color orange:    "#ffaa00"
    readonly property color seablue:   "#006994"
    readonly property color white:     "#ffffff"

    color: bg

    // -- ALERT DETAIL POPUP ---------------------------------------------------
    Popup {
        id: alertPopup
        x: (parent.width  - width)  / 2
        y: (parent.height - height) / 2
        width: 440
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 0

        property string alertSymbol: ""
        property string alertName: ""
        property string alertType: ""
        property string alertDesc: ""
        property string alertTime: ""
        property double alertValue: 0

        background: Rectangle {
            color: "#0d0d0d"
            border.color: alertPopup.alertType === "bullish" ? greenCol
                        : alertPopup.alertType === "bearish" ? redCol : orange
            border.width: 1
            radius: 4
        }

        ColumnLayout {
            width: alertPopup.width
            spacing: 12
            anchors.margins: 20
            anchors.fill: parent

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Rectangle {
                    width: typeLabel.implicitWidth + 16; height: 22; radius: 3
                    color: alertPopup.alertType === "bullish" ? "#003322"
                         : alertPopup.alertType === "bearish" ? "#330011" : "#2a1f00"
                    Text {
                        id: typeLabel
                        anchors.centerIn: parent
                        text: alertPopup.alertType.toUpperCase()
                        color: alertPopup.alertType === "bullish" ? greenCol
                             : alertPopup.alertType === "bearish" ? redCol : orange
                        font.pixelSize: 10; font.letterSpacing: 2; font.bold: true
                    }
                }

                Text {
                    text: alertPopup.alertSymbol
                    color: cyan; font.pixelSize: 18; font.bold: true; leftPadding: 8
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: "×"; color: "#666"; font.pixelSize: 20
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: alertPopup.close()
                    }
                }
            }

            Text {
                text: alertPopup.alertName
                color: cyan; font.pixelSize: 14; font.bold: true
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: borderCol }

            Text {
                Layout.fillWidth: true
                text: alertPopup.alertDesc
                color: "#aaa"; font.pixelSize: 12
                wrapMode: Text.WordWrap; lineHeight: 1.5
            }

            Text {
                text: "Detected: " + alertPopup.alertTime
                color: seablue; font.pixelSize: 10
            }
        }
    }


    // -- DEBUG PANEL (F1) ----------------------------------------------------
    Shortcut {
        sequence: "F1"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (debugPopup.visible)
            {
                debugPopup.close()
            }
            else
            {
                debugPopup.open()
            }
        }
    }

    Popup {
        id: debugPopup
        parent: Overlay.overlay          // Better for modal behavior
        anchors.centerIn: parent
        width: parent.width * 0.82
        height: parent.height * 0.80
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 0
        focus: true                      // Important for ESC to work

        background: Rectangle {
            color: "#050505"
            border.color: cyanFade
            border.width: 1
            radius: 4
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 0
            spacing: 0

            // Title bar
            Rectangle {
                Layout.fillWidth: true
                height: 38
                color: "#0a0a0a"
                radius: 4

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14

                    Text {
                        text: "DEBUG LOG"
                        color: cyan
                        font.pixelSize: 11
                        font.letterSpacing: 3
                        font.bold: true
                        font.family: "Courier New"
                    }

                    Text {
                        text: "F12 to toggle · ESC to close · click outside to close"
                        color: cyan
                        font.pixelSize: 10
                        font.family: "Courier New"
                        leftPadding: 12
                    }

                    Item { Layout.fillWidth: true }

                    // Stats row
                    Text {
                        text: "watchlistModel:" + watchlistModel.count +
                              "  alerts:" + alertModel.count +
                              "  log:" + logger.entries.length
                        color: cyan
                        font.pixelSize: 10
                        font.family: "Courier New"
                    }

                    Rectangle {
                        width: 52; height: 22; radius: 3
                        color: dbClearArea.containsMouse ? "#1a0000" : "transparent"
                        border.color: dbClearArea.containsMouse ? redCol : "#2a2a2a"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "CLEAR"
                            color: dbClearArea.containsMouse ? redCol : seablue
                            font.pixelSize: 9
                            font.letterSpacing: 1
                            font.family: "Courier New"
                        }
                        MouseArea {
                            id: dbClearArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: logger.clear()
                        }
                    }
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: borderCol }

            // Log list (rest remains the same)
            ListView {
                id: logView
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: logger.entries
                clip: true
                spacing: 0
                verticalLayoutDirection: ListView.TopToBottom   // Normal order

                ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                        active: true

                        background: Rectangle {
                            implicitWidth: 8
                            color: "#0a0a0a"          // dark panel color
                            radius: 4
                        }

                        contentItem: Rectangle {
                            implicitWidth: 6
                            radius: 4
                            color: logView.movingVertically ||
                                   (parent && parent.pressed) ? cyan : cyanDim
                            opacity: logView.movingVertically ||
                                     (parent && parent.pressed) ? 1.0 : 0.6
                        }
                }

                // Auto-scroll to bottom when new log arrives
                onCountChanged: {
                    Qt.callLater(scrollToBottom)
                }

                function scrollToBottom() {
                    if (count > 0)
                        positionViewAtIndex(count - 1, ListView.End)
                }

                delegate: Rectangle {
                    width: logView.width
                    height: logLine.implicitHeight + 4
                    color: index % 2 === 0 ? "#050505" : "#070707"

                    readonly property bool isWarn:   modelData.indexOf("WARN") !== -1
                    readonly property bool isCrit:   modelData.indexOf("CRIT") !== -1 || modelData.indexOf("FATAL") !== -1
                    readonly property bool isSignal: modelData.indexOf("SignalEngine") !== -1
                    readonly property bool isFetch:  modelData.indexOf("Provider") !== -1 || modelData.indexOf("Fetch") !== -1 || modelData.indexOf("fetch") !== -1
                    readonly property bool isQml:    modelData.indexOf("QML") !== -1

                    // left accent bar
                    Rectangle {
                        width: 2; height: parent.height
                        color: isCrit   ? redCol   :
                               isWarn   ? orange    :
                               isSignal ? greenCol  :
                               isFetch  ? cyanDim   :
                               isQml    ? cyan       : "transparent"
                        opacity: 0.7
                    }

                    Text {
                        id: logLine
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData
                        color: isCrit   ? redCol   :
                               isWarn   ? orange    :
                               isSignal ? greenCol  :
                               isFetch  ? cyanDim   :
                               isQml    ? cyan       : white
                        font.pixelSize: 11
                        font.family: "Courier New"
                        wrapMode: Text.WrapAnywhere
                    }
                }

                Text {
                    anchors.centerIn: parent
                    visible: logger.entries.length === 0
                    text: "No log entries yet"
                    color: seablue
                    font.pixelSize: 12
                    font.family: "Courier New"
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: borderCol }

            // Quick inject row
            Rectangle {
                Layout.fillWidth: true
                height: 36
                color: "#0a0a0a"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 8

                    Text {
                        text: ">"
                        color: cyan
                        font.pixelSize: 13
                        font.family: "Courier New"
                    }

                    TextInput {
                        id: debugInput
                        Layout.fillWidth: true
                        color: cyan
                        font.pixelSize: 12
                        font.family: "Courier New"
                        verticalAlignment: TextInput.AlignVCenter

                        Keys.onReturnPressed: {
                            if (text.trim().length > 0) {
                                logger.log(text)
                                text = ""
                            }
                        }

                        Text {
                            anchors.fill: parent
                            text: "Type a message and press Enter to log it…"
                            color: cyan
                            font.pixelSize: 12
                            font.family: "Courier New"
                            verticalAlignment: Text.AlignVCenter
                            visible: !debugInput.text && !debugInput.activeFocus
                        }
                    }
                }
            }
        }
    }

    // -- ROOT LAYOUT ----------------------------------------------------------
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ════════════════════════════════════════════════════════════════════
        // LEFT PANEL — watchlistModel + Chart
        // ════════════════════════════════════════════════════════════════════
        ColumnLayout {
            Layout.preferredWidth: 520
            Layout.maximumWidth: 520
            Layout.fillHeight: true
            spacing: 0

            // -- Header bar --------------------------------------------------
            Rectangle {
                Layout.fillWidth: true
                height: 44
                color: panel
                border.color: borderCol
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14

                    Text {
                        text: "watchlistModel"
                        color: cyan
                        font.pixelSize: 11
                        font.letterSpacing: 3
                        font.bold: true
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: watchlistModel.count + " / 20"
                        color: watchlistModel.count >= 20 ? redCol : cyanDim
                        font.pixelSize: 11
                        font.letterSpacing: 1
                    }

                    // Live feed status dot (only visible when Alpaca selected)
                    Row {
                        spacing: 5
                        visible: providerBox.currentIndex === 2
                        anchors.verticalCenter: undefined
                        Rectangle {
                            width: 7; height: 7; radius: 4
                            anchors.verticalCenter: parent.verticalCenter
                            color: alpacaWs.connected ? greenCol : "#555"
                                                SequentialAnimation on opacity {
                                loops: Animation.Infinite
                                running: alpacaWs.connected
                                NumberAnimation { to: 0.3; duration: 700 }
                                NumberAnimation { to: 1.0; duration: 700 }
                            }
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: alpacaWs.connected ? "LIVE" : "CONNECTING…"
                            color: alpacaWs.connected ? greenCol : "#555"
                            font.pixelSize: 9
                            font.letterSpacing: 1
                            font.bold: true
                        }
                    }

                    ComboBox {
                        id: providerBox
                        implicitWidth: 110
                        implicitHeight: 28
                        currentIndex: 2
                        model: ["Stooq", "Yahoo", "Alpaca", "Finnhub"]
                        onCurrentIndexChanged: stockFetcher.setProvider(currentIndex)

                        background: Rectangle {
                            color: bg; border.color: cyanFade; border.width: 1; radius: 3
                        }
                        contentItem: Text {
                            text: providerBox.displayText
                            color: cyanDim; font.pixelSize: 11
                            verticalAlignment: Text.AlignVCenter; leftPadding: 8
                        }
                        delegate: ItemDelegate {
                            width: providerBox.width
                            background: Rectangle {
                                color: highlighted ? hoverCol : bg
                                border.color: "#001a1a"
                            }
                            contentItem: Text {
                                text: modelData
                                color: highlighted ? cyan : cyanDim
                                font.pixelSize: 11; leftPadding: 8
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                        popup: Popup {
                            y: providerBox.height; width: providerBox.width; padding: 0
                            background: Rectangle { color: bg; border.color: cyanFade; border.width: 1 }
                            contentItem: ListView {
                                implicitHeight: contentHeight
                                model: providerBox.delegateModel; clip: true
                            }
                        }
                    }

                    Rectangle {
                        width: 28; height: 28
                        color: refreshArea.containsMouse ? hoverCol : "transparent"
                        border.color: cyanFade; border.width: 1; radius: 3

                        Text { anchors.centerIn: parent; text: "↺"; color: cyan; font.pixelSize: 16 }
                        MouseArea {
                            id: refreshArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: stockFetcher.refreshNow()
                        }
                    }
                }
            }

            // -- watchlistModel column headers -------------------------------------
            Rectangle {
                Layout.fillWidth: true
                height: 24
                color: panelAlt
                border.color: borderCol
                border.width: 1

                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    leftPadding: 14; spacing: 0
                    Text { text: "SYMBOL"; color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 80 }
                    Text { text: "NAME";   color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 180 }
                    Text { text: "PRICE";  color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 80 }
                    Text { text: "CHG%";   color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 70 }
                }
            }

            // -- watchlistModel rows -----------------------------------------------
            ListView {
                id: watchlistModelView
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: watchlistModel
                clip: true
                property string selectedSymbol: ""

                delegate: Rectangle {
                    width: watchlistModelView.width
                    height: 42

                    color: watchlistModelView.selectedSymbol === symbol
                           ? cyanFade
                           : (rowArea.containsMouse ? hoverCol : (index % 2 === 0 ? panel : bg))

                    border.color: borderCol
                    border.width: 1


                    // ---------------------------------------------
                    // LEFT CLICK AREA (EXCLUDES DELETE BUTTON)
                    // ---------------------------------------------
                    MouseArea {
                        id: rowArea
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.right: removeBtnArea.left

                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            watchlistModelView.selectedSymbol = symbol
                            historyModel.setSymbol(symbol)
                            stockFetcher.fetchHistory(symbol)
                            tradeTickModel.symbol = symbol
                        }
                    }


                    // ---------------------------------------------
                    // DELETE BUTTON AREA
                    // ---------------------------------------------
                    Item {
                        id: removeBtnArea
                        width: 32
                        height: parent.height
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter


                        Rectangle {
                            id: removeBtn
                            width: 22
                            height: 22
                            radius: 11
                            anchors.centerIn: parent

                            color: removeArea.containsMouse ? redCol : "transparent"
                            border.color: removeArea.containsMouse ? redCol : "#333"
                            border.width: 1
                            opacity: 0.85

                            Text {
                                anchors.centerIn: parent
                                text: "×"
                                color: removeArea.containsMouse ? "white" : "#666"
                                font.pixelSize: 14
                            }

                            MouseArea {
                                id: removeArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor

                                onClicked: {
                                    watchlistModel.removeStock(symbol)
                                }
                            }
                        }
                    }


                    // ---------------------------------------------
                    // ROW CONTENT
                    // ---------------------------------------------
                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        leftPadding: 14
                        spacing: 0

                        Text { text: symbol; color: cyan; font.pixelSize: 13; font.bold: true; width: 80 }
                        Text { text: name; color: cyanDim; font.pixelSize: 12; width: 180; elide: Text.ElideRight }

                        Text {
                            text: price > 0 ? price.toFixed(2) : "—"
                            color: price > 0 ? cyan : seablue
                            font.pixelSize: 13
                            font.bold: true
                            width: 80
                        }

                        Text {
                            readonly property double pct: changePct
                            text: price > 0 ? (pct >= 0 ? "+" : "") + pct.toFixed(2) + "%" : "—"
                            color: pct > 0 ? greenCol : (pct < 0 ? redCol : seablue)
                            font.pixelSize: 12
                            width: 70
                        }
                    }
                }
                Column {
                    anchors.centerIn: parent
                    visible: watchlistModel.count === 0
                    spacing: 8

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "→"; color: cyanFade; font.pixelSize: 28
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Search stocks on the right"
                        color: "#666"; font.pixelSize: 13
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "and add them to your watchlistModel"
                        color: seablue; font.pixelSize: 12
                    }
                }
            }

            // -- Chart --------------------------------------------------------
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 200
                color: panel
                border.color: borderCol
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 0

                    Text {
                        visible: watchlistModelView.selectedSymbol !== ""
                        text: watchlistModelView.selectedSymbol + "  —  1 YEAR"
                        color: cyanDim; font.pixelSize: 10; font.letterSpacing: 2
                        leftPadding: 8; topPadding: 4
                    }

                    ChartView {
                        id: chart
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        antialiasing: true
                        theme: ChartView.ChartThemeDark
                        backgroundColor: "transparent"
                        plotAreaColor: "transparent"
                        legend.visible: false
                        margins.top: 4; margins.bottom: 4
                        margins.left: 4; margins.right: 4

                        ValueAxis { id: xAxis; labelsVisible: false; gridVisible: false; lineVisible: false; color: "transparent" }
                        ValueAxis { id: yAxis; labelsColor: "#555"; gridLineColor: "#1a1a1a"; labelFormat: "%.0f" }

                        LineSeries {
                            id: priceSeries; axisX: xAxis; axisY: yAxis
                            color: cyan; width: 1.5
                        }

                        function rebuildChart() {
                            priceSeries.clear()
                            const count = historyModel.rowCount()
                            if (count === 0) return
                            let minY = 999999, maxY = -999999
                            for (let i = 0; i < count; i++) {
                                const p = historyModel.priceAt(i)
                                priceSeries.append(i, p)
                                if (p < minY) minY = p
                                if (p > maxY) maxY = p
                            }
                            xAxis.min = 0; xAxis.max = Math.max(1, count - 1)
                            yAxis.min = minY * 0.99; yAxis.max = maxY * 1.01
                        }

                        Connections {
                            target: historyModel
                            function onModelReset()    { Qt.callLater(chart.rebuildChart) }
                            function onSymbolChanged() { Qt.callLater(chart.rebuildChart) }
                        }

                        Text {
                            anchors.centerIn: parent
                            visible: historyModel.rowCount() === 0 && watchlistModelView.selectedSymbol === ""
                            text: "Click a stock to see its chart"
                            color: "#2a2a2a"; font.pixelSize: 12
                        }
                    }
                }
            }

            // -- Order Book / Trade Tape ---------------------------------------
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 260
                color: panel
                border.color: borderCol
                border.width: 1
                visible: watchlistModelView.selectedSymbol !== ""

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Header
                    Rectangle {
                        Layout.fillWidth: true
                        height: 34
                        color: panelAlt

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14

                            Text {
                                text: watchlistModelView.selectedSymbol + "  LEVEL 1 + TAPE"
                                color: cyan
                                font.pixelSize: 11
                                font.letterSpacing: 2
                                font.bold: true
                            }

                            Item { Layout.fillWidth: true }

                            // Spread badge
                            Rectangle {
                                visible: watchlistModel.ask > 0 && watchlistModel.bid > 0
                                width: spreadLabel.implicitWidth + 14
                                height: 20; radius: 3
                                color: cyanFade

                                Text {
                                    id: spreadLabel
                                    anchors.centerIn: parent
                                    text: "SPREAD  $" + tradeTickModel.spread.toFixed(3) +
                                          "  (" + tradeTickModel.spreadPct.toFixed(3) + "%)"
                                    color: cyanDim
                                    font.pixelSize: 10
                                    font.letterSpacing: 1
                                }
                            }
                        }
                    }

                    // -- Top of book --------------------------------------------
                    Rectangle {
                        Layout.fillWidth: true
                        height: 54
                        color: "#070707"

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 0
                            spacing: 0

                            // BID side
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "transparent"

                                // Volume bar behind the text
                                Rectangle {
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                    width: watchlistModel.bid > 0 && watchlistModel.ask > 0
                                           ? parent.width * (watchlistModel.bidSize /
                                             Math.max(watchlistModel.bidSize + watchlistModel.askSize, 1))
                                           : 0
                                    color: greenCol
                                    opacity: 0.08
                                    Behavior on width { NumberAnimation { duration: 200 } }
                                }

                                ColumnLayout {
                                    anchors.centerIn: parent
                                    spacing: 2

                                    Text {
                                        Layout.alignment: Qt.AlignHCenter
                                        text: "BID"
                                        color: seablue; font.pixelSize: 9; font.letterSpacing: 2
                                    }
                                    Text {
                                        Layout.alignment: Qt.AlignHCenter
                                        text: watchlistModel.bid > 0
                                              ? "$" + watchlistModelModel.bid.toFixed(2) : "—"
                                        color: greenCol
                                        font.pixelSize: 16; font.bold: true
                                    }
                                    Text {
                                        Layout.alignment: Qt.AlignHCenter
                                        text: watchlistModel.bidSize > 0
                                              ? watchlistModel.bidSize + " sh" : ""
                                        color: "#555"; font.pixelSize: 10
                                    }
                                }
                            }

                            // Center divider with last price
                            Rectangle {
                                width: 80; Layout.fillHeight: true
                                color: "#0a0a0a"
                                border.color: borderCol; border.width: 1

                                ColumnLayout {
                                    anchors.centerIn: parent
                                    spacing: 1

                                    Text {
                                        Layout.alignment: Qt.AlignHCenter
                                        text: "LAST"
                                        color: "#333"; font.pixelSize: 9; font.letterSpacing: 2
                                    }
                                    Text {
                                        Layout.alignment: Qt.AlignHCenter
                                        readonly property var entry: watchlistModelView.selectedSymbol !== "" ? null : null
                                        text: {
                                            // find price for selected symbol in watchlistModel
                                            for (var i = 0; i < watchlistModel.count; i++) {
                                                var idx = watchlistModel.index(i, 0)
                                                // use tradeTickModel last tick price as fallback
                                            }
                                            return tradeTickModel.count > 0
                                                   ? "$" + watchlistModel.bid.toFixed(2) : "—"
                                        }
                                        color: cyan; font.pixelSize: 13; font.bold: true
                                    }
                                }
                            }

                            // ASK side
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "transparent"

                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                    width: watchlistModel.bid > 0 && watchlistModel.ask > 0
                                           ? parent.width * (watchlistModel.askSize /
                                             Math.max(watchlistModel.bidSize + watchlistModel.askSize, 1))
                                           : 0
                                    color: redCol
                                    opacity: 0.08
                                    Behavior on width { NumberAnimation { duration: 200 } }
                                }

                                ColumnLayout {
                                    anchors.centerIn: parent
                                    spacing: 2

                                    Text {
                                        Layout.alignment: Qt.AlignHCenter
                                        text: "ASK"
                                        color: seablue; font.pixelSize: 9; font.letterSpacing: 2
                                    }
                                    Text {
                                        Layout.alignment: Qt.AlignHCenter
                                        text: watchlistModel.ask > 0
                                              ? "$" + watchlistModel.ask.toFixed(2) : "—"
                                        color: redCol
                                        font.pixelSize: 16; font.bold: true
                                    }
                                    Text {
                                        Layout.alignment: Qt.AlignHCenter
                                        text: watchlistModel.askSize > 0
                                              ? watchlistModel.askSize + " sh" : ""
                                        color: "#555"; font.pixelSize: 10
                                    }
                                }
                            }
                        }
                    }

                    // -- Tape column headers ------------------------------------
                    Rectangle {
                        Layout.fillWidth: true; height: 22
                        color: "#050505"

                        Row {
                            anchors.verticalCenter: parent.verticalCenter
                            leftPadding: 12; spacing: 0
                            Text { text: "TIME";  color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 72 }
                            Text { text: "PRICE"; color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 90 }
                            Text { text: "SIZE";  color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 80 }
                            Text { text: "SIDE";  color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 60 }
                            Text { text: "EXCH";  color: seablue; font.pixelSize: 10; font.letterSpacing: 2 }
                        }
                    }

                    // -- Trade tape ---------------------------------------------
                    ListView {
                        id: tapeView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: tradeTickModel
                        clip: true

                        add: Transition {
                            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 150 }
                        }

                        delegate: Rectangle {
                            width: tapeView.width; height: 26
                            color: index % 2 === 0 ? panel : bg

                            readonly property bool isBuy:  tickSide === "buy"
                            readonly property bool isSell: tickSide === "sell"

                            // Left accent
                            Rectangle {
                                width: 2; height: parent.height
                                color: isBuy ? greenCol : isSell ? redCol : seablue
                            }

                            Row {
                                anchors.verticalCenter: parent.verticalCenter
                                leftPadding: 12; spacing: 0

                                Text {
                                    text: tickTime
                                    color: seablue; font.pixelSize: 11
                                    font.family: "Courier New"; width: 72
                                }
                                Text {
                                    text: "$" + tickPrice.toFixed(2)
                                    color: isBuy ? greenCol : isSell ? redCol : cyanDim
                                    font.pixelSize: 12; font.bold: true; width: 90
                                }
                                Text {
                                    text: tickSize.toLocaleString()
                                    color: "#666"; font.pixelSize: 11; width: 80
                                }
                                // SIDE badge
                                Rectangle {
                                    width: 42; height: 16; radius: 2
                                    anchors.verticalCenter: parent.verticalCenter
                                    visible: tickSide !== "" && tickSide !== "unknown"
                                    color: isBuy ? "#002211" : "#220011"

                                    Text {
                                        anchors.centerIn: parent
                                        text: isBuy ? "BUY" : "SELL"
                                        color: isBuy ? greenCol : redCol
                                        font.pixelSize: 9; font.bold: true; font.letterSpacing: 1
                                    }
                                }
                                // Spacer when side badge is hidden
                                Item {
                                    width: 42; height: 1
                                    visible: tickSide === "" || tickSide === "unknown"
                                }

                                // EXCHANGE badge
                                Rectangle {
                                    visible: tickExchange !== "" && tickExchange !== "—"
                                    width: exchLabel.implicitWidth + 10
                                    height: 16; radius: 2
                                    anchors.verticalCenter: parent.verticalCenter
                                    color: "#001520"
                                    border.color: "#003344"; border.width: 1

                                    Text {
                                        id: exchLabel
                                        anchors.centerIn: parent
                                        text: tickExchange
                                        color: cyanDim
                                        font.pixelSize: 9
                                        font.letterSpacing: 1
                                        font.family: "Courier New"
                                    }
                                }
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            visible: tradeTickModel.count === 0
                            text: "Waiting for trades…"
                            color: seablue; font.pixelSize: 11
                        }
                    }
                }
            }

            // -- Positions panel ----------------------------------------------
                       Rectangle {
                           Layout.fillWidth: true
                           Layout.preferredHeight: 220
                           color: panel
                           border.color: borderCol
                           border.width: 1
                           visible: hasAlpaca

                           ColumnLayout {
                               anchors.fill: parent
                               spacing: 0

                               // Header
                               Rectangle {
                                   Layout.fillWidth: true
                                   height: 34
                                   color: panelAlt

                                   RowLayout {
                                       anchors.fill: parent
                                       anchors.leftMargin: 14
                                       anchors.rightMargin: 14

                                       Text {
                                           text: positionModel.isPaper ? "PAPER POSITIONS" : "LIVE POSITIONS"
                                           color: positionModel.isPaper ? orange : redCol
                                           font.pixelSize: 11
                                           font.letterSpacing: 3
                                           font.bold: true
                                       }

                                       // Paper badge
                                       Rectangle {
                                           visible: positionModel.isPaper
                                           width: paperLabel.implicitWidth + 10
                                           height: 18; radius: 3
                                           color: "#2a1f00"
                                           Text {
                                               id: paperLabel
                                               anchors.centerIn: parent
                                               text: "PAPER"
                                               color: orange
                                               font.pixelSize: 9
                                               font.letterSpacing: 2
                                               font.bold: true
                                           }
                                       }

                                       Item { Layout.fillWidth: true }

                                       // Loading indicator
                                       Text {
                                           text: "↺"
                                           color: cyanFade
                                           font.pixelSize: 14
                                           visible: positionModel.loading
                                           RotationAnimator on rotation {
                                               loops: Animation.Infinite
                                               running: positionModel.loading
                                               from: 0; to: 360; duration: 1000
                                           }
                                       }

                                       // Total P&L
                                       Text {
                                           readonly property double pl: positionModel.totalPL
                                           text: (pl >= 0 ? "+" : "") + "$" + pl.toFixed(2)
                                           color: pl >= 0 ? greenCol : redCol
                                           font.pixelSize: 12
                                           font.bold: true
                                           visible: positionModel.count > 0
                                       }

                                       // Refresh button
                                       Rectangle {
                                           width: 24; height: 24; radius: 3
                                           color: posRefreshArea.containsMouse ? hoverCol : "transparent"
                                           border.color: cyanFade; border.width: 1

                                           Text {
                                               anchors.centerIn: parent
                                               text: "↺"; color: cyanDim; font.pixelSize: 13
                                           }
                                           MouseArea {
                                               id: posRefreshArea
                                               anchors.fill: parent; hoverEnabled: true
                                               cursorShape: Qt.PointingHandCursor
                                               onClicked: positionModel.refresh()
                                           }
                                       }
                                   }
                               }

                               // Column headers
                               Rectangle {
                                   Layout.fillWidth: true; height: 22
                                   color: "#070707"

                                   Row {
                                       anchors.verticalCenter: parent.verticalCenter
                                       leftPadding: 12; spacing: 0
                                       Text { text: "SYM";   color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 60 }
                                       Text { text: "QTY";   color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 50 }
                                       Text { text: "ENTRY"; color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 68 }
                                       Text { text: "PRICE"; color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 68 }
                                       Text { text: "P&L";   color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 80 }
                                       Text { text: "%";     color: seablue; font.pixelSize: 10; font.letterSpacing: 2 }
                                   }
                               }

                               // Positions list
                               ListView {
                                   id: positionsView
                                   Layout.fillWidth: true
                                   Layout.fillHeight: true
                                   model: positionModel
                                   clip: true

                                   delegate: Rectangle {
                                       width: positionsView.width
                                       height: 34
                                       color: posRowArea.containsMouse ? hoverCol
                                            : (index % 2 === 0 ? panel : bg)
                                       border.color: borderCol; border.width: 1

                                       readonly property bool isProfit: unrealizedPL >= 0

                                       // Left accent — green for profit, red for loss
                                       Rectangle {
                                           width: 2; height: parent.height
                                           color: isProfit ? greenCol : redCol
                                           opacity: 0.7
                                       }

                                       Row {
                                           anchors.verticalCenter: parent.verticalCenter
                                           leftPadding: 12; spacing: 0

                                           Text {
                                               text: symbol; color: cyan
                                               font.pixelSize: 12; font.bold: true; width: 60
                                           }
                                           Text {
                                               text: qty % 1 === 0 ? qty.toFixed(0) : qty.toFixed(2)
                                               color: side === "short" ? redCol : cyanDim
                                               font.pixelSize: 11; width: 50
                                           }
                                           Text {
                                               text: "$" + avgEntry.toFixed(2)
                                               color: "#666"; font.pixelSize: 11; width: 68
                                           }
                                           Text {
                                               text: "$" + currentPrice.toFixed(2)
                                               color: cyanDim; font.pixelSize: 11; width: 68
                                           }
                                           Text {
                                               text: (isProfit ? "+" : "") + "$" + unrealizedPL.toFixed(2)
                                               color: isProfit ? greenCol : redCol
                                               font.pixelSize: 11; font.bold: true; width: 80
                                           }
                                           Text {
                                               readonly property double pct: unrealizedPLPct
                                               text: (pct >= 0 ? "+" : "") + pct.toFixed(2) + "%"
                                               color: pct >= 0 ? greenCol : redCol
                                               font.pixelSize: 11
                                           }
                                       }

                                       // Close button — appears on hover
                                       Rectangle {
                                           anchors.right: parent.right
                                           anchors.rightMargin: 8
                                           anchors.verticalCenter: parent.verticalCenter

                                           width: 46
                                           height: 22
                                           radius: 3

                                           visible: posRowArea.containsMouse || closePosArea.containsMouse

                                           color: closePosArea.containsMouse ? "#330000" : "transparent"
                                           border.color: closePosArea.containsMouse ? redCol : "#333"
                                           border.width: 1

                                           z: 2

                                           Text {
                                               anchors.centerIn: parent
                                               text: "CLOSE"
                                               color: closePosArea.containsMouse ? redCol : "#555"
                                               font.pixelSize: 9
                                               font.letterSpacing: 1
                                           }

                                           MouseArea {
                                               id: closePosArea
                                               anchors.fill: parent
                                               hoverEnabled: true
                                               cursorShape: Qt.PointingHandCursor

                                               z: 3   // ensures hover is always captured first

                                               onClicked: {
                                                   positionModel.closePosition(symbol)
                                               }
                                           }
                                       }

                                       // Row area
                                       MouseArea {
                                           id: posRowArea
                                           anchors.fill: parent
                                           hoverEnabled: true
                                           z: 1   // below close button
                                           onClicked: mouse.accepted = false
                                       }
                                   }

                                   // Empty state
                                   Column {
                                       anchors.centerIn: parent
                                       visible: positionModel.count === 0 && !positionModel.loading
                                       spacing: 6
                                       Text {
                                           anchors.horizontalCenter: parent.horizontalCenter
                                           text: "No open positions"
                                           color: "#333"; font.pixelSize: 12
                                       }
                                       Text {
                                           anchors.horizontalCenter: parent.horizontalCenter
                                           text: positionModel.providerName
                                           color: seablue; font.pixelSize: 10
                                           font.letterSpacing: 1
                                       }
                                   }
                               }
                           }
                       }
        }

        // Vertical divider
        Rectangle { width: 1; Layout.fillHeight: true; color: borderCol }

        // ════════════════════════════════════════════════════════════════════
        // RIGHT PANEL — Browse + Alerts
        // ════════════════════════════════════════════════════════════════════
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // -- Search header ------------------------------------------------
            Rectangle {
                Layout.fillWidth: true
                height: 44
                color: panel
                border.color: borderCol
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14; anchors.rightMargin: 14
                    spacing: 10

                    Text {
                        text: "S&P 500"
                        color: cyan; font.pixelSize: 11; font.letterSpacing: 3; font.bold: true
                    }

                    Rectangle {
                        Layout.fillWidth: true; height: 28
                        color: bg
                        border.color: searchField.activeFocus ? cyan : cyanFade
                        border.width: 1; radius: 3

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 8; anchors.rightMargin: 8
                            spacing: 6

                            Text { text: "⌕"; color: seablue; font.pixelSize: 14 }

                            TextInput {
                                id: searchField
                                Layout.fillWidth: true
                                color: cyan; font.pixelSize: 12
                                verticalAlignment: TextInput.AlignVCenter; clip: true
                                onTextChanged: browseModel.filter = text

                                Text {
                                    anchors.fill: parent
                                    text: "Search symbol or name…"
                                    color: "#333"; font.pixelSize: 12
                                    verticalAlignment: Text.AlignVCenter
                                    visible: !searchField.text && !searchField.activeFocus
                                }
                            }

                            Text {
                                text: "×"; color: seablue; font.pixelSize: 14
                                visible: searchField.text.length > 0
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: searchField.text = ""
                                }
                            }
                        }
                    }
                }
            }

            // -- Browse column headers ----------------------------------------
            Rectangle {
                Layout.fillWidth: true; height: 24
                color: panelAlt; border.color: borderCol; border.width: 1

                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    leftPadding: 14; spacing: 0
                    Text { text: "SYMBOL"; color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 90 }
                    Text { text: "NAME";   color: seablue; font.pixelSize: 10; font.letterSpacing: 2 }
                }
            }

            // -- Browse list --------------------------------------------------
            ListView {
                id: browseView
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: browseModel
                clip: true

                delegate: Rectangle {
                    width: browseView.width; height: 38
                    color: browseArea.containsMouse ? hoverCol : (index % 2 === 0 ? panel : bg)
                    border.color: borderCol; border.width: 1

                    // watchlistModel.count is included so the binding re-evaluates
                    // whenever an item is added or removed from the watchlistModel.
                    readonly property bool inWatch: watchlistModel.count, watchlistModel.contains(symbol)

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14; anchors.rightMargin: 10

                        Text {
                            text: symbol
                            color: inWatch ? cyanDim : cyan
                            font.pixelSize: 12; font.bold: true; width: 90
                        }
                        Text {
                            text: name
                            color: inWatch ? seablue : cyanDim
                            font.pixelSize: 12
                            Layout.fillWidth: true; elide: Text.ElideRight
                        }

                        Rectangle {
                            width: 56; height: 22; radius: 3
                            color: inWatch ? "transparent" : (addArea.containsMouse ? cyan : "transparent")
                            border.color: inWatch ? "#2a2a2a" : (addArea.containsMouse ? cyan : cyanFade)
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: inWatch ? "✓" : "+ ADD"
                                color: inWatch ? seablue : (addArea.containsMouse ? bg : cyanDim)
                                font.pixelSize: 10; font.letterSpacing: 1; font.bold: true
                            }

                            MouseArea {
                                id: addArea; anchors.fill: parent; hoverEnabled: true
                                cursorShape: inWatch ? Qt.ArrowCursor : Qt.PointingHandCursor
                                enabled: !inWatch && watchlistModel.count < 20
                                onClicked: watchlistModel.addStock(symbol, name)
                            }
                        }
                    }

                    MouseArea {
                        id: browseArea; anchors.fill: parent; hoverEnabled: true
                        propagateComposedEvents: true
                        onClicked: mouse.accepted = false
                    }
                }
            }

            // Horizontal divider
            Rectangle { Layout.fillWidth: true; height: 1; color: borderCol }

            // -- Alerts header ------------------------------------------------
            Rectangle {
                Layout.fillWidth: true; height: 36
                color: panel; border.color: borderCol; border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14; anchors.rightMargin: 14

                    Text {
                        text: "SIGNALS"
                        color: cyan; font.pixelSize: 11; font.letterSpacing: 3; font.bold: true
                    }

                    // Live badge
                    Rectangle {
                        width: 6; height: 6; radius: 3
                        color: greenCol
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.2; duration: 800 }
                            NumberAnimation { to: 1.0; duration: 800 }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: alertModel.count + " alerts"
                        color: seablue; font.pixelSize: 10; font.letterSpacing: 1
                    }

                    // TEST button
                    Rectangle {
                        width: 52; height: 22; radius: 3
                        color: testArea.containsMouse ? "#001a0d" : "transparent"
                        border.color: testArea.containsMouse ? greenCol : "#2a2a2a"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent; text: "TEST"
                            color: testArea.containsMouse ? greenCol : seablue
                            font.pixelSize: 9; font.letterSpacing: 1
                        }
                        MouseArea {
                            id: testArea; anchors.fill: parent; hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: alertModel.addTestSignal(
                                watchlistModelView.selectedSymbol !== "" ? watchlistModelView.selectedSymbol : "TEST"
                            )
                        }
                    }

                    // CLEAR button
                    Rectangle {
                        width: 52; height: 22; radius: 3
                        color: clearArea.containsMouse ? "#1a0000" : "transparent"
                        border.color: clearArea.containsMouse ? redCol : "#2a2a2a"
                        border.width: 1
                        visible: alertModel.count > 0

                        Text {
                            anchors.centerIn: parent; text: "CLEAR"
                            color: clearArea.containsMouse ? redCol : seablue
                            font.pixelSize: 9; font.letterSpacing: 1
                        }
                        MouseArea {
                            id: clearArea; anchors.fill: parent; hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: alertModel.clear()
                        }
                    }
                }
            }

            // -- Alerts column headers ----------------------------------------
            Rectangle {
                Layout.fillWidth: true; height: 22
                color: panelAlt; border.color: borderCol; border.width: 1

                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    leftPadding: 10; spacing: 0
                    Text { text: "";   color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 22 }
                    Text { text: "SYM";    color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 60 }
                    Text { text: "SIGNAL"; color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 160 }
                    Text { text: "VALUE";  color: seablue; font.pixelSize: 10; font.letterSpacing: 2; width: 70 }
                    Text { text: "TIME";   color: seablue; font.pixelSize: 10; font.letterSpacing: 2 }
                }
            }

            // -- Alerts list --------------------------------------------------
            ListView {
                id: alertsView
                Layout.fillWidth: true
                Layout.preferredHeight: 180
                model: alertModel
                clip: true

                delegate: Rectangle {
                    width: alertsView.width; height: 34
                    color: alertRowArea.containsMouse ? hoverCol : (index % 2 === 0 ? panel : bg)
                    border.color: borderCol; border.width: 1

                    readonly property color typeColor:
                        signalType === "bullish" ? greenCol :
                        signalType === "bearish" ? redCol : orange

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        leftPadding: 10; spacing: 0

                        // Colored dot for type
                        Item {
                            width: 22; height: 34
                            Rectangle {
                                width: 6; height: 6; radius: 3
                                anchors.centerIn: parent
                                color: typeColor
                            }
                        }

                        Text {
                            text: symbol; color: cyan
                            font.pixelSize: 12; font.bold: true; width: 60
                            anchors.verticalCenter: undefined
                            height: 34
                            verticalAlignment: Text.AlignVCenter
                        }
                        Text {
                            text: signalName; color: typeColor
                            font.pixelSize: 11; width: 160
                            height: 34; verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                        Text {
                            text: signalValue.toFixed(2)
                            color: "#666"; font.pixelSize: 11; width: 70
                            height: 34; verticalAlignment: Text.AlignVCenter
                        }
                        Text {
                            text: signalTime; color: seablue
                            font.pixelSize: 10
                            height: 34; verticalAlignment: Text.AlignVCenter
                        }
                    }

                    // Subtle left accent bar
                    Rectangle {
                        width: 2; height: parent.height
                        color: typeColor; opacity: 0.6
                    }

                    MouseArea {
                        id: alertRowArea; anchors.fill: parent
                        hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            alertPopup.alertSymbol = symbol
                            alertPopup.alertName   = signalName
                            alertPopup.alertType   = signalType
                            alertPopup.alertDesc   = description
                            alertPopup.alertTime   = signalTime
                            alertPopup.alertValue  = signalValue
                            alertPopup.open()
                        }
                    }
                }

                // Empty state
                Text {
                    anchors.centerIn: parent
                    visible: alertModel.count === 0
                    text: "Signals appear after fetching history\n(click a watchlistModel stock)"
                    color: "#333"; font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter; lineHeight: 1.6
                }
            }
        }
    }
}
