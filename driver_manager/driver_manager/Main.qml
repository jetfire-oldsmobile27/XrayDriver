// Файл: main.qml

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 600
    title: "Менеджер драйвера динамической трубки"

    // Базовый URL сервера
    property string baseUrl: "http://localhost:8080"

    // Хранение данных
    property var statusData: ({})
    property var statsData: ({})
    property var configData: ({})
    property var exposureLogs: []
    // Теперь просто одна строка с текстом всего лога
    property string systemLogText: ""

    // ------------------------------------------------------------------
        //  Новые свойства для показа ошибки
        // ------------------------------------------------------------------
        property string errorMessage: ""
        property bool showError: false

        // ------------------------------------------------------------------
        //  Таймер авто-скрытия уведомления
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






    ColumnLayout {
        anchors.fill: parent

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
                    //padding: 20

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
                    //padding: 20

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
                    Layout.fillWidth: true       // Колонка сама по себе тянется на всю ширину ScrollView
                    Layout.margins: 20           // Общие отступы для ColumnLayout (можно убрать, если каждый GroupBox сам задаёт margin)

                    /****************************************************************
                     * 4.1 Журнал экспозиций
                     ****************************************************************/
                    GroupBox {
                        title: "Журнал экспозиций"
                        Layout.fillWidth: true   // растягиваем GroupBox на всю ширину ColumnLayout (-40 в оригинале заменяется на гибкий отступ)
                        Layout.margins: 0        // можно задать отступы внутри GroupBox, если нужно
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
                                    width: parent.width; height: 30
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
                        Layout.fillWidth: true   // говорит ColumnLayout: «растяни меня на всю ширину»
                        Layout.margins: 0        // здесь можно указывать внутренние отступы, например 0, а внешние задать у ColumnLayout

                        ColumnLayout {
                            spacing: 10
                            // === Кнопка загрузки лога ===
                            Button {
                                text: "Загрузить текущий лог-файл драйвера"
                                onClicked: fetchSystemLogFile()
                            }

                            // === Обёртка ScrollView для TextArea ===
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

                                    // Без anchors.fill — ScrollView сам управляет размерами
                                }
                            }
                        }
                    }
                }
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
        opacity: showError ? 0.95 : 0.0   // чуть просвечивает, когда открыт

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
                leftMargin: paddingHorizontal
                rightMargin: paddingHorizontal
                topMargin: paddingVertical
                bottomMargin: paddingVertical
            }
            clip: true

            Text {
                id: errorText
                text: window.errorMessage
                color: "white"
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                width: parent.width  // чтобы текст переносился по ширине ScrollView
            }
        }
    }

    /***************************************************************************
     *  Общие JS-функции для сетевых запросов
     ***************************************************************************/

    // Универсальная отправка POST-запроса (с обработкой ошибок)
    function sendPost(path, data) {
        var xhr = new XMLHttpRequest();
        xhr.open("POST", baseUrl + path);
        xhr.setRequestHeader("Content-Type", "application/json");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    // При желании можно показать уведомление об успехе
                } else {
                    // Пытаемся распарсить JSON-ошибку: { "error": "текст ошибки" }
                    try {
                        var respObj = JSON.parse(xhr.responseText);
                        if (respObj.error) {
                            window.errorMessage = respObj.error;
                        } else {
                            window.errorMessage = "Неизвестная ошибка: " + xhr.responseText;
                        }
                    } catch (e) {
                        window.errorMessage = "Ошибка ответа от сервера: " + xhr.responseText;
                    }
                    window.showError = true;
                    hideErrorTimer.restart();
                    console.log("Ошибка: " + xhr.responseText);
                }
            }
        }
        xhr.send(JSON.stringify(data));
    }

    // GET /api/connection/test (с обработкой ошибок)
    function fetchConnectionTest() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", baseUrl + "/api/connection/test");
            xhr.onreadystatechange = function() {
                if (xhr.readyState === XMLHttpRequest.DONE) {
                    if (xhr.status === 200) {
                        // Сервер вернул HTTP 200, разбираем JSON вида { "status": "OK" или "ERROR", "response_time": <число> }
                        try {
                            var resp = JSON.parse(xhr.responseText);
                            if (resp.status === "OK") {
                                connectionResult.text = "OK, " + resp.response_time + " мс";
                            } else {
                                // Сервер вернул status = "ERROR"
                                window.errorMessage = "Ошибка соединения: ответ сервера = ERROR, время = " + resp.response_time + " мс";
                                window.showError = true;
                                hideErrorTimer.restart();
                                connectionResult.text = "ERROR";
                            }
                        } catch (e) {
                            // Не удалось распарсить JSON
                            window.errorMessage = "Неправильный формат ответа от /api/connection/test";
                            window.showError = true;
                            hideErrorTimer.restart();
                            connectionResult.text = "Ошибка";
                        }
                    } else {
                        // HTTP-уровневая ошибка (статус ≠ 200)
                        try {
                            var respObj = JSON.parse(xhr.responseText);
                            if (respObj.error) {
                                window.errorMessage = respObj.error;
                            } else {
                                window.errorMessage = "Неизвестная ошибка со стороны драйвера: " + xhr.responseText;
                            }
                        } catch (e) {
                            window.errorMessage = "Ошибка проверки соединения со стороны микроконтроллера: HTTP " + xhr.status;
                        }
                        window.showError = true;
                        hideErrorTimer.restart();
                        connectionResult.text = "Ошибка";
                    }
                }
            }
            xhr.send();
        }

    // GET /api/status (с обработкой ошибок)
    function fetchStatus() {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", baseUrl + "/api/status");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    statusData = JSON.parse(xhr.responseText);
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

    // GET /api/stats (с обработкой ошибок)
    function fetchStats() {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", baseUrl + "/api/stats");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    statsData = JSON.parse(xhr.responseText);
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

    // GET /api/config (с обработкой ошибок)
    function fetchConfig() {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", baseUrl + "/api/config");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    configData = JSON.parse(xhr.responseText);
                    comPortField.text = configData.com_port || "";
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

    // GET /api/logs/exposure (с обработкой ошибок)
    function fetchExposureLogs() {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", baseUrl + "/api/logs/exposure");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    exposureLogs = JSON.parse(xhr.responseText);
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

    // GET /api/logs/system (с обработкой ошибок)
    function fetchSystemLogFile() {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", baseUrl + "/api/logs/system");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    systemLogText = xhr.responseText;
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
