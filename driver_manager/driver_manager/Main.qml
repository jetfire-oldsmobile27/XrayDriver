import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Shapes 1.15 as Shapes

Window {
    id: window
    width: 1024
    height: 768
    minimumWidth: 720
    minimumHeight: 800
    visible: true
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowMinMaxButtonsHint

    // ---------- Material-свойства для всего окна --------------------
    Material.theme: Material.Light          // Распространяется на все дочерние элементы :contentReference[oaicite:3]{index=3}
    Material.accent: Material.Green         // Акцентный цвет :contentReference[oaicite:4]{index=4}
    // ----------------------------------------------------------------

    property string buttonsColor: "#BB86FC"
    property string backgroundColor: "white"
    property string panelColor: "#1F1B24"
    property string borderColor: "#292929"
    property string textColor: "#E0E0E0"
    property string hoverColor: "#3700B3"

    WindowControls {}

    Action {
        id: closeHotKey
        text: "&Open"
        shortcut: "Ctrl+Q"
        onTriggered: window.close()
    }
    ToolButton { action: closeHotKey }

    Rectangle {
        id: visibleWindow
        width: window.width
        height: window.height
        radius: 10
        color: window.backgroundColor
        border.color: window.borderColor
        border.width: 1
    }

    // ===================================================================
    // ВЕРХНЯЯ ПАНЕЛЬ (без изменений)
    // ===================================================================
    Rectangle {
        id: title
        x: (window.width - width) / 2
        y: 0
        width: window.width
        height: 70
        radius: 10
        color: window.panelColor
        border.color: window.borderColor
        border.width: 1

        MouseArea {
            id: dragArea
            anchors.fill: parent
            property real dragStartX
            property real dragStartY

            onPressed: (mouse) => {
                           dragStartX = mouse.x
                           dragStartY = mouse.y
                       }
            onPositionChanged: (mouse) => {
                                   var dx = mouse.x - dragStartX
                                   var dy = mouse.y - dragStartY
                                   window.x += dx
                                   window.y += dy
                               }
        }

        Item {
            id: titleRowLike
            anchors.fill: parent

            Text {
                text: "Менеджер драйвера динамической трубки"
                color: window.textColor
                font.family: "Noto Sans"
                font.pointSize: 15
                font.bold: true
                anchors.centerIn: titleRowLike
            }

            Row {
                spacing: 4
                anchors.right: parent.right
                anchors.rightMargin: 20
                anchors.verticalCenter: titleRowLike.verticalCenter

                // Кнопка «Свернуть»
                Item {
                    id: minimizeButton
                    width: 30
                    height: 30

                    Rectangle {
                        id: minimizeButtonRect
                        anchors.verticalCenter: minimizeButton.verticalCenter
                        width: minimizeButton.width * 0.8
                        height: 3
                        color: window.buttonsColor
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: window.lower()
                        onEntered: minimizeButtonRect.color = window.hoverColor
                        onExited: minimizeButtonRect.color = window.buttonsColor
                    }
                }

                // Кнопка «Максимизировать/Свернуть»
                Item {
                    id: maxScaleButton
                    width: 30
                    height: 30
                    property bool isFullscreen: false

                    Image {
                        id: fullScreenStat
                        visible: maxScaleButton.isFullscreen
                        source: "qrc:/fullscreen.svg"
                        anchors.centerIn: parent
                    }
                    Image {
                        id: defaultScreenStat
                        visible: !maxScaleButton.isFullscreen
                        source: "qrc:/fullscreen_exit.svg"
                        anchors.centerIn: parent
                    }
                    MouseArea {
                        hoverEnabled: true
                        anchors.fill: maxScaleButton
                        onClicked: {
                            if (!maxScaleButton.isFullscreen) {
                                window.visibility = Window.FullScreen
                                maxScaleButton.isFullscreen = true
                            } else {
                                window.visibility = Window.Windowed
                                maxScaleButton.isFullscreen = false
                            }
                        }
                    }
                }

                // Кнопка «Закрыть»
                Item {
                    id: closeButton
                    width: 30
                    height: 30

                    Image {
                        source: "qrc:/close.svg"
                        anchors.centerIn: parent
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: window.close()
                    }
                }
            }
        }
    }

    // ===================================================================
    // СВОЙСТВА ДЛЯ ДАННЫХ (без изменений)
    // ===================================================================
    property string baseUrl: "http://localhost:8080"
    property var statusData: ({})
    property var statsData: ({})
    property var configData: ({})
    property var exposureLogs: ([])

    property string systemLogText: ""

    property string errorMessage: ""
    property bool showError: false
    Timer {
        id: hideErrorTimer
        interval: 3000; repeat: false
        onTriggered: window.showError = false
    }

    property string successMessage: ""
    property bool showSuccess: false
    Timer {
        id: hideSuccessTimer
        interval: 3000; repeat: false
        onTriggered: window.showSuccess = false
    }

    ColumnLayout {
        id: workAreaLimiter
        width: visibleWindow.width
        height: visibleWindow.height - title.height
        anchors.top: title.bottom

        // ===================================================================
        // TAB-BAR И СОДЕРЖИМОЕ ВКЛАДОК
        // ===================================================================
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            TabButton { text: "Панель управления" }
            TabButton { text: "Управление" }
            TabButton { text: "Настройки" }
            TabButton { text: "Журналы" }
        }
        StackLayout {
            id: stackLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            // *******************************************************************
            // 1. ПАНЕЛЬ УПРАВЛЕНИЯ
            // *******************************************************************
            ScrollView {
                ColumnLayout {
                    width: parent.width
                    spacing: 20

                    //-----------------------------------------------
                    // Карточка: состояние системы (Frame с elevation)
                    //-----------------------------------------------
                    Frame {
                        // Используем Frame как «плитку-карточку» с тенью и скруглением :contentReference[oaicite:5]{index=5}
                        width: parent.width - 40
                        Material.elevation: 4             // уровень тени :contentReference[oaicite:6]{index=6}
                        background: Rectangle {
                            color: Material.background      // фон из темы :contentReference[oaicite:7]{index=7}
                            radius: 8                        // скруглённые углы
                        }
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10
                            Text {
                                text: "Состояние системы"
                                font.pointSize: 16
                                font.bold: true
                                color: Material.foreground      // цвет текста из темы :contentReference[oaicite:8]{index=8}
                            }
                            ColumnLayout {
                                spacing: 8
                                Text { text: "Напряжение (кВ): " + (statusData.voltage_kv || "n/a") }
                                Text { text: "Ток (мА): " + (statusData.current_ma || "n/a") }
                                Text { text: "Экспозиция активна: " + (statusData.exposure_active || false) }
                                Text { text: "Нить накала включена: " + (statusData.filament_on || false) }
                                Text { text: "Состояние ошибки: " + (statusData.error_state || "") }
                                Text { text: "Последняя ошибка: " + (statusData.last_error || "Нет") }
                                Button {
                                    text: "Обновить статус"
                                    Material.elevation: 2           // тень на кнопке
                                    onClicked: fetchStatus()
                                }
                            }
                        }
                    }

                    //-----------------------------------------------
                    // Карточка: статистика (Frame с elevation)
                    //-----------------------------------------------
                    Frame {
                        width: parent.width - 40
                        Material.elevation: 4
                        background: Rectangle {
                            color: Material.background
                            radius: 8
                        }
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10
                            Text {
                                text: "Статистика"
                                font.pointSize: 16
                                font.bold: true
                                color: Material.foreground
                            }
                            ColumnLayout {
                                spacing: 8
                                Text { text: "Всего экспозиций: " + (statsData.total_exposures || "0") }
                                Text { text: "Время последней ошибки: " + (statsData.last_error || "Нет") }
                                Button {
                                    text: "Обновить статистику"
                                    Material.elevation: 2
                                    onClicked: fetchStats()
                                }
                            }
                        }
                    }

                    Component.onCompleted: {
                        fetchStatus()
                        fetchStats()
                    }
                }
            }

            // *******************************************************************
            // 2. УПРАВЛЕНИЕ
            // *******************************************************************
            ScrollView {
                ColumnLayout {
                    width: parent.width
                    spacing: 20

                    //-----------------------------------------------
                    // Карточка: Экспозиция
                    //-----------------------------------------------
                    Frame {
                        width: parent.width - 40
                        Material.elevation: 4
                        background: Rectangle {
                            color: Material.background
                            radius: 8
                        }
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10
                            Text {
                                text: "Экспозиция"
                                font.pointSize: 16
                                font.bold: true
                                color: Material.foreground
                            }
                            RowLayout {
                                spacing: 10
                                Text { text: "Длительность (мс):" }
                                TextField {
                                    id: durationField
                                    placeholderText: "1000"
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    Material.elevation: 1           // тень для TextField
                                    background: Rectangle {
                                        radius: 4
                                        color: Material.background
                                        border.color: Material.color(Material.Primary, Material.Mid)
                                    }
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Text { text: "Режим:" }
                                ComboBox {
                                    id: modeCombo
                                    model: ["тестовый", "стандартный", "импульсный"]
                                    currentIndex: 0
                                    Material.elevation: 1
                                    background: Rectangle {
                                        radius: 4
                                        color: Material.background
                                        border.color: Material.color(Material.Primary, Material.Mid)
                                    }
                                }
                            }
                            Button {
                                text: "Начать экспозицию"
                                Material.elevation: 2
                                onClicked: {
                                    var params = {
                                        "duration": parseInt(durationField.text),
                                        "mode": modeCombo.currentText
                                    }
                                    sendPost("/api/exposure/now", params)
                                }
                            }
                        }
                    }

                    //-----------------------------------------------
                    // Карточка: Напряжение
                    //-----------------------------------------------
                    Frame {
                        width: parent.width - 40
                        Material.elevation: 4
                        background: Rectangle {
                            color: Material.background
                            radius: 8
                        }
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10
                            Text {
                                text: "Напряжение"
                                font.pointSize: 16
                                font.bold: true
                                color: Material.foreground
                            }
                            RowLayout {
                                spacing: 10
                                Text { text: "Напряжение (кВ):" }
                                TextField {
                                    id: voltageField
                                    placeholderText: "50"
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    Material.elevation: 1
                                    background: Rectangle {
                                        radius: 4
                                        color: Material.background
                                        border.color: Material.color(Material.Primary, Material.Mid)
                                    }
                                }
                            }
                            Button {
                                text: "Установить напряжение"
                                Material.elevation: 2
                                onClicked: {
                                    var params = { "voltage": parseInt(voltageField.text) }
                                    sendPost("/api/voltage", params)
                                }
                            }
                        }
                    }

                    //-----------------------------------------------
                    // Карточка: Ток
                    //-----------------------------------------------
                    Frame {
                        width: parent.width - 40
                        Material.elevation: 4
                        background: Rectangle {
                            color: Material.background
                            radius: 8
                        }
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10
                            Text {
                                text: "Ток"
                                font.pointSize: 16
                                font.bold: true
                                color: Material.foreground
                            }
                            RowLayout {
                                spacing: 10
                                Text { text: "Ток (мА):" }
                                TextField {
                                    id: currentField
                                    placeholderText: "0.1"
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    Material.elevation: 1
                                    background: Rectangle {
                                        radius: 4
                                        color: Material.background
                                        border.color: Material.color(Material.Primary, Material.Mid)
                                    }
                                }
                            }
                            Button {
                                text: "Установить ток"
                                Material.elevation: 2
                                onClicked: {
                                    var params = { "current": parseFloat(currentField.text) }
                                    sendPost("/api/current", params)
                                }
                            }
                        }
                    }

                    //-----------------------------------------------
                    // Карточка: Аварийная остановка и драйвер
                    //-----------------------------------------------
                    Frame {
                        width: parent.width - 40
                        Material.elevation: 4
                        background: Rectangle {
                            color: Material.background
                            radius: 8
                        }
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10
                            Text {
                                text: "Аварийная остановка и драйвер"
                                font.pointSize: 16
                                font.bold: true
                                color: Material.foreground
                            }
                            RowLayout {
                                spacing: 10
                                Button {
                                    text: "Аварийная остановка"
                                    Material.elevation: 2
                                    onClicked: sendPost("/api/emergency_stop", {})
                                }
                                Button {
                                    text: "Перезапустить драйвер"
                                    Material.elevation: 2
                                    onClicked: sendPost("/api/driver/restart", {})
                                }
                            }
                        }
                    }

                    //-----------------------------------------------
                    // Карточка: Тест соединения
                    //-----------------------------------------------
                    Frame {
                        width: parent.width - 40
                        Material.elevation: 4
                        background: Rectangle {
                            color: Material.background
                            radius: 8
                        }
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10
                            Text {
                                text: "Тест соединения"
                                font.pointSize: 16
                                font.bold: true
                                color: Material.foreground
                            }
                            RowLayout {
                                spacing: 10
                                Button {
                                    text: "Проверить соединение"
                                    Material.elevation: 2
                                    onClicked: fetchConnectionTest()
                                }
                                Text {
                                    id: connectionResult
                                    text: ""
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                    }
                }
            }

            // *******************************************************************
            // 3. НАСТРОЙКИ
            // *******************************************************************
            ScrollView {
                ColumnLayout {
                    width: parent.width
                    spacing: 20

                    //-----------------------------------------------
                    // Карточка: Настройки рентгена
                    //-----------------------------------------------
                    Frame {
                        width: parent.width - 40
                        Material.elevation: 4
                        background: Rectangle {
                            color: Material.background
                            radius: 8
                        }
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10
                            Text {
                                text: "Настройки рентгена"
                                font.pointSize: 16
                                font.bold: true
                                color: Material.foreground
                            }
                            Text { text: "COM-порт:" }
                            TextField {
                                id: comPortField
                                placeholderText: "напр. COM1 или /dev/ttyUSB0"
                                Material.elevation: 1
                                background: Rectangle {
                                    radius: 4
                                    color: Material.background
                                    border.color: Material.color(Material.Primary, Material.Mid)
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Button {
                                    text: "Загрузить настройки"
                                    Material.elevation: 2
                                    onClicked: fetchConfig()
                                }
                                Button {
                                    text: "Сохранить настройки"
                                    Material.elevation: 2
                                    onClicked: {
                                        var obj = {}
                                        obj["com_port"] = comPortField.text
                                        sendPost("/api/config", obj)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // *******************************************************************
            // 4. ЖУРНАЛЫ
            // *******************************************************************
            ScrollView {
                width: parent.width
                ColumnLayout {
                    spacing: 20
                    Layout.fillWidth: true
                    Layout.margins: 20

                    //---------------------------------------------------------------
                    // Журнал экспозиций: «плитки»-строки с elevation
                    //---------------------------------------------------------------
                    Frame {
                        //title: "Журнал экспозиций"  // хотя GroupBox уже был, здесь демонстрируем Frame как карточку
                        width: parent.width
                        Material.elevation: 4
                        background: Rectangle {
                            color: Material.background
                            radius: 8
                        }
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10
                            RowLayout {
                                spacing: 10
                                Button {
                                    text: "Загрузить журнал экспозиций"
                                    Material.elevation: 2
                                    onClicked: fetchExposureLogs()
                                }
                            }
                            ListView {
                                id: exposureListView
                                Layout.fillWidth: true
                                height: 200
                                model: exposureLogs
                                delegate: Frame {
                                    // Каждая запись — «плитка»-строка с elevation :contentReference[oaicite:9]{index=9}
                                    width: parent.width
                                    height: 40
                                    Material.elevation: 1
                                    background: Rectangle {
                                        color: Material.color(Material.Background, Material.Light)
                                        radius: 4
                                    }
                                    RowLayout {
                                        anchors.fill: parent
                                        Text {
                                            text: modelData.timestamp + ": " +
                                                  modelData.voltage + "кВ, " +
                                                  modelData.current + "мА, " +
                                                  modelData.duration + "мс, " +
                                                  modelData.mode
                                            font.pixelSize: 14
                                            color: Material.foreground
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // ===================================================================
                    // Журнал системы (исправленный блок)
                    // ===================================================================
                    Frame {
                        width: parent.width - 40
                        Material.elevation: 4
                        background: Rectangle {
                            color: Material.background
                            radius: 8
                        }
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10
                            Text {
                                text: "Журнал системы"
                                font.pointSize: 16
                                font.bold: true
                                color: Material.foreground
                            }

                            // Кнопка для загрузки лог-файла
                            RowLayout {
                                spacing: 10
                                Button {
                                    text: "Загрузить текущий лог-файл драйвера"
                                    Material.elevation: 2
                                    onClicked: fetchSystemLogFile()
                                }
                            }

                            // ИСПРАВЛЕННАЯ СЕКЦИЯ: ScrollView и TextArea
                            ScrollView {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                clip: true

                                TextArea {
                                    id: systemLogArea
                                    text: window.systemLogText
                                    readOnly: true
                                    wrapMode: TextArea.NoWrap
                                    font.family: "monospace"
                                    font.pixelSize: 13

                                    // Автоматически прокручивать вниз при обновлении
                                    onTextChanged: {
                                        cursorPosition = text.length;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ===================================================================
    // ВСПЛЫВАЮЩИЕ УВЕДОМЛЕНИЯ (без изменений)
    // ===================================================================
    Rectangle {
        id: successPopup
        height: window.showSuccess ? popupHeight : 0
        width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        color: window.showSuccess ? "#4CAF50" : "transparent"
        radius: 8
        opacity: showSuccess ? 0.95 : 0.0
        Behavior on height {
            NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
        }
        Behavior on color {
            ColorAnimation { duration: 250; easing.type: Easing.InOutQuad }
        }
        Behavior on opacity {
            NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
        }
        readonly property int popupHeight: 50
        readonly property int paddingHorizontal: 10
        readonly property int paddingVertical: 5
        ScrollView {
            id: successScrollArea
            anchors {
                left: parent.left; right: parent.right
                top: parent.top; bottom: parent.bottom
                leftMargin: successPopup.paddingHorizontal
                rightMargin: successPopup.paddingHorizontal
                topMargin: successPopup.paddingVertical
                bottomMargin: successPopup.paddingVertical
            }
            clip: true
            Text {
                id: successText
                text: window.successMessage
                color: "white"
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                width: parent.width
            }
        }
    }

    Rectangle {
        id: errorPopup
        height: window.showError ? popupHeight : 0
        width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        color: window.showError ? "#D32F2F" : "transparent"
        radius: 8
        opacity: showError ? 0.95 : 0.0
        Behavior on height {
            NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
        }
        Behavior on color {
            ColorAnimation { duration: 250; easing.type: Easing.InOutQuad }
        }
        Behavior on opacity {
            NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
        }
        readonly property int popupHeight: 50
        readonly property int paddingHorizontal: 10
        readonly property int paddingVertical: 5
        ScrollView {
            id: scrollArea
            anchors {
                left: parent.left; right: parent.right
                top: parent.top; bottom: parent.bottom
                leftMargin: errorPopup.paddingHorizontal
                rightMargin: errorPopup.paddingHorizontal
                topMargin: errorPopup.paddingVertical
                bottomMargin: errorPopup.paddingVertical
            }
            clip: true
            Text {
                id: errorText
                text: window.errorMessage
                color: "white"
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                width: parent.width
            }
        }
    }

    // ===================================================================
    // СЕТЕВЫЕ ФУНКЦИИ (без изменений)
    // ===================================================================
    function sendPost(path, data) {
        var xhr = new XMLHttpRequest()
        xhr.open("POST", baseUrl + path)
        xhr.setRequestHeader("Content-Type", "application/json")
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    try {
                        var resp = JSON.parse(xhr.responseText)
                        if (resp.status && resp.status === "success") {
                            window.successMessage = "Операция выполнена успешно!"
                            window.showSuccess = true
                            hideSuccessTimer.restart()
                        }
                    } catch (e) {
                        window.successMessage = "Операция выполнена успешно!"
                        window.showSuccess = true
                        hideSuccessTimer.restart()
                    }
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText)
                        window.errorMessage = respObj.error || "Неизвестная ошибка"
                    } catch (e) {
                        window.errorMessage = "Ошибка ответа от сервера"
                    }
                    window.showError = true
                    hideErrorTimer.restart()
                }
            }
        }
        xhr.send(JSON.stringify(data))
    }

    function fetchConnectionTest() {
        var xhr = new XMLHttpRequest()
        xhr.open("GET", baseUrl + "/api/connection/test")
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    try {
                        var resp = JSON.parse(xhr.responseText)
                        if (resp.status === "OK") {
                            connectionResult.text = "OK, " + resp.response_time + " мс"
                            window.successMessage = "Соединение установлено!"
                            window.showSuccess = true
                            hideSuccessTimer.restart()
                        } else {
                            window.errorMessage = "Ошибка соединения: ответ = ERROR, время = " + resp.response_time + " мс"
                            window.showError = true
                            hideErrorTimer.restart()
                            connectionResult.text = "ERROR"
                        }
                    } catch (e) {
                        window.errorMessage = "Неправильный формат ответа от /api/connection/test"
                        window.showError = true
                        hideErrorTimer.restart()
                        connectionResult.text = "Ошибка"
                    }
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText)
                        if (respObj.error) {
                            window.errorMessage = respObj.error
                        } else {
                            window.errorMessage = "Неизвестная ошибка со стороны драйвера: " + xhr.responseText
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка проверки соединения: HTTP " + xhr.status
                    }
                    window.showError = true
                    hideErrorTimer.restart()
                    connectionResult.text = "Ошибка"
                }
            }
        }
        xhr.send()
    }

    function fetchStatus() {
        var xhr = new XMLHttpRequest()
        xhr.open("GET", baseUrl + "/api/status")
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    statusData = JSON.parse(xhr.responseText)
                    window.successMessage = "Статус обновлен"
                    window.showSuccess = true
                    hideSuccessTimer.restart()
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText)
                        if (respObj.error) {
                            window.errorMessage = respObj.error
                        } else {
                            window.errorMessage = "Неизвестная ошибка: " + xhr.responseText
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка загрузки статуса: код " + xhr.status
                    }
                    window.showError = true
                    hideErrorTimer.restart()
                }
            }
        }
        xhr.send()
    }

    function fetchStats() {
        var xhr = new XMLHttpRequest()
        xhr.open("GET", baseUrl + "/api/stats")
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    statsData = JSON.parse(xhr.responseText)
                    window.successMessage = "Статистика обновлена"
                    window.showSuccess = true
                    hideSuccessTimer.restart()
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText)
                        if (respObj.error) {
                            window.errorMessage = respObj.error
                        } else {
                            window.errorMessage = "Неизвестная ошибка: " + xhr.responseText
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка загрузки статистики: код " + xhr.status
                    }
                    window.showError = true
                    hideErrorTimer.restart()
                }
            }
        }
        xhr.send()
    }

    function fetchConfig() {
        var xhr = new XMLHttpRequest()
        xhr.open("GET", baseUrl + "/api/config")
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    configData = JSON.parse(xhr.responseText)
                    comPortField.text = configData.com_port || ""
                    window.successMessage = "Настройки загружены"
                    window.showSuccess = true
                    hideSuccessTimer.restart()
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText)
                        if (respObj.error) {
                            window.errorMessage = respObj.error
                        } else {
                            window.errorMessage = "Неизвестная ошибка: " + xhr.responseText
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка загрузки настроек: код " + xhr.status
                    }
                    window.showError = true
                    hideErrorTimer.restart()
                }
            }
        }
        xhr.send()
    }

    function fetchExposureLogs() {
        var xhr = new XMLHttpRequest()
        xhr.open("GET", baseUrl + "/api/logs/exposure")
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    exposureLogs = JSON.parse(xhr.responseText)
                    window.successMessage = "Журнал экспозиций загружен"
                    window.showSuccess = true
                    hideSuccessTimer.restart()
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText)
                        if (respObj.error) {
                            window.errorMessage = respObj.error
                        } else {
                            window.errorMessage = "Неизвестная ошибка: " + xhr.responseText
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка загрузки журнала экспозиций: код " + xhr.status
                    }
                    window.showError = true
                    hideErrorTimer.restart()
                }
            }
        }
        xhr.send()
    }

    function fetchSystemLogFile() {
        var xhr = new XMLHttpRequest()
        xhr.open("GET", baseUrl + "/api/logs/system")
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    window.systemLogText = xhr.responseText
                    console.log("Получен лог:", xhr.responseText);
                    window.successMessage = "Журнал системы загружен"
                    window.showSuccess = true
                    hideSuccessTimer.restart()
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText)
                        if (respObj.error) {
                            window.errorMessage = respObj.error
                        } else {
                            window.errorMessage = "Неизвестная ошибка: " + xhr.responseText
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка загрузки лог-файла: код " + xhr.status
                    }
                    window.showError = true
                    hideErrorTimer.restart()
                }
            }
        }
        xhr.send()
    }
}
