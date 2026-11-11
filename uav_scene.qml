import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

// 3D-сцена задания 1. Объект "uav" (UavScene) передаётся из C++.
// Модель строится по motor_count(): лучи и винты расставляются
// равномерно по кругу, поэтому одна сцена подходит для любого наследника UAV.
Item {
    id: root

    readonly property int motors: uav.motor_count
    readonly property real armLength: 70 + motors * 6
    readonly property real propRadius: Math.min(48, armLength * Math.sin(Math.PI / Math.max(3, motors)) * 0.9)

    // Высота модели над площадкой: плавный подъём и спуск
    property real altitude: uav.airborne ? 160 : 0
    Behavior on altitude { NumberAnimation { duration: 2600; easing.type: Easing.InOutQuad } }

    readonly property bool spinning: uav.airborne || altitude > 0.5

    property real propAngle: 0
    NumberAnimation on propAngle {
        from: 0; to: 360
        duration: 260
        loops: Animation.Infinite
        running: root.spinning
    }

    // Лёгкое покачивание в зависании
    property real hover: 0
    SequentialAnimation on hover {
        loops: Animation.Infinite
        running: uav.airborne
        NumberAnimation { from: -4; to: 4; duration: 1400; easing.type: Easing.InOutSine }
        NumberAnimation { from: 4; to: -4; duration: 1400; easing.type: Easing.InOutSine }
    }

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#1d2430"
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        Node {
            id: cameraOrigin
            y: 80
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 230, 560)
                eulerRotation.x: -20
                clipFar: 5000
            }
        }

        DirectionalLight {
            eulerRotation.x: -50
            eulerRotation.y: 35
            brightness: 1.3
            castsShadow: true
            shadowFactor: 60
            shadowMapQuality: Light.ShadowMapQualityHigh
        }
        DirectionalLight {
            eulerRotation.x: -20
            eulerRotation.y: -140
            brightness: 0.35
        }

        // Земля
        Model {
            source: "#Rectangle"
            eulerRotation.x: -90
            scale: Qt.vector3d(20, 20, 1)
            materials: PrincipledMaterial { baseColor: "#33402f"; roughness: 0.95 }
        }
        // Посадочная площадка
        Model {
            source: "#Cylinder"
            y: 1
            scale: Qt.vector3d(2.6, 0.02, 2.6)
            materials: PrincipledMaterial { baseColor: "#5b6270"; roughness: 0.8 }
        }
        Model {
            source: "#Cylinder"
            y: 2.2
            scale: Qt.vector3d(2.3, 0.01, 2.3)
            materials: PrincipledMaterial { baseColor: "#f2b134"; roughness: 0.6 }
        }
        Model {
            source: "#Cylinder"
            y: 2.6
            scale: Qt.vector3d(2.15, 0.01, 2.15)
            materials: PrincipledMaterial { baseColor: "#5b6270"; roughness: 0.8 }
        }

        // ---------- Аппарат ----------
        Node {
            id: drone
            y: 26 + root.altitude + root.hover

            // Корпус
            Model {
                source: "#Sphere"
                scale: Qt.vector3d(0.62, 0.24, 0.62)
                materials: PrincipledMaterial { baseColor: "#d9dde3"; metalness: 0.2; roughness: 0.35 }
            }
            Model {
                source: "#Cylinder"
                y: 9
                scale: Qt.vector3d(0.4, 0.05, 0.4)
                materials: PrincipledMaterial { baseColor: "#2c3440"; roughness: 0.5 }
            }
            // Камера-подвес (показывает «нос» аппарата)
            Model {
                source: "#Sphere"
                y: -12; z: 18
                scale: Qt.vector3d(0.16, 0.16, 0.16)
                materials: PrincipledMaterial { baseColor: "#111418"; roughness: 0.2 }
            }

            // Шасси
            Repeater3D {
                model: 4
                delegate: Node {
                    required property int index
                    eulerRotation.y: 45 + index * 90
                    Model {
                        source: "#Cylinder"
                        x: 20; y: -16
                        scale: Qt.vector3d(0.04, 0.26, 0.04)
                        materials: PrincipledMaterial { baseColor: "#2c3440" }
                    }
                }
            }

            // Лучи, моторы и винты — по одному на каждый мотор
            Repeater3D {
                model: root.motors
                delegate: Node {
                    id: arm
                    required property int index
                    readonly property bool front: index === 0 || index === root.motors - 1
                    eulerRotation.y: 90 - 180 / root.motors - index * 360 / root.motors

                    Model {
                        source: "#Cube"
                        x: root.armLength / 2
                        scale: Qt.vector3d(root.armLength / 100, 0.05, 0.07)
                        materials: PrincipledMaterial { baseColor: "#3a4350"; roughness: 0.5 }
                    }
                    Model {
                        source: "#Cylinder"
                        x: root.armLength; y: 5
                        scale: Qt.vector3d(0.11, 0.14, 0.11)
                        materials: PrincipledMaterial {
                            baseColor: arm.front ? "#e0533d" : "#1f2329"
                            metalness: 0.4; roughness: 0.4
                        }
                    }
                    Node {
                        x: root.armLength; y: 13
                        eulerRotation.y: (arm.index % 2 === 0 ? 1 : -1) * root.propAngle + arm.index * 37

                        Model {
                            source: "#Cube"
                            scale: Qt.vector3d(root.propRadius * 2 / 100, 0.012, 0.06)
                            materials: PrincipledMaterial { baseColor: "#e9edf2"; roughness: 0.4 }
                        }
                        // Диск вращения — заметен, когда винты крутятся
                        Model {
                            source: "#Cylinder"
                            scale: Qt.vector3d(root.propRadius / 50, 0.004, root.propRadius / 50)
                            opacity: root.spinning ? 0.22 : 0.0
                            materials: PrincipledMaterial {
                                baseColor: "#9fd3ff"
                                alphaMode: PrincipledMaterial.Blend
                                lighting: PrincipledMaterial.NoLighting
                            }
                        }
                    }
                }
            }
        }
    }

    OrbitCameraController {
        anchors.fill: view
        origin: cameraOrigin
        camera: camera
    }

    // ---------- Подписи поверх сцены ----------
    Column {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 16
        spacing: 4

        Text {
            text: uav.type_name + ": " + uav.motor_count + " motors"
            color: "white"
            font.pixelSize: 22
            font.bold: true
        }
        Text {
            text: uav.airborne ? "Состояние: в воздухе" : "Состояние: на земле"
            color: uav.airborne ? "#7fe08a" : "#c9ced6"
            font.pixelSize: 15
        }
        Text {
            text: "Высота модели: " + Math.round(root.altitude / 160 * 30) + " м"
            color: "#c9ced6"
            font.pixelSize: 15
        }
    }

    Text {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        text: "ЛКМ — вращать камеру, колесо — масштаб. Красные моторы — передние."
        color: "#8a93a3"
        font.pixelSize: 12
    }
}
