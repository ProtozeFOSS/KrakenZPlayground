import QtQuick 2.15

Rectangle{
    radius: 8
    color: "#526ca4"
    border.color: "#e64f4f4f"
    height: 64 + attributeView.height
    //Row{
        // icon
        // name
        // label
        // mapped edit pencil - (if mapped, sensor name, device[0], value)
        // else, show a set map button
    //}
    Image{
        id: sensorImage
        anchors{
            left:parent.left
            top:parent.top
            margins:8
            topMargin:6
        }
        height:64
        width:64
        fillMode:Image.PreserveAspectFit
        source:SystemMonitor.sensorIconSource(modelData.type)
    }
    Row{
        id:flatHeader
        spacing:width/4
        width:parent.width
        height:24
        anchors{
            left:sensorImage.right
            top:sensorImage.top
            leftMargin:8
        }

        Text{
            text:"Sensor"
            color:"#bbbbbb"
            style:Text.Sunken
            styleColor: "black"
            font.bold: true
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
        }
        Text{
            text:modelData.label ? "Label":""
            color:"#bbbbbb"
            style:Text.Sunken
            styleColor: "black"
            font.bold: true
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
        }
    }
    Row{
        id:header
        anchors{
            top:flatHeader.bottom
            left:sensorImage.right
            right:parent.right
            leftMargin:10
            topMargin:-15
        }
        padding:2
        spacing:width/4
        TextEdit{
            color:"white"
            text:modelData.name ? modelData.name:modelData.path
            font.pixelSize: 24
            font.bold: true
            readOnly:true
            selectByMouse:true
        }
        Text{
            color:"white"
            text:modelData.label ? modelData.label:""
            font.pixelSize:24
            font.bold: true
        }

    }

    Column{
        id:attributeView
        anchors{
            top:sensorImage.bottom
            left:parent.left
            right:parent.right
            rightMargin:8
            leftMargin:24
            topMargin:-16
        }
        padding:2
        Row{
            id:smallHeader
            width:parent.width
            spacing:width/4
            height:18
            leftPadding:80
            Text{
                text:"Attribute"
                color:"#bbbbbb"
                style:Text.Sunken
                styleColor: "black"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
            }
            Text{
                text:"Value"
                color:"#bbbbbb"
                style:Text.Sunken
                styleColor: "black"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
            }
            Text{
                text:"Mapping"
                color:"#bbbbbb"
                style:Text.Sunken
                styleColor: "black"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
            }
        }
        Repeater{
            model:modelData.attributes
            delegate:Rectangle{
                height: 32
                width:attributeView.width
                border.color: "gray"
                border.width: 1
                radius:6
                Text{
                    id: sensorNameText
                    leftPadding:8
                    text:modelData.name.length > 0 ? modelData.name: modelData.path;
                    font.pixelSize: 16
                    width:parent.width/3
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    anchors{
                        left:parent.left
                        verticalCenter: parent.verticalCenter
                    }
                }
                Text{
                    anchors{
                        left:sensorNameText.right
                        verticalCenter:parent.verticalCenter
                        right:mappingLoader.left
                        margins:2
                        rightMargin:16
                    }
                    text:modelData.value !== undefined ? modelData.value.toFixed(3):""
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                //                Text{
                //                    text:SystemMonitor.sensorTypeToString(modelData.type)
                //                }

                //                Text{
                //                    text:modelData.label
                //                }

                Loader{
                    id: mappingLoader
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right:parent.right
                    anchors.rightMargin: 4
                    sourceComponent:modelData.sensor !== null ? mappedSensorComponent : mapButtonComponent
                    onItemChanged: {
                        if(item) {
                            item.parent = parent;
                            width = item.width;
                            height = item.height;
                            item.modelData = modelData;
                        }
                    }
                }
            }
        }
    }

    Component{
        id: mapButtonComponent
        Rectangle{
            height:22
            width:130
            radius:4
            color:"lightblue"
            anchors.verticalCenter: parent.verticalCenter
            anchors.right:parent.right
            anchors.rightMargin:24
            property var modelData:null
            Text{
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                text:"Map To Sensor"
                verticalAlignment:Text.AlignVCenter
                font.pixelSize: 14
            }
            MouseArea{
                id:mouse
                anchors.fill: parent
                onClicked:{
                    if(modelData) {
                        console.log("Clicked to map: " + modelData.name);
                    }
                }
            }
        }
    }
    Component{
        id:mappedSensorComponent
        Row{
           height:parent.height
           width:190
           anchors.verticalCenter: parent.verticalCenter
           anchors.right:parent.right
           anchors.rightMargin:8
           property var modelData:null
           spacing:4
           onModelDataChanged: {
               if(modelData) {
                  deviceIcon.source = SystemMonitor.sensorIconSource(modelData.sensor)
                  sensorLabel.text = SystemMonitor.sensorIDToString(modelData.sensor) +"[" + modelData.device + "]"
               }
           }

           Image{
               // mapped sensor id
               // icon, Name of value
               // value
               id:deviceIcon
               anchors.verticalCenter:parent.verticalCenter
               height:parent.height -4
               fillMode:Image.PreserveAspectFit
               source: ""
           }
           Text{
               id:sensorLabel
               text:""
               anchors.verticalCenter: parent.verticalCenter
               horizontalAlignment: Text.AlignHCenter
               verticalAlignment: Text.AlignVCenter
               font.pixelSize: 12
           }
        }
    }
}
