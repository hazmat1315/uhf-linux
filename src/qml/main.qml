import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: win
    visible: true
    width: 1100
    height: 720
    title: "UHF Linux"
    color: "#121216"

    readonly property color sidebarBg: "#17171d"
    readonly property color accent: "#e23b3b"
    readonly property color textColor: "#f2f2f5"
    readonly property color subText: "#9a9aa5"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ---- Sidebar ----
        Rectangle {
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            color: win.sidebarBg

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 6

                Label {
                    text: "UHF"
                    color: win.accent
                    font.pixelSize: 26
                    font.bold: true
                    Layout.bottomMargin: 18
                }

                Repeater {
                    model: ["Home", "Live TV", "Movies", "Series", "Search", "Settings"]
                    delegate: ItemDelegate {
                        required property string modelData
                        required property int index
                        Layout.fillWidth: true
                        text: modelData
                        highlighted: index === 1 // Live TV active in this slice
                        contentItem: Label {
                            text: parent.text
                            color: parent.highlighted ? win.accent : win.textColor
                            font.pixelSize: 15
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // ---- Main content ----
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Playlist loader bar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                color: "#1d1d24"
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10
                    TextField {
                        id: pathField
                        Layout.fillWidth: true
                        placeholderText: "Path to a local .m3u / .m3u8 file"
                        color: win.textColor
                        selectByMouse: true
                    }
                    Button {
                        text: "Load"
                        onClicked: app.loadM3uFile(pathField.text)
                    }
                }
            }

            // Channel list
            ListView {
                id: list
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: app.channels
                ScrollBar.vertical: ScrollBar {}

                delegate: ItemDelegate {
                    required property int index
                    required property string name
                    required property string group
                    width: ListView.view.width
                    onDoubleClicked: app.play(index)
                    onClicked: list.currentIndex = index
                    highlighted: ListView.isCurrentItem
                    contentItem: RowLayout {
                        spacing: 12
                        ColumnLayout {
                            spacing: 2
                            Layout.fillWidth: true
                            Label {
                                text: name
                                color: win.textColor
                                font.pixelSize: 15
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                            Label {
                                text: group
                                color: win.subText
                                font.pixelSize: 12
                                visible: group.length > 0
                            }
                        }
                        Button {
                            text: "▶ Play"
                            onClicked: app.play(index)
                        }
                    }
                }

                // Empty hint
                Label {
                    anchors.centerIn: parent
                    visible: list.count === 0
                    text: "Load a playlist to see channels.\nDouble-click a channel to play."
                    horizontalAlignment: Text.AlignHCenter
                    color: win.subText
                    font.pixelSize: 15
                }
            }

            // Status bar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 34
                color: "#17171d"
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    Label {
                        text: app.status
                        color: win.subText
                        font.pixelSize: 12
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    Label {
                        visible: app.nowPlaying.length > 0
                        text: "♪ " + app.nowPlaying
                        color: win.accent
                        font.pixelSize: 12
                    }
                }
            }
        }
    }
}
