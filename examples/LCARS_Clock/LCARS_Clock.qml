import QtQuick 2.15

Rectangle {
    id: circleDigitClock
    anchors.fill: parent

    Connections{
        target:AppController
        function onDraw() {
            canvas.now = new Date()
            canvas.requestPaint()
        }
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        property var now : new Date()

        onPaint: {
            var today = now.toDateString()
            var time = now.toLocaleTimeString()
            var hrs = now.getHours()
            var min = now.getMinutes()
            var sec = now.getSeconds()
            var mil = now.getMilliseconds()
            var smoothsec = sec + (mil/1000)
            var smoothmin = min + (smoothsec/60)

            var ctx = canvas.getContext("2d")
            ctx.strokeStyle = '#9a99ff'
            ctx.lineWidth = 17
            ctx.shadowBlur= 2
            ctx.shadowColor = 'black'

            //Background
            ctx.fillStyle = '#ffcc9a'
            ctx.fillStyle = 'rgba(00 ,00 , 00, 1)'
            ctx.fillRect(0, 0, 500, 500)

            //Hours
            ctx.beginPath()
            ctx.arc(160, 160, 150, degToRad(270), degToRad((hrs*30)-90))
            ctx.stroke()

            ctx.strokeStyle = '#ff9a66'
            //Minutes
            ctx.beginPath()
            ctx.arc(160, 160, 130, degToRad(284), degToRad((smoothmin*6)-76))
            ctx.stroke()

            ctx.strokeStyle = "#cd6667";
            //Seconds
            ctx.beginPath()
            ctx.arc(160, 160, 110, degToRad(298), degToRad((smoothsec*6)-62))
            ctx.stroke()

            ctx.strokeStyle = 'black'
            ctx.shadowColor = '#9a99ff';
            //Date
            ctx.font = "22px Arial bold"
            ctx.lineWidth = 1;
            ctx.strokeText(today, 72, 180);
            ctx.fillStyle = '#cc6698'
            ctx.fillText(today, 72, 180)

            //Time
            var timeOut = time+":"+mil;
            ctx.font = "20px Arial Bold"
            ctx.strokeText(timeOut, 85, 150);
            ctx.fillStyle = '#cc6698'
            ctx.fillText(timeOut, 85, 150)
        }

        function degToRad(degree) {
            var factor = Math.PI/180
            return degree*factor
        }
    }
    Component.onCompleted: {
        AppController.setFrameDelay(920);
    }
}
