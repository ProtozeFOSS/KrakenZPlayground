import QtQuick 2.15

QtObject {
    id:systemObject
    function getSuffix(type: string):string {
        switch(type){
            case "Load":{
                return "%";
            }
            case "Temperature":{
                return "°C";
            }
            case "Clock":{
                return "Mhz";
            }
        }
    }
}
