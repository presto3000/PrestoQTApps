import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: "S&P 500 Stock Viewer"

    // -------------------------
    // THEME
    // -------------------------
    property color bg: "#000000"
    property color panel: "#0a0a0a"
    property color cyan: "#00ffff"
    property color cyanDim: "#00cccc"
    property color hover: "#001a1a"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // =========================
        // PROVIDER COMBOBOX
        // =========================
        ComboBox {
            id: providerBox

            model: [
                "Stooq",
                "Yahoo",
                "AlphaVantage",
                "Finnhub"
            ]

            onCurrentIndexChanged: {
                stockFetcher.setProvider(currentIndex)
            }

            background: Rectangle {
                color: bg
                border.color: cyan
                border.width: 1
                radius: 4
            }

            contentItem: Text {
                text: providerBox.displayText
                color: cyan
                verticalAlignment: Text.AlignVCenter
                leftPadding: 10
            }

            delegate: ItemDelegate {
                width: providerBox.width

                background: Rectangle {
                    color: highlighted ? hover : bg
                    border.color: "#002a2a"
                }

                contentItem: Text {
                    text: modelData
                    color: highlighted ? cyan : cyanDim
                }
            }
        }

        // =========================
        // STOCK LIST
        // =========================
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: stockModel
            clip: true

            delegate: Rectangle {
                width: ListView.view.width
                height: 44

                color: index % 2 === 0 ? panel : bg

                border.color: "#101010"
                border.width: 1

                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 40
                    leftPadding: 10

                    Text {
                        text: symbol
                        color: cyan
                        width: 80
                        font.bold: true
                    }

                    Text {
                        text: name
                        color: cyanDim
                        width: 300
                        elide: Text.ElideRight
                    }

                    Text {
                        text: price.toFixed(2)
                        color: price > 0 ? cyan : "red"
                        font.bold: true
                    }
                }

                // hover highlight
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true

                    onEntered: parent.color = hover
                    onExited: parent.color = index % 2 === 0 ? panel : bg

                    onClicked: {
                        stockFetcher.fetchHistory(symbol)
                        historyModel.setSymbol(symbol)
                    }
                }
            }
        }
        // ChartView {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 250
        //     Layout.minimumHeight: 200

        //     title: "Line Chart"
        //     antialiasing: true

        //     LineSeries {
        //         name: "Line"
        //         XYPoint { x: 0; y: 0 }
        //         XYPoint { x: 1.1; y: 2.1 }
        //         XYPoint { x: 1.9; y: 3.3 }
        //         XYPoint { x: 2.1; y: 2.1 }
        //         XYPoint { x: 2.9; y: 4.9 }
        //         XYPoint { x: 3.4; y: 3.0 }
        //         XYPoint { x: 4.1; y: 3.3 }
        //     }
        // }

        ChartView {
            id: chart
            Layout.fillWidth: true
            Layout.preferredHeight: 250
            antialiasing: true
            theme: ChartView.ChartThemeDark

            ValueAxis {
                id: xAxis
            }

            ValueAxis {
                id: yAxis
            }

            LineSeries {
                id: series
                axisX: xAxis
                axisY: yAxis
            }

            function rebuildChart() {
                series.clear()

                let minY = 999999
                let maxY = -999999

                let count = historyModel.rowCount()

                for (let i = 0; i < count; i++) {
                    let price = historyModel.priceAt(i)
                    series.append(i, price)

                    if (price < minY) minY = price
                    if (price > maxY) maxY = price
                }

                // FORCE correct scaling
                xAxis.min = 0
                xAxis.max = Math.max(1, count - 1)

                yAxis.min = minY
                yAxis.max = maxY
            }

            Connections {
                target: historyModel

                function onModelReset() {
                    Qt.callLater(chart.rebuildChart)
                }

                function onSymbolChanged() {
                    Qt.callLater(chart.rebuildChart)
                }
            }

            Component.onCompleted: {
                Qt.callLater(rebuildChart)
            }
        }
    }
}