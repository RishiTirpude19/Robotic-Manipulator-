# 6-DOF Robotic Manipulator

A custom-built **6 Degrees-of-Freedom (DOF) robotic arm** powered by the **ESP32 Wi-Fi module**, designed for flexible and remote control applications in embedded robotics. This manipulator supports both **manual control** and **inverse kinematics**, and offers a **“teach and repeat”** functionality—all accessible through a responsive **web-based interface**.

## 🚀 Features

- 🧠 **ESP32-based Control**: Fast, wireless microcontroller using C++ for real-time control.
- 🌐 **Wi-Fi Connectivity**: Control the arm remotely through a browser on the same network.
- 🤖 **Manual Control Mode**: Directly control individual joints of the arm from the UI.
- 🧮 **Inverse Kinematics Mode**: Move the end-effector to specific (x, y, z) coordinates using IK algorithms.
- 📚 **Teach and Repeat**: Record sequences of positions and replay them on demand.
- 🧩 **Modular Design**: Easily adaptable for additional DOF or gripper support.
- 💡 **Web Interface**: Intuitive and responsive control dashboard hosted on ESP32.

---

## ⚙️ Hardware Components

- 🔌 **ESP32 Dev Board**
- 🔧 **6x Servo Motors (SG90 or MG995 or similar)**
- 🦾 **Custom 3D-Printed or Assembled Arm**
- 🔋 **Power Supply (5V for servos, regulated)**
- 🧠 **Breadboard & Jumper Wires**
- 💻 **Wi-Fi-enabled device (PC/Mobile) for control**

---

## 💻 Software Stack

- 📟 **Firmware**: Written in **C++**, deployed to ESP32 using **Arduino IDE** / PlatformIO.
- 🌐 **Web Interface**: Hosted from ESP32, includes sliders, input fields, and buttons for control.
- 🔢 **Inverse Kinematics**: Implemented using trigonometric and geometric algorithms.
- 🧠 **Teach & Repeat**: Joint angles stored in memory arrays and replayed sequentially.

---

