import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "首页"
    property string familyId: ""
    property string today: ""
    property int dinerCount: 2
    property int breakfastCount: 3
    property int lunchCount: 4
    property int dinnerCount: 4
    property string dietMode: "maintenance"
    background: Rectangle { color: Theme.bgPage }

    Component.onCompleted: {
        var d = new Date()
        today = d.getFullYear() + "-" + String(d.getMonth()+1).padStart(2,'0') + "-" + String(d.getDate()).padStart(2,'0')
        var fams = familyService.getUserFamilies(session.userId)
        if (fams.length > 0) familyId = fams[0].familyId
        loadMenu()
    }

    function loadMenu() {
        if (!familyId) return
        var menu = menuService.getTodayMenu(familyId, today)
        bModel.clear(); lModel.clear(); dModel.clear()
        fill(bModel, menu.breakfast); fill(lModel, menu.lunch); fill(dModel, menu.dinner)
    }
    function fill(model, slot) {
        if (!slot || !slot.items) return
        for (var i = 0; i < slot.items.length; i++) model.append(slot.items[i])
        calcNutrition()
    }
    function getMid(mt) { var m = menuService.getTodayMenu(familyId, today); var s = m[mt]; return s ? s.menuId : "" }
    function gen(mt, cnt)  { menuService.generateMenu(familyId, today, mt, cnt, dietMode, dinerCount); loadMenu() }
    function swap(mt, rid) { menuService.swapDish(getMid(mt), rid); loadMenu() }
    function lock(mt, rid) { menuService.toggleLock(getMid(mt), rid); loadMenu() }

    ListModel { id: bModel }
    ListModel { id: lModel }
    ListModel { id: dModel }

    property double totalCal: 0
    property double totalPro: 0
    property double totalCarb: 0
    property double totalFat: 0
    property double targetCal: 2000
    property double targetPro: 150
    property double targetCarb: 250
    property double targetFat: 65
    property double userTdee: 2000

    function calcNutrition() {
        totalCal = 0; totalPro = 0; totalCarb = 0; totalFat = 0
        sumModel(bModel); sumModel(lModel); sumModel(dModel)
        userTdee = authService.getUserTdee(session.userId)
        var s = menuService.getNutritionSummary(familyId, today, dietMode, dinerCount, userTdee)
        if (s.targetCal) {
            targetCal  = s.targetCal
            targetPro  = s.targetPro
            targetCarb = s.targetCarb
            targetFat  = s.targetFat
        }
        chartCanvas.requestPaint()
    }

    function sumModel(model) {
        for (var i = 0; i < model.count; i++) {
            var item = model.get(i)
            totalCal  += (item.calPer100 || 0)
            totalPro  += (item.proPer100 || 0)
            totalCarb += (item.carbPer100 || 0)
            totalFat  += (item.fatPer100 || 0)
        }
    }

    ListModel {
        id: dm
        ListElement { t: "减脂"; v: "weight_loss"; c: "#64C288" }
        ListElement { t: "增肌"; v: "muscle_gain"; c: "#FF9A5A" }
        ListElement { t: "维持"; v: "maintenance"; c: "#999999" }
        ListElement { t: "聚餐"; v: "gathering";   c: "#72B5E4" }
        ListElement { t: "放纵"; v: "cheat_day";   c: "#F9D07B" }
    }

    ScrollView {
        anchors { fill: parent; margins: Theme.spacingMedium }
        Column {
            width: parent.width
            spacing: Theme.spacingMedium

            // ---- 一键全天生 ----
            Rectangle {
                width: parent.width
                height: 40
                radius: Theme.radiusLarge
                color: Theme.primary
                Text { anchors.centerIn: parent; text: "一键生成全天菜单"; font.pixelSize: Theme.fontSizeBody; color: "white" }
                MouseArea { anchors.fill: parent; onClicked: {
                    gen("breakfast", breakfastCount)
                    gen("lunch", lunchCount)
                    gen("dinner", dinnerCount)
                }}
            }

            // ---- 顶部控制栏 + 饮食模式 ----
            Row {
                width: parent.width
                Row {
                    spacing: Theme.spacingSmall
                    Icon { name: "person"; size: 20; color: Theme.primary; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "今天由 我 掌勺"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                }
            }
            Row {
                spacing: Theme.spacingSmall
                Repeater {
                    model: dm
                    delegate: Rectangle {
                        width: 56
                        height: 28
                        radius: 8
                        color: dietMode === model.v ? Qt.lighter(model.c, 1.8) : Theme.bgCard
                        border { width: 1; color: dietMode === model.v ? model.c : Theme.borderLight }
                        Text { anchors.centerIn: parent; text: model.t; font.pixelSize: Theme.fontSizeSmall; color: dietMode === model.v ? model.c : Theme.textPrimary }
                        MouseArea { anchors.fill: parent; onClicked: { dietMode = model.v; calcNutrition() } }
                    }
                }
            }

            // ---- 柱状对比图 ----
            Canvas {
                id: chartCanvas
                width: parent.width
                height: 120
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    var maxCal = Math.max(targetCal, totalCal, 1)
                    var maxNut = Math.max(targetPro, targetCarb, targetFat, totalPro, totalCarb, totalFat, 1)
                    if (maxCal === 0) return
                    var labels = ["热量", "蛋白", "碳水", "脂肪"]
                    var curV   = [totalCal, totalPro, totalCarb, totalFat]
                    var tgtV   = [targetCal, targetPro, targetCarb, targetFat]
                    var colors = ["#FF9A5A", "#72B5E4", "#F9D07B", "#64C288"]
                    var barW = (width - 80) / 18
                    for (var i = 0; i < 4; i++) {
                        var x = 40 + i * barW * 5
                        var maxV = i === 0 ? maxCal : maxNut
                        // 推荐量（空心柱）
                        var hT = tgtV[i] / maxV * 90
                        ctx.strokeStyle = "#999999"
                        ctx.lineWidth = 2
                        ctx.strokeRect(x, 110 - hT, barW, hT)
                        // 当前量（实心柱）
                        var hC = curV[i] / maxV * 90
                        ctx.fillStyle = colors[i]
                        ctx.fillRect(x + barW + 4, 110 - hC, barW, hC)
                        // 标签
                        ctx.fillStyle = "#333333"
                        ctx.font = "10px sans-serif"
                        ctx.fillText(labels[i], x + 4, 118)
                    }
                    // 图例
                    ctx.fillStyle = "#64C288"
                    ctx.fillRect(width - 100, 4, 10, 10)
                    ctx.fillStyle = "#333333"
                    ctx.fillText("当前", width - 86, 14)
                    ctx.strokeStyle = "#999999"
                    ctx.lineWidth = 2
                    ctx.strokeRect(width - 100, 20, 10, 10)
                    ctx.fillText("推荐", width - 86, 30)
                }
            }

            // ---- 营养摘要 ----
            Flow {
                width: parent.width
                spacing: Theme.spacingMedium
                Rectangle {
                    width: parent.width > 0 ? (parent.width - Theme.spacingMedium * 3) / 4 : 60
                    height: 52
                    radius: Theme.radiusSmall
                    color: Theme.bgCard
                    border { width: 1; color: Theme.primary }
                    Column { anchors.centerIn: parent; spacing: 2
                        Text { text: totalCal.toFixed(0); font.pixelSize: 18; color: Theme.primary; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "kcal"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                }
                Rectangle {
                    width: parent.width > 0 ? (parent.width - Theme.spacingMedium * 3) / 4 : 60
                    height: 52
                    radius: Theme.radiusSmall
                    color: Theme.bgCard
                    border { width: 1; color: Theme.info }
                    Column { anchors.centerIn: parent; spacing: 2
                        Text { text: totalPro.toFixed(1); font.pixelSize: 18; color: Theme.info; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "蛋白 g"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                }
                Rectangle {
                    width: parent.width > 0 ? (parent.width - Theme.spacingMedium * 3) / 4 : 60
                    height: 52
                    radius: Theme.radiusSmall
                    color: Theme.bgCard
                    border { width: 1; color: Theme.accent }
                    Column { anchors.centerIn: parent; spacing: 2
                        Text { text: totalCarb.toFixed(1); font.pixelSize: 18; color: Theme.accent; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "碳水 g"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                }
                Rectangle {
                    width: parent.width > 0 ? (parent.width - Theme.spacingMedium * 3) / 4 : 60
                    height: 52
                    radius: Theme.radiusSmall
                    color: Theme.bgCard
                    border { width: 1; color: Theme.secondary }
                    Column { anchors.centerIn: parent; spacing: 2
                        Text { text: totalFat.toFixed(1); font.pixelSize: 18; color: Theme.secondary; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "脂肪 g"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                }
            }

            // 人数
            Row {
                spacing: Theme.spacingSmall
                Text { text: "用餐人数:"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 28; height: 28; radius: 14; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { text: "−"; anchors.centerIn: parent; color: Theme.textHint; MouseArea { anchors.fill: parent; onClicked: { if (dinerCount > 1) dinerCount-- } } }
                }
                Text { text: dinerCount + "人"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 28; height: 28; radius: 14; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { text: "+"; anchors.centerIn: parent; color: Theme.textHint; MouseArea { anchors.fill: parent; onClicked: dinerCount++ } }
                }
            }

            // ========== 早餐 ==========
            Row { spacing: Theme.spacingSmall
                Text { text: "🥣 早餐"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 20; height: 20; radius: 10; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { text: "−"; anchors.centerIn: parent; color: Theme.textHint; font.pixelSize: 14 }
                    MouseArea { anchors.fill: parent; onClicked: { if (breakfastCount > 1) breakfastCount-- } }
                }
                Text { text: breakfastCount + " 道"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 20; height: 20; radius: 10; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { text: "+"; anchors.centerIn: parent; color: Theme.textHint; font.pixelSize: 14 }
                    MouseArea { anchors.fill: parent; onClicked: breakfastCount++ }
                }
            }
            Row {
                spacing: Theme.spacingSmall
                Repeater {
                    model: bModel
                    delegate: DishCard { mt: "breakfast" }
                }
                Rectangle {
                    width: 80; height: 64; radius: 16; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { anchors.centerIn: parent; text: "生成"; font.pixelSize: Theme.fontSizeSmall; color: Theme.primary }
                    MouseArea { anchors.fill: parent; onClicked: gen("breakfast", breakfastCount) }
                }
            }

            // ========== 午餐 ==========
            Row { spacing: Theme.spacingSmall
                Text { text: "🍱 午餐"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 20; height: 20; radius: 10; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { text: "−"; anchors.centerIn: parent; color: Theme.textHint; font.pixelSize: 14 }
                    MouseArea { anchors.fill: parent; onClicked: { if (lunchCount > 1) lunchCount-- } }
                }
                Text { text: lunchCount + " 道"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 20; height: 20; radius: 10; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { text: "+"; anchors.centerIn: parent; color: Theme.textHint; font.pixelSize: 14 }
                    MouseArea { anchors.fill: parent; onClicked: lunchCount++ }
                }
            }
            Row {
                spacing: Theme.spacingSmall
                Repeater {
                    model: lModel
                    delegate: DishCard { mt: "lunch" }
                }
                Rectangle {
                    width: 80; height: 64; radius: 16; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { anchors.centerIn: parent; text: "生成"; font.pixelSize: Theme.fontSizeSmall; color: Theme.primary }
                    MouseArea { anchors.fill: parent; onClicked: gen("lunch", lunchCount) }
                }
            }

            // ========== 晚餐 ==========
            Row { spacing: Theme.spacingSmall
                Text { text: "🍲 晚餐"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 20; height: 20; radius: 10; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { text: "−"; anchors.centerIn: parent; color: Theme.textHint; font.pixelSize: 14 }
                    MouseArea { anchors.fill: parent; onClicked: { if (dinnerCount > 1) dinnerCount-- } }
                }
                Text { text: dinnerCount + " 道"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 20; height: 20; radius: 10; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { text: "+"; anchors.centerIn: parent; color: Theme.textHint; font.pixelSize: 14 }
                    MouseArea { anchors.fill: parent; onClicked: dinnerCount++ }
                }
            }
            Row {
                spacing: Theme.spacingSmall
                Repeater {
                    model: dModel
                    delegate: DishCard { mt: "dinner" }
                }
                Rectangle {
                    width: 80; height: 64; radius: 16; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { anchors.centerIn: parent; text: "生成"; font.pixelSize: Theme.fontSizeSmall; color: Theme.primary }
                    MouseArea { anchors.fill: parent; onClicked: gen("dinner", dinnerCount) }
                }
            }
        }
    }

    component DishCard: Rectangle {
        property string mt: ""
        width: 160
        height: 110
        radius: 16
        color: Theme.bgCard
        border { width: 1; color: Theme.borderLight }
        Column {
            anchors { fill: parent; margins: 12 }
            spacing: 6
            Text { text: model.name; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; elide: Text.ElideRight; width: parent.width }
            Text { text: model.calPer100.toFixed(0) + " kcal/100g"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }
            Row {
                spacing: 4
                Text { text: "份数:"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint; anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 18; height: 18; radius: 9; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { text: "−"; anchors.centerIn: parent; font.pixelSize: 12; color: Theme.textHint }
                    MouseArea { anchors.fill: parent; onClicked: {
                        var sv = (model.servingsOverride > 0 ? model.servingsOverride : 1.0) - 0.5
                        if (sv >= 0.5) {
                            var mid = getMid(mt)
                            menuService.adjustServings(mid, model.recipeId, sv)
                            loadMenu()
                        }
                    }}
                }
                Text { text: (model.servingsOverride > 0 ? model.servingsOverride : 1.0).toFixed(1); font.pixelSize: Theme.fontSizeSmall; color: Theme.info; anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 18; height: 18; radius: 9; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { text: "+"; anchors.centerIn: parent; font.pixelSize: 12; color: Theme.textHint }
                    MouseArea { anchors.fill: parent; onClicked: {
                        var sv = (model.servingsOverride > 0 ? model.servingsOverride : 1.0) + 0.5
                        if (sv <= dinerCount * 2.5) {
                            var mid = getMid(mt)
                            menuService.adjustServings(mid, model.recipeId, sv)
                            loadMenu()
                        }
                    }}
                }
            }
            Row {
                spacing: 12
                Text {
                    text: model.isLocked ? "🔒" : "🔓"
                    font.pixelSize: 16
                    MouseArea { anchors.fill: parent; onClicked: lock(mt, model.recipeId) }
                }
                Text {
                    text: "🔄"
                    font.pixelSize: 16
                    MouseArea { anchors.fill: parent; onClicked: swap(mt, model.recipeId) }
                }
            }
        }
    }

}
