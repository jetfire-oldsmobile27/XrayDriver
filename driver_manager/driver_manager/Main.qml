// Файл: main.qml

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

Window {
    id: window
    width: 1280
    height: 720
    minimumWidth: 720
    minimumHeight: 800
    visible: true
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowMinMaxButtonsHint

    property string buttonsColor: "#BB86FC"
    property string backgroundColor: "white"
    property string panelColor: "#1F1B24"
    property string borderColor: "#292929"
    property string textColor: "#E0E0E0"
    property string hoverColor: "#3700B3"

    Material.theme: Material.Light
    Material.accent: Material.Green


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

    // Верхняя кастомная панель
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

                // Кнопка "Свернуть"
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

                // Кнопка "Максимизировать/Свернуть"
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
                                window.visibility = Window.FullScreen;
                                maxScaleButton.isFullscreen = true;
                            } else {
                                window.visibility = Window.Windowed;
                                maxScaleButton.isFullscreen = false;
                            }
                        }
                    }
                }

                // Кнопка "Закрыть"
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


    // Базовый URL сервера
    property string baseUrl: "http://localhost:8080"

    // Хранение данных
    property var statusData: ({})
    property var statsData: ({})
    property var configData: ({})
    property var exposureLogs: ([])

    // Теперь просто одна строка с текстом всего лога
    property string systemLogText: ""

    // ------------------------------------------------------------------
    //  Новые свойства для показа ошибки
    // ------------------------------------------------------------------
    property string errorMessage: ""
    property bool showError: false

    // ------------------------------------------------------------------
    //  Таймер авто-скрытия уведомления об ошибке
    // ------------------------------------------------------------------
    Timer {
        id: hideErrorTimer
        interval: 3000    // 3 секунды
        repeat: false
        onTriggered: {
            // Когда таймер сработал, плавно скрываем прямоугольник
            window.showError = false;
        }
    }

    // ------------------------------------------------------------------
    //  Новые свойства для показа уведомлений об успехе
    // ------------------------------------------------------------------
    property string successMessage: ""
    property bool showSuccess: false

    // ------------------------------------------------------------------
    //  Таймер авто-скрытия уведомления об успехе
    // ------------------------------------------------------------------
    Timer {
        id: hideSuccessTimer
        interval: 3000    // 3 секунды
        repeat: false
        onTriggered: {
            window.showSuccess = false;
        }
    }

    ColumnLayout {
        id: workAreaLimiter
        width: visibleWindow.width
        height: visibleWindow.height - title.height
        anchors.top: title.bottom
        //anchors.fill: parent

        // Вкладки
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            TabButton { text: "Панель управления" }
            TabButton { text: "Управление" }
            TabButton { text: "Настройки" }
            TabButton { text: "Журналы" }
        }

        // Содержимое вкладок
        StackLayout {
            id: stackLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            /***********************************************************************
             * 1. Панель управления (Состояние системы и статистика)
             ***********************************************************************/
            ScrollView {
                ColumnLayout {
                    width: parent.width
                    spacing: 20

                    GroupBox {
                        title: "Состояние системы"
                        width: parent.width - 40
                        ColumnLayout {
                            spacing: 10
                            Text { text: "Напряжение (кВ): " + (statusData.voltage_kv || "n/a") }
                            Text { text: "Ток (мА): " + (statusData.current_ma || "n/a") }
                            Text { text: "Экспозиция активна: " + (statusData.exposure_active || false) }
                            Text { text: "Нить накала включена: " + (statusData.filament_on || false) }
                            Text { text: "Состояние ошибки: " + (statusData.error_state || "") }
                            Text { text: "Последняя ошибка: " + (statusData.last_error || "Нет") }
                            Button {
                                text: "Обновить статус"
                                onClicked: fetchStatus()
                            }
                        }
                    }

                    GroupBox {
                        title: "Статистика"
                        width: parent.width - 40
                        ColumnLayout {
                            spacing: 10
                            Text { text: "Всего экспозиций: " + (statsData.total_exposures || "0") }
                            Text { text: "Время последней ошибки: " + (statsData.last_error || "Нет") }
                            Button {
                                text: "Обновить статистику"
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

            /***********************************************************************
             * 2. Управление (Экспозиция, Напряжение, Ток, Авария, Драйвер)
             ***********************************************************************/
            ScrollView {
                ColumnLayout {
                    width: parent.width
                    spacing: 20

                    GroupBox {
                        title: "Экспозиция"
                        width: parent.width - 40
                        ColumnLayout {
                            spacing: 10
                            RowLayout {
                                spacing: 10
                                Text { text: "Длительность (мс):" }
                                TextField {
                                    id: durationField
                                    placeholderText: "1000"
                                    inputMethodHints: Qt.ImhDigitsOnly
                                }
                            }
                            RowLayout {
                                spacing: 10
                                Text { text: "Режим:" }
                                ComboBox {
                                    id: modeCombo
                                    model: ["тестовый", "стандартный", "импульсный"]
                                    currentIndex: 0
                                }
                            }
                            Button {
                                text: "Начать экспозицию"
                                onClicked: {
                                    var params = {
                                        "duration": parseInt(durationField.text),
                                        "mode": modeCombo.currentText
                                    };
                                    sendPost("/api/exposure/now", params);
                                }
                            }
                        }
                    }

                    GroupBox {
                        title: "Напряжение"
                        width: parent.width - 40
                        ColumnLayout {
                            spacing: 10
                            RowLayout {
                                spacing: 10
                                Text { text: "Напряжение (кВ):" }
                                TextField {
                                    id: voltageField
                                    placeholderText: "50"
                                    inputMethodHints: Qt.ImhDigitsOnly
                                }
                            }
                            Button {
                                text: "Установить напряжение"
                                onClicked: {
                                    var params = { "voltage": parseInt(voltageField.text) };
                                    sendPost("/api/voltage", params);
                                }
                            }
                        }
                    }

                    GroupBox {
                        title: "Ток"
                        width: parent.width - 40
                        ColumnLayout {
                            spacing: 10
                            RowLayout {
                                spacing: 10
                                Text { text: "Ток (мА):" }
                                TextField {
                                    id: currentField
                                    placeholderText: "0.1"
                                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                                }
                            }
                            Button {
                                text: "Установить ток"
                                onClicked: {
                                    var params = { "current": parseFloat(currentField.text) };
                                    sendPost("/api/current", params);
                                }
                            }
                        }
                    }

                    GroupBox {
                        title: "Аварийная остановка и драйвер"
                        width: parent.width - 40
                        ColumnLayout {
                            spacing: 10
                            Button {
                                text: "Аварийная остановка"
                                onClicked: sendPost("/api/emergency_stop", {})
                            }
                            Button {
                                text: "Перезапустить драйвер"
                                onClicked: sendPost("/api/driver/restart", {})
                            }
                        }
                    }

                    GroupBox {
                        title: "Тест соединения"
                        width: parent.width - 40
                        ColumnLayout {
                            spacing: 10
                            RowLayout {
                                spacing: 10
                                Button {
                                    text: "Проверить соединение"
                                    onClicked: fetchConnectionTest()
                                }
                                Text { id: connectionResult; text: "" }
                            }
                        }
                    }
                }
            }

            /***********************************************************************
             * 3. Настройки (Просмотр и сохранение настроек: COM-порт)
             ***********************************************************************/
            ScrollView {
                ColumnLayout {
                    width: parent.width
                    spacing: 20

                    GroupBox {
                        title: "Настройки рентгена"
                        width: parent.width - 40
                        ColumnLayout {
                            spacing: 10
                            Text { text: "COM-порт:" }
                            TextField { id: comPortField; placeholderText: "напр. COM1 или /dev/ttyUSB0" }
                            Button {
                                text: "Загрузить настройки"
                                onClicked: fetchConfig()
                            }
                            Button {
                                text: "Сохранить настройки"
                                onClicked: {
                                    var obj = {};
                                    obj["com_port"] = comPortField.text;
                                    sendPost("/api/config", obj);
                                }
                            }
                        }
                    }
                }
            }

            /***********************************************************************
             * 4. Журналы (Журнал экспозиций + Журнал системы)
             ***********************************************************************/
            ScrollView {
                width: parent.width
                ColumnLayout {
                    spacing: 20
                    Layout.fillWidth: true
                    Layout.margins: 20

                    /****************************************************************
                     * 4.1 Журнал экспозиций
                     ****************************************************************/
                    GroupBox {
                        title: "Журнал экспозиций"
                        Layout.fillWidth: true
                        Layout.margins: 0
                        ColumnLayout {
                            spacing: 10
                            Button {
                                text: "Загрузить журнал экспозиций"
                                onClicked: fetchExposureLogs()
                            }
                            ListView {
                                id: exposureListView
                                Layout.fillWidth: true
                                height: 200
                                model: exposureLogs
                                delegate: Rectangle {
                                    width: parent.width
                                    height: 30
                                    Text {
                                        text: modelData.timestamp + ": " +
                                              modelData.voltage + "кВ, " +
                                              modelData.current + "мА, " +
                                              modelData.duration + "мс, " +
                                              modelData.mode
                                        font.pixelSize: 14
                                    }
                                }
                            }
                        }
                    }

                    /****************************************************************
                     * 4.2 Журнал системы
                     ****************************************************************/
                    GroupBox {
                        title: "Журнал системы"
                        Layout.fillWidth: true
                        Layout.margins: 0

                        ColumnLayout {
                            spacing: 10
                            Button {
                                text: "Загрузить текущий лог-файл драйвера"
                                onClicked: fetchSystemLogFile()
                            }
                            ScrollView {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Layout.preferredHeight: 300

                                TextArea {
                                    id: systemLogArea
                                    text: systemLogText
                                    readOnly: true
                                    wrapMode: TextArea.NoWrap
                                    font.family: "monospace"
                                    font.pixelSize: 13
                                }
                            }
                        }
                    }
                }
            }

        }
    }

    // ------------------------------------------------------------------
    //  Плавающее уведомление об успешной операции (вверху)
    // ------------------------------------------------------------------
    Rectangle {
        id: successPopup
        // Когда showSuccess == true, height плавно становится popupHeight, иначе — 0.
        height: window.showSuccess ? popupHeight : 0
        width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top

        // Зеленый цвет для успешных операций
        color: window.showSuccess ? "#4CAF50" : "transparent"
        radius: 8
        opacity: showSuccess ? 0.95 : 0.0

        // Параметры анимации
        Behavior on height {
            NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
        }
        Behavior on color {
            ColorAnimation { duration: 250; easing.type: Easing.InOutQuad }
        }
        Behavior on opacity {
            NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
        }

        // Фиксированная «целевая» высота
        readonly property int popupHeight: 50

        // Внутренние отступы
        readonly property int paddingHorizontal: 10
        readonly property int paddingVertical: 5

        // Оборачиваем текст в ScrollView
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

    // ------------------------------------------------------------------
    //  Плавающее уведомление об ошибке (внизу, «скрыто» изначально)
    // ------------------------------------------------------------------
    Rectangle {
        id: errorPopup
        // Когда showError == true, height плавно становится popupHeight, иначе — 0.
        height: window.showError ? popupHeight : 0
        width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom

        // Цвет «переливается» из полностью прозрачного в насыщенный красный
        color: window.showError ? "#D32F2F" : "transparent"
        radius: 8
        opacity: showError ? 0.95 : 0.0

        // Параметры анимации: поведение при смене height и color
        Behavior on height {
            NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
        }
        Behavior on color {
            ColorAnimation { duration: 250; easing.type: Easing.InOutQuad }
        }
        Behavior on opacity {
            NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
        }

        // Фиксированная «целевая» высота (когда уведомление полностью показано)
        readonly property int popupHeight: 50

        // Внутренний отступ, чтобы текст не упирался в края
        readonly property int paddingHorizontal: 10
        readonly property int paddingVertical: 5

        // Оборачиваем текст в ScrollView, чтобы длинные сообщения можно было прокручивать
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

    /***************************************************************************
     *  Общие JS-функции для сетевых запросов (МОДИФИЦИРОВАННЫЕ)
     ***************************************************************************/

    // Универсальная отправка POST-запроса с обработкой ошибок и успехов
    function sendPost(path, data) {
        var xhr = new XMLHttpRequest();
        xhr.open("POST", baseUrl + path);
        xhr.setRequestHeader("Content-Type", "application/json");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    // Показываем уведомление об успехе
                    try {
                        var resp = JSON.parse(xhr.responseText);
                        if (resp.status && resp.status === "success") {
                            window.successMessage = "Операция выполнена успешно!";
                            window.showSuccess = true;
                            hideSuccessTimer.restart();
                        }
                    } catch (e) {
                        window.successMessage = "Операция выполнена успешно!";
                        window.showSuccess = true;
                        hideSuccessTimer.restart();
                    }
                } else {
                    // Обработка ошибок (как раньше)
                    try {
                        var respObj = JSON.parse(xhr.responseText);
                        window.errorMessage = respObj.error || "Неизвестная ошибка";
                    } catch (e) {
                        window.errorMessage = "Ошибка ответа от сервера";
                    }
                    window.showError = true;
                    hideErrorTimer.restart();
                }
            }
        }
        xhr.send(JSON.stringify(data));
    }

    // GET /api/connection/test с обработкой успехов и ошибок
    function fetchConnectionTest() {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", baseUrl + "/api/connection/test");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    try {
                        var resp = JSON.parse(xhr.responseText);
                        if (resp.status === "OK") {
                            connectionResult.text = "OK, " + resp.response_time + " мс";
                            // Показываем уведомление об успехе
                            window.successMessage = "Соединение установлено!";
                            window.showSuccess = true;
                            hideSuccessTimer.restart();
                        } else {
                            window.errorMessage = "Ошибка соединения: ответ сервера = ERROR, время = " + resp.response_time + " мс";
                            window.showError = true;
                            hideErrorTimer.restart();
                            connectionResult.text = "ERROR";
                        }
                    } catch (e) {
                        window.errorMessage = "Неправильный формат ответа от /api/connection/test";
                        window.showError = true;
                        hideErrorTimer.restart();
                        connectionResult.text = "Ошибка";
                    }
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText);
                        if (respObj.error) {
                            window.errorMessage = respObj.error;
                        } else {
                            window.errorMessage = "Неизвестная ошибка со стороны драйвера: " + xhr.responseText;
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка проверки соединения со стороны драйвера: HTTP " + xhr.status;
                    }
                    window.showError = true;
                    hideErrorTimer.restart();
                    connectionResult.text = "Ошибка";
                }
            }
        }
        xhr.send();
    }

    // GET /api/status с обработкой успехов и ошибок
    function fetchStatus() {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", baseUrl + "/api/status");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    statusData = JSON.parse(xhr.responseText);
                    // Показываем уведомление об успехе
                    window.successMessage = "Статус обновлен";
                    window.showSuccess = true;
                    hideSuccessTimer.restart();
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText);
                        if (respObj.error) {
                            window.errorMessage = respObj.error;
                        } else {
                            window.errorMessage = "Неизвестная ошибка: " + xhr.responseText;
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка загрузки статуса: код " + xhr.status;
                    }
                    window.showError = true;
                    hideErrorTimer.restart();
                }
            }
        }
        xhr.send();
    }

    // GET /api/stats с обработкой ошибок (успешное обновление не требует уведомления)
    function fetchStats() {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", baseUrl + "/api/stats");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    statsData = JSON.parse(xhr.responseText);
                    window.successMessage = "Статистика обновлена";
                    window.showSuccess = true;
                    hideSuccessTimer.restart();
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText);
                        if (respObj.error) {
                            window.errorMessage = respObj.error;
                        } else {
                            window.errorMessage = "Неизвестная ошибка: " + xhr.responseText;
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка загрузки статистики: код " + xhr.status;
                    }
                    window.showError = true;
                    hideErrorTimer.restart();
                }
            }
        }
        xhr.send();
    }

    // GET /api/config с обработкой успехов и ошибок
    function fetchConfig() {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", baseUrl + "/api/config");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    configData = JSON.parse(xhr.responseText);
                    comPortField.text = configData.com_port || "";
                    window.successMessage = "Настройки загружены";
                    window.showSuccess = true;
                    hideSuccessTimer.restart();
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText);
                        if (respObj.error) {
                            window.errorMessage = respObj.error;
                        } else {
                            window.errorMessage = "Неизвестная ошибка: " + xhr.responseText;
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка загрузки настроек: код " + xhr.status;
                    }
                    window.showError = true;
                    hideErrorTimer.restart();
                }
            }
        }
        xhr.send();
    }

    // GET /api/logs/exposure с обработкой успехов и ошибок
    function fetchExposureLogs() {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", baseUrl + "/api/logs/exposure");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    exposureLogs = JSON.parse(xhr.responseText);
                    window.successMessage = "Журнал экспозиций загружен";
                    window.showSuccess = true;
                    hideSuccessTimer.restart();
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText);
                        if (respObj.error) {
                            window.errorMessage = respObj.error;
                        } else {
                            window.errorMessage = "Неизвестная ошибка: " + xhr.responseText;
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка загрузки журнала экспозиций: код " + xhr.status;
                    }
                    window.showError = true;
                    hideErrorTimer.restart();
                }
            }
        }
        xhr.send();
    }

    // GET /api/logs/system с обработкой успехов и ошибок
    function fetchSystemLogFile() {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", baseUrl + "/api/logs/system");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    systemLogText = xhr.responseText;
                    window.successMessage = "Журнал системы загружен";
                    window.showSuccess = true;
                    hideSuccessTimer.restart();
                } else {
                    try {
                        var respObj = JSON.parse(xhr.responseText);
                        if (respObj.error) {
                            window.errorMessage = respObj.error;
                        } else {
                            window.errorMessage = "Неизвестная ошибка: " + xhr.responseText;
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка загрузки лог-файла: код " + xhr.status;
                    }
                    window.showError = true;
                    hideErrorTimer.restart();
                }
            }
        }
        xhr.send();
    }

}
