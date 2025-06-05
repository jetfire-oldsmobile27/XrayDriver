import QtQuick

Item {
    MouseArea {
        id: topArea
        height: 5
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        cursorShape: Qt.SizeVerCursor
        onPressed: {
            previousY = mouseY
        }
        onMouseYChanged: {
            var dy = mouseY - previousY
            if (root.height - dy >= root.minimumHeight) {
                root.setY(root.y + dy)
                root.setHeight(root.height - dy)
            }
        }
    }

    MouseArea {
        id: bottomArea
        height: 5
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        cursorShape: Qt.SizeVerCursor
        onPressed: {
            previousY = mouseY
        }
        onMouseYChanged: {
            var dy = mouseY - previousY
            if (root.height + dy >= root.minimumHeight) {
                root.setHeight(root.height + dy)
            }
        }
    }

    MouseArea {
        id: leftArea
        width: 5
        anchors {
            top: topArea.bottom
            bottom: bottomArea.top
            left: parent.left
        }
        cursorShape: Qt.SizeHorCursor
        onPressed: {
            previousX = mouseX
        }
        onMouseXChanged: {
            var dx = mouseX - previousX
            if (root.width - dx >= root.minimumWidth) {
                root.setX(root.x + dx)
                root.setWidth(root.width - dx)
            }
        }
    }

    MouseArea {
        id: rightArea
        width: 5
        anchors {
            top: topArea.bottom
            bottom: bottomArea.top
            right: parent.right
        }
        cursorShape: Qt.SizeHorCursor
        onPressed: {
            previousX = mouseX
        }
        onMouseXChanged: {
            var dx = mouseX - previousX
            if (root.width + dx >= root.minimumWidth) {
                root.setWidth(root.width + dx)
            }
        }
    }

    MouseArea {
        id: topLeftArea
        width: 5
        height: 5
        anchors {
            top: parent.top
            left: parent.left
        }
        cursorShape: Qt.SizeFDiagCursor
        onPressed: {
            previousX = mouseX
            previousY = mouseY
        }
        onMouseXChanged: {
            var dx = mouseX - previousX
            if (root.width - dx >= root.minimumWidth) {
                root.setX(root.x + dx)
                root.setWidth(root.width - dx)
            }
        }
        onMouseYChanged: {
            var dy = mouseY - previousY
            if (root.height - dy >= root.minimumHeight) {
                root.setY(root.y + dy)
                root.setHeight(root.height - dy)
            }
        }
    }

    MouseArea {
        id: topRightArea
        width: 5
        height: 5
        anchors {
            top: parent.top
            right: parent.right
        }
        cursorShape: Qt.SizeBDiagCursor
        onPressed: {
            previousX = mouseX
            previousY = mouseY
        }
        onMouseXChanged: {
            var dx = mouseX - previousX
            if (root.width + dx >= root.minimumWidth) {
                root.setWidth(root.width + dx)
            }
        }
        onMouseYChanged: {
            var dy = mouseY - previousY
            if (root.height - dy >= root.minimumHeight) {
                root.setY(root.y + dy)
                root.setHeight(root.height - dy)
            }
        }
    }

    MouseArea {
        id: bottomLeftArea
        width: 5
        height: 5
        anchors {
            bottom: parent.bottom
            left: parent.left
        }
        cursorShape: Qt.SizeBDiagCursor
        onPressed: {
            previousX = mouseX
            previousY = mouseY
        }
        onMouseXChanged: {
            var dx = mouseX - previousX
            if (root.width - dx >= root.minimumWidth) {
                root.setX(root.x + dx)
                root.setWidth(root.width - dx)
            }
        }
        onMouseYChanged: {
            var dy = mouseY - previousY
            if (root.height + dy >= root.minimumHeight) {
                root.setHeight(root.height + dy)
            }
        }
    }

    MouseArea {
        id: bottomRightArea
        width: 5
        height: 5
        anchors {
            bottom: parent.bottom
            right: parent.right
        }
        cursorShape: Qt.SizeFDiagCursor
        onPressed: {
            previousX = mouseX
            previousY = mouseY
        }
        onMouseXChanged: {
            var dx = mouseX - previousX
            if (root.width + dx >= root.minimumWidth) {
                root.setWidth(root.width + dx)
            }
        }
        onMouseYChanged: {
            var dy = mouseY - previousY
            if (root.height + dy >= root.minimumHeight) {
                root.setHeight(root.height + dy)
            }
        }
    }
}
