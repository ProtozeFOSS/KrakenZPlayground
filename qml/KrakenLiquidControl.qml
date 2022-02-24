import QtQuick 2.15

Item {
    id:krakenLC
    width: 320
    height: width
    Connections{
        target: KrakenZDriver
        function onLiquidTemperatureChanged(temperature) { krakenLC.setColorLense(temperature);}

    }
    function setColorLense(temperature){
        if(temperature < 30){ // cool
            colorLense.color = "#039edb";
        }else if(temperature < 33){
            colorLense.color = "#31ffe7";
        }else if(temperature < 36){
            colorLense.color = "#deff4b";
        }else if(temperature < 39){
            colorLense.color = "#faff60";
        }else if(temperature < 42){
            colorLense.color = "#ffe560";
        }else if(temperature < 47){
            colorLense.color = "#ffbd60";
        }else if(temperature < 52){
            colorLense.color = "#ffbd60";
        }else{
            colorLense.color = "#ff1500";
        }
    }
    Rectangle{
        id: colorLense
        anchors{
            bottom:parent.bottom
            left:parent.left
            right:parent.right
        }
        height: parent.height * (KrakenZDriver.liquidTemperature/60) + 15
        color: "black"
    }
    Text{
        anchors.centerIn: parent
        text:KrakenZDriver.liquidTemperature.toString().slice(0,5) + "°"
        font.pixelSize: 64
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color:"white"
        styleColor: "black"
        style: Text.Outline
    }
    Component.onCompleted: {
        krakenLC.setColorLense(KrakenZDriver.liquidTemperature);
    }
}
