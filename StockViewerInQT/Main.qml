import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

ApplicationWindow {
    visible: true
    width: 1100
    height: 700
    title: "S&P 500 Stock Viewer"

    minimumWidth: 1100
    maximumWidth: 1100
    minimumHeight: 700
    maximumHeight: 700

    // ── THEME ────────────────────────────────────────────────────────────────
    readonly property color bg:       "#000000"
    readonly property color panel:    "#0a0a0a"
    readonly property color panelAlt: "#0d0d0d"
    readonly property color borderCol: "#1a1a1a"
    readonly property color cyan:     "#00ffff"
    readonly property color cyanDim:  "#00cccc"
    readonly property color cyanFade: "#003333"
    readonly property color hover:    "#001a1a"
    readonly property color red:      "#ff4444"
    readonly property color green:    "#00ff88"

    color: bg

    // === ROOT LAYOUT ====================================================================
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ====================================================================
        // LEFT PANEL — Watchlist + Chart
        // ====================================================================
        ColumnLayout {
            Layout.preferredWidth: 520
            Layout.maximumWidth: 520
            Layout.fillHeight: true
            spacing: 0

            // === Header bar ====================================================================
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
                        text: "WATCHLIST"
                        color: cyan
                        font.pixelSize: 11
                        font.letterSpacing: 3
                        font.bold: true
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: watchlist.count + " / 20"
                        color: watchlist.count >= 20 ? red : cyanDim
                        font.pixelSize: 11
                        font.letterSpacing: 1
                    }

                    // Provider selector
                    ComboBox {
                        id: providerBox
                        implicitWidth: 110
                        implicitHeight: 28
                        model: ["Stooq", "Yahoo"]

                        onCurrentIndexChanged: stockFetcher.setProvider(currentIndex)

                        background: Rectangle {
                            color: bg
                            border.color: cyanFade
                            border.width: 1
                            radius: 3
                        }
                        contentItem: Text {
                            text: providerBox.displayText
                            color: cyanDim
                            font.pixelSize: 11
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 8
                        }
                        delegate: ItemDelegate {
                            width: providerBox.width
                            background: Rectangle {
                                color: highlighted ? hover : bg
                                border.color: "#001a1a"
                            }
                            contentItem: Text {
                                text: modelData
                                color: highlighted ? cyan : cyanDim
                                font.pixelSize: 11
                                leftPadding: 8
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                        popup: Popup {
                            y: providerBox.height
                            width: providerBox.width
                            padding: 0
                            background: Rectangle { color: bg; border.color: cyanFade; border.width: 1 }
                            contentItem: ListView {
                                implicitHeight: contentHeight
                                model: providerBox.delegateModel
                                clip: true
                            }
                        }
                    }

                    // Refresh button
                    Rectangle {
                        width: 28; height: 28
                        color: refreshArea.containsMouse ? hover : "transparent"
                        border.color: cyanFade
                        border.width: 1
                        radius: 3

                        Text {
                            anchors.centerIn: parent
                            text: "↺"
                            color: cyan
                            font.pixelSize: 16
                        }
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

            // ── Watchlist column headers ─────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true
                height: 24
                color: panelAlt
                border.color: borderCol
                border.width: 1

                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    leftPadding: 14
                    spacing: 0

                    Text { text: "SYMBOL"; color: "#444"; font.pixelSize: 10; font.letterSpacing: 2; width: 80 }
                    Text { text: "NAME";   color: "#444"; font.pixelSize: 10; font.letterSpacing: 2; width: 200 }
                    Text { text: "PRICE";  color: "#444"; font.pixelSize: 10; font.letterSpacing: 2; width: 80 }
                    Text { text: "CHG%";   color: "#444"; font.pixelSize: 10; font.letterSpacing: 2; width: 70 }
                }
            }

            // ── Watchlist rows ───────────────────────────────────────────────
            ListView {
                id: watchlistView
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: watchlist
                clip: true

                property string selectedSymbol: ""

                delegate: Rectangle {
                    width: watchlistView.width
                    height: 42
                    color: watchlistView.selectedSymbol === symbol
                           ? cyanFade
                           : (rowArea.containsMouse ? hover : (index % 2 === 0 ? panel : bg))

                    border.color: borderCol
                    border.width: 1

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        leftPadding: 14
                        spacing: 0

                        Text {
                            text: symbol
                            color: cyan
                            font.pixelSize: 13
                            font.bold: true
                            width: 80
                        }
                        Text {
                            text: name
                            color: cyanDim
                            font.pixelSize: 12
                            width: 200
                            elide: Text.ElideRight
                        }
                        Text {
                            text: price > 0 ? price.toFixed(2) : "—"
                            color: price > 0 ? cyan : "#444"
                            font.pixelSize: 13
                            font.bold: true
                            width: 80
                        }
                        Text {
                            readonly property double pct: changePct
                            text: price > 0 ? (pct >= 0 ? "+" : "") + pct.toFixed(2) + "%" : "—"
                            color: pct > 0 ? green : (pct < 0 ? red : "#444")
                            font.pixelSize: 12
                            width: 70
                        }
                    }

                    // Remove button (appears on hover)
                    Rectangle {
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        width: 22; height: 22
                        radius: 11

                        z: 10

                        color: removeArea.containsMouse ? red : "transparent"
                        border.color: removeArea.containsMouse ? red : "#333"
                        border.width: 1
                        visible: rowArea.containsMouse || removeArea.containsMouse
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

                            z: 11

                            cursorShape: Qt.PointingHandCursor

                            onPressed: mouse.accepted = true   // stop propagation
                            onClicked: {
                                console.log("REMOVE", symbol)
                                watchlist.removeStock(symbol)
                            }
                        }
                    }

                    MouseArea {
                        id: rowArea
                        anchors.fill: parent
                        hoverEnabled: true
                        z: 1

                        onClicked: (mouse) => {
                            // ONLY handle click if it didn't come from remove button
                            if (!removeArea.containsMouse) {
                                watchlistView.selectedSymbol = symbol
                                historyModel.setSymbol(symbol)
                                stockFetcher.fetchHistory(symbol)
                            }
                        }
                    }
                }

                // Empty state
                Column {
                    anchors.centerIn: parent
                    visible: watchlist.count === 0
                    spacing: 8

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "→"
                        color: cyanFade
                        font.pixelSize: 28
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Search stocks on the right"
                        color: "#666"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "and add them to your watchlist"
                        color: "#444"
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            // ── Chart ────────────────────────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 220
                color: panel
                border.color: borderCol
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 0

                    Text {
                        visible: watchlistView.selectedSymbol !== ""
                        text: watchlistView.selectedSymbol + "  —  1 YEAR"
                        color: cyanDim
                        font.pixelSize: 10
                        font.letterSpacing: 2
                        leftPadding: 8
                        topPadding: 4
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
                        margins.top: 4
                        margins.bottom: 4
                        margins.left: 4
                        margins.right: 4

                        ValueAxis { id: xAxis; labelsVisible: false; gridVisible: false; lineVisible: false; color: "transparent" }
                        ValueAxis { id: yAxis; labelsColor: "#555";  gridLineColor: "#1a1a1a"; labelFormat: "%.0f" }

                        LineSeries {
                            id: series
                            axisX: xAxis
                            axisY: yAxis
                            color: cyan
                            width: 1.5
                        }

                        function rebuildChart() {
                            series.clear()
                            const count = historyModel.rowCount()
                            if (count === 0) return

                            let minY =  999999
                            let maxY = -999999

                            for (let i = 0; i < count; i++) {
                                const p = historyModel.priceAt(i)
                                series.append(i, p)
                                if (p < minY) minY = p
                                if (p > maxY) maxY = p
                            }

                            xAxis.min = 0
                            xAxis.max = Math.max(1, count - 1)
                            yAxis.min = minY * 0.99
                            yAxis.max = maxY * 1.01
                        }

                        Connections {
                            target: historyModel
                            function onModelReset()   { Qt.callLater(chart.rebuildChart) }
                            function onSymbolChanged() { Qt.callLater(chart.rebuildChart) }
                        }

                        // No data label
                        Text {
                            anchors.centerIn: parent
                            visible: historyModel.rowCount() === 0 && watchlistView.selectedSymbol === ""
                            text: "Click a stock to see its chart"
                            color: "#2a2a2a"
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }

        // Vertical divider
        Rectangle { width: 1; Layout.fillHeight: true; color: border }

        // ════════════════════════════════════════════════════════════════════
        // RIGHT PANEL — Browse / Search S&P 500
        // ════════════════════════════════════════════════════════════════════
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // ── Search header ────────────────────────────────────────────────
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
                    spacing: 10

                    Text {
                        text: "S&P 500"
                        color: cyan
                        font.pixelSize: 11
                        font.letterSpacing: 3
                        font.bold: true
                    }

                    // Search box
                    Rectangle {
                        Layout.fillWidth: true
                        height: 28
                        color: bg
                        border.color: searchField.activeFocus ? cyan : cyanFade
                        border.width: 1
                        radius: 3

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 6

                            Text { text: "⌕"; color: "#444"; font.pixelSize: 14 }

                            TextInput {
                                id: searchField
                                Layout.fillWidth: true
                                color: cyan
                                font.pixelSize: 12
                                verticalAlignment: TextInput.AlignVCenter
                                clip: true
                                onTextChanged: browseModel.filter = text

                                Text {
                                    anchors.fill: parent
                                    text: "Search symbol or name…"
                                    color: "#333"
                                    font.pixelSize: 12
                                    verticalAlignment: Text.AlignVCenter
                                    visible: !searchField.text && !searchField.activeFocus
                                }
                            }

                            // Clear button
                            Text {
                                text: "×"
                                color: "#444"
                                font.pixelSize: 14
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

            // ── Browse column headers ────────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true
                height: 24
                color: panelAlt
                border.color: borderCol
                border.width: 1

                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    leftPadding: 14
                    spacing: 0

                    Text { text: "SYMBOL"; color: "#444"; font.pixelSize: 10; font.letterSpacing: 2; width: 90 }
                    Text { text: "NAME";   color: "#444"; font.pixelSize: 10; font.letterSpacing: 2 }
                }
            }

            // ── Browse list ──────────────────────────────────────────────────
            ListView {
                id: browseView
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: browseModel
                clip: true

                delegate: Rectangle {
                    width: browseView.width
                    height: 38
                    color: browseArea.containsMouse ? hover : (index % 2 === 0 ? panel : bg)
                    border.color: borderCol
                    border.width: 1

                    readonly property bool inWatch: watchlist.contains(symbol)

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 10

                        Text {
                            text: symbol
                            color: inWatch ? cyanDim : cyan
                            font.pixelSize: 12
                            font.bold: true
                            width: 90
                        }
                        Text {
                            text: name
                            color: inWatch ? "#444" : cyanDim
                            font.pixelSize: 12
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }

                        // Add / already-in-watchlist indicator
                        Rectangle {
                            width: 56; height: 22
                            radius: 3
                            color: inWatch
                                   ? "transparent"
                                   : (addArea.containsMouse ? cyan : "transparent")
                            border.color: inWatch ? "#2a2a2a" : (addArea.containsMouse ? cyan : cyanFade)
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: inWatch ? "✓" : "+ ADD"
                                color: inWatch
                                       ? "#444"
                                       : (addArea.containsMouse ? bg : cyanDim)
                                font.pixelSize: 10
                                font.letterSpacing: 1
                                font.bold: true
                            }

                            MouseArea {
                                id: addArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: inWatch ? Qt.ArrowCursor : Qt.PointingHandCursor
                                enabled: !inWatch && watchlist.count < 20
                                onClicked: watchlist.addStock(symbol, name)
                            }
                        }
                    }

                    MouseArea {
                        id: browseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        // hover only — addArea handles the actual click
                        // propagate so addArea (child z-order) still receives clicks
                        propagateComposedEvents: true
                        onClicked: mouse.accepted = false
                    }
                }
            }
        }
    }
}
