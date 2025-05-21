import tkinter as tk
from tkinter import messagebox
import serial
import time
import threading
import Kinematic
import cv2
from PIL import Image, ImageTk

# Setup Serial
ser = serial.Serial('COM5', 9600)
time.sleep(1)

def send_angles():
    try:
        theta1 = float(entry_theta1.get())
        theta2 = float(entry_theta2.get())
        theta3 = float(entry_theta3.get())

        if not (0 <= theta1 <= 45):
            messagebox.showerror("Error", "Theta 1 phải trong khoảng 0 đến 45")
            return
        if not (0 <= theta2 <= 45):
            messagebox.showerror("Error", "Theta 2 phải trong khoảng 0 đến 45")
            return
        if not (0 <= theta3 <= 45):
            messagebox.showerror("Error", "Theta 3 phải trong khoảng 0 đến 45")
            return

        try:
            result = Kinematic.forward_kinematic(theta1, theta2, theta3)
            if result[0] is False:
                messagebox.showerror("Error", "Không thể tính vị trí. Kiểm tra lại góc đầu vào.")
                return

            _, x, y, z = result  # Bỏ qua giá trị đầu vì nó là True/False

            # Giới hạn tọa độ (ví dụ)
            if not (-149.34 <= x <= 158.08):
                messagebox.showerror("Lỗi", f"X={x:.2f} nằm ngoài giới hạn robot")
                return
            if not (-137.94 <= y <= 137.94):
                messagebox.showerror("Lỗi", f"Y={y:.2f} nằm ngoài giới hạn robot")
                return
            if not (-398.23 <= z <= -287.30):
                messagebox.showerror("Lỗi", f"Z={z:.2f} nằm ngoài giới hạn robot")
                return

            # Nếu vị trí hợp lệ, gửi dữ liệu
            data = f"{theta1}A{theta2}B{theta3}C\r"
            print(f"Gửi: {data.strip()}")
            ser.write(data.encode())
            # Cập nhật giao diện
            entry_x.config(state='normal')
            entry_y.config(state='normal')
            entry_z.config(state='normal')
            entry_x.delete(0, tk.END)
            entry_y.delete(0, tk.END)
            entry_z.delete(0, tk.END)
            entry_x.insert(0, f"{x:.2f}")
            entry_y.insert(0, f"{y:.2f}")
            entry_z.insert(0, f"{z:.2f}")
            entry_x.config(state='readonly')
            entry_y.config(state='readonly')
            entry_z.config(state='readonly')

        except Exception as e:
            print(f"Lỗi khi tính forward kinematic: {e}")
            messagebox.showerror("Lỗi", "Không thể tính vị trí. Kiểm tra lại hàm forward_kinematic.")

    except ValueError:
        messagebox.showerror("Error", "Vui lòng nhập đúng định dạng số!")

def calculate_inv_kinematic():
    try:
        x_val = float(entry_x_ik.get())
        y_val = float(entry_y_ik.get())
        z_val = float(entry_z_ik.get())

        angles = Kinematic.inverse_kinematic(x_val, y_val, z_val)

        entry_theta1_ik.config(state=tk.NORMAL)
        entry_theta1_ik.delete(0, tk.END)
        entry_theta1_ik.insert(0, f"{angles[0]:.2f}")
        entry_theta1_ik.config(state='readonly')

        entry_theta2_ik.config(state=tk.NORMAL)
        entry_theta2_ik.delete(0, tk.END)
        entry_theta2_ik.insert(0, f"{angles[1]:.2f}")
        entry_theta2_ik.config(state='readonly')

        entry_theta3_ik.config(state=tk.NORMAL)
        entry_theta3_ik.delete(0, tk.END)
        entry_theta3_ik.insert(0, f"{angles[2]:.2f}")
        entry_theta3_ik.config(state='readonly')

    except ValueError:
        messagebox.showerror("Lỗi", "Vui lòng nhập giá trị số hợp lệ cho X, Y, Z.")
    except ValueError as e:
        messagebox.showerror("Lỗi tính toán", str(e))
    except Exception as e:
        messagebox.showerror("Lỗi không xác định", f"Đã xảy ra lỗi: {e}")
# def run_trajectory(P0, Pf, tf=3.0, dt=0.1):
#     t = 0
#     while t <= tf:
#         x, y, z = Kinematic.trajectory_planning_2_point(t, P0, Pf, tf)
#         try:
#             theta1, theta2, theta3 = Kinematic.inverse_kinematic(x, y, z)
#             data = f"{theta1:.2f}A{theta2:.2f}B{theta3:.2f}C\r"
#             print(f"[{t:.2f}s] Gửi: {data.strip()}")
#             ser.write(data.encode())
#             while True:
#                 if ser.in_waiting > 0:
#                     resp = ser.readline().decode().strip()
#                     if resp == "OK":
#                         break
#             time.sleep(dt)
#             t += dt
#         except Exception as e:
#             print(f"Lỗi tại t={t:.2f}s: {e}")
#             break
def send_trajectory():
    try:
        x0 = float(entry_x0.get())
        y0 = float(entry_y0.get())
        z0 = float(entry_z0.get())
        xf = float(entry_xf.get())
        yf = float(entry_yf.get())
        zf = float(entry_zf.get())
        tf = float(entry_tf.get())  # thêm ô nhập tf

        # Gửi dạng: x0,y0,z0|xf,yf,zf|tf
        data = f"{x0},{y0},{z0}|{xf},{yf},{zf}|{tf}\r"
        ser.write(data.encode())
        print(f"Gửi: {data.strip()}")

    except Exception as e:
        messagebox.showerror("Lỗi", str(e))

# def start_trajectory():
#     try:
#         # Lấy P0
#         x0 = float(entry_x0.get())
#         y0 = float(entry_y0.get())
#         z0 = float(entry_z0.get())
#
#         # Lấy Pf
#         xf = float(entry_xf.get())
#         yf = float(entry_yf.get())
#         zf = float(entry_zf.get())
#
#         tf = 5.0  # thời gian quỹ đạo, bạn có thể thêm ô nhập nếu muốn chỉnh thời gian
#
#         threading.Thread(target=run_trajectory, args=((x0, y0, z0), (xf, yf, zf), tf)).start()
#
#     except Exception as e:
#         messagebox.showerror("Lỗi", f"Không thể khởi động quỹ đạo: {e}")
def set_home():
    ser.write(bytes('h' + '\r', 'utf-8'))
    result = Kinematic.forward_kinematic(0,0,0)

    _, x, y, z = result  # bỏ qua giá trị đầu vì nó là true, false

    entry_x.config(state='normal')  # cho phép thay đổi dữ liệu
    entry_y.config(state='normal')
    entry_z.config(state='normal')
    entry_x.delete(0, tk.END)  # xóa dữ liệu cũ
    entry_y.delete(0, tk.END)
    entry_z.delete(0, tk.END)
    entry_x.insert(0, f"{x:.2f}")  # ghi dữ liệu mới
    entry_y.insert(0, f"{y:.2f}")
    entry_z.insert(0, f"{z:.2f}")
    entry_x.config(state='readonly')  # khóa lại để người dùng k sửa đc
    entry_y.config(state='readonly')
    entry_z.config(state='readonly')
def stop():
    ser.write(bytes('s' + '\r', 'utf-8'))
def hut_namcham():
    ser.write(bytes('u' + '\r', 'utf-8'))
def tha_namcham():
    ser.write(bytes('d' + '\r', 'utf-8'))
def read_serial():
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            print(f"Nhận: {line}")
            text_box.insert(tk.END, line + "\n")
            text_box.see(tk.END)  # Auto scroll xuống dòng mới nhất
        time.sleep(0.1)

def send_theta1():
    try:
        val = float(entry_theta1.get())
        if not (0 <= val <= 45):
            messagebox.showerror("Error", "Theta 1 phải trong khoảng 0 đến 45")
            return
        ser.write(f"{val:.2f}A\r".encode())
        print(f"Gửi: {val:.2f}A")
    except ValueError:
        messagebox.showerror("Error", "Theta 1 không hợp lệ!")

def send_theta2():
    try:
        val = float(entry_theta2.get())
        if not (0 <= val <= 45):
            messagebox.showerror("Error", "Theta 2 phải trong khoảng 0 đến 45")
            return
        ser.write(f"{val:.2f}B\r".encode())
        print(f"Gửi: {val:.2f}B")
    except ValueError:
        messagebox.showerror("Error", "Theta 2 không hợp lệ!")

def send_theta3():
    try:
        val = float(entry_theta3.get())
        if not (0 <= val <= 45):
            messagebox.showerror("Error", "Theta 3 phải trong khoảng 0 đến 45")
            return
        ser.write(f"{val:.2f}C\r".encode())
        print(f"Gửi: {val:.2f}C")
    except ValueError:
        messagebox.showerror("Error", "Theta 3 không hợp lệ!")

cap = None
cam_running = False

def start_camera():
    global cap, cam_running
    if not cam_running:
        cap = cv2.VideoCapture(0)
        cam_running = True
        update_frame()

def stop_camera():
    global cap, cam_running
    cam_running = False
    if cap:
        cap.release()
        label_cam.config(image='')

def update_frame():
    global cap, cam_running
    if cam_running and cap.isOpened():
        ret, frame = cap.read()
        if ret:
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            img = Image.fromarray(frame)
            imgtk = ImageTk.PhotoImage(image=img)
            label_cam.imgtk = imgtk
            label_cam.configure(image=imgtk)
        label_cam.after(10, update_frame)

# Giao diện chính
window = tk.Tk()
# window.title("Gửi góc điều khiển tới Arduino")
window.geometry("850x750")
window.configure(bg="#f0f0f5")  # Màu nền nhẹ nhàng

font_title = ("Arial", 20, "bold")
font_label = ("Arial", 12)
font_entry = ("Arial", 12)
font_button = ("Arial", 12, "bold")

# Hiển thị ảnh khoa đào tạo
try:
    image = Image.open("Banner0.png")  # Thay bằng tên file ảnh của bạn
    image = image.resize((750, 100))  # Resize ảnh nếu cần
    photo = ImageTk.PhotoImage(image)
    label_image = tk.Label(window, image=photo, bg="#f0f0f5")
    label_image.image = photo  # Giữ tham chiếu ảnh
    label_image.pack(pady=(10, 5))
except Exception as e:
    print(f"Lỗi khi tải ảnh: {e}")
title = tk.Label(window, text="CONTROL INTERFACE", font=font_title, bg="#f0f0f5", fg="#333")
title.pack(pady=10)

# Frame chính chứa các phần bên trái và bên phải
frame_main = tk.Frame(window, bg="#f0f0f5")
frame_main.pack(pady=10, padx=20, fill="x") # Thêm padx và fill để frame chính rộng hơn

# Inputs bên trái
frame_inputs = tk.Frame(frame_main, bg="#f0f0f5")
frame_inputs.pack(side=tk.LEFT, fill="y") # fill="y" để frame inputs cao bằng frame buttons
# ||||||||||||||||||||||||||
# label_for_kinematic = tk.Label(frame_inputs, text="FORWARD KINEMATIC", font=("Helvetica", 17, "bold"), bg="#f0f0f5", fg="#333")
# label_for_kinematic.grid(row=0, column=0, columnspan=3, padx=10, pady=(20, 5), sticky="w")
# |||||||||||||||||||||||||||
# Khung tổng cho phần bên phải
frame_camera = tk.Frame(frame_main, bg="#e0e0e0", bd=2, relief=tk.SUNKEN)
frame_camera.pack(side=tk.RIGHT, padx=10, pady=10, fill="both", expand=True)

# Label hiển thị camera - chiếm phần lớn khung
label_cam = tk.Label(frame_camera, bg="black")
label_cam.pack(fill="both", expand=True, padx=10, pady=10)

# Khung chứa nút start/stop đặt phía dưới cùng bên phải
frame_cam_buttons = tk.Frame(frame_camera, bg="#e0e0e0")
frame_cam_buttons.pack(side=tk.BOTTOM, anchor="e", padx=10, pady=10)

btn_start_cam = tk.Button(frame_cam_buttons, text="START CAMERA", command=start_camera,
                          font=font_button, bg="#4CAF50", fg="white", width=16)
btn_start_cam.pack(side=tk.LEFT, padx=5)

btn_stop_cam = tk.Button(frame_cam_buttons, text="STOP CAMERA", command=stop_camera,
                         font=font_button, bg="#f44336", fg="white", width=16)
btn_stop_cam.pack(side=tk.LEFT, padx=5)


label_for_kinematic = tk.Label(frame_inputs, text="FORWARD KINEMATIC", font=("Helvetica", 17, "bold"), bg="#f0f0f5", fg="#333")
label_for_kinematic.grid(row=0, column=0, columnspan=2, padx=10, pady=(20, 5), sticky="w")
# sticky: xác định tiện ích bám vào phía nào của ô
# columnspan:chỉ định số cột tiện ích chiếm giữ

# Theta 1
label_theta1 = tk.Label(frame_inputs, text="Theta 1 (°):", font=font_label, bg="#f0f0f5")
label_theta1.grid(row=1, column=0, padx=10, pady=5, sticky="w")
entry_theta1 = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_theta1.grid(row=1, column=1, pady=5)
button_t1 = tk.Button(frame_inputs, text="SEND", command=send_theta1, font=font_button, bg="#9C27B0", fg="white", width=6)
button_t1.grid(row=1, column=2, padx=5)

# Theta 2
label_theta2 = tk.Label(frame_inputs, text="Theta 2 (°):", font=font_label, bg="#f0f0f5")
label_theta2.grid(row=2, column=0, padx=10, pady=5, sticky="w")
entry_theta2 = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_theta2.grid(row=2, column=1, pady=5)
button_t2 = tk.Button(frame_inputs, text="SEND", command=send_theta2, font=font_button, bg="#FF9800", fg="white", width=6)
button_t2.grid(row=2, column=2, padx=5)

# Theta 3
label_theta3 = tk.Label(frame_inputs, text="Theta 3 (°):", font=font_label, bg="#f0f0f5")
label_theta3.grid(row=3, column=0, padx=10, pady=5, sticky="w")
entry_theta3 = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_theta3.grid(row=3, column=1, pady=5)
button_t3 = tk.Button(frame_inputs, text="SEND", command=send_theta3, font=font_button, bg="#3F51B5", fg="white", width=6)
button_t3.grid(row=3, column=2, padx=5)

# Nhãn "Vị trí (mm):"
label_result = tk.Label(frame_inputs, text="POSITION (mm):", font=("Helvetica", 17, "bold"), bg="#f0f0f5")
label_result.grid(row=4, column=0, columnspan=3, padx=10, pady=(20, 5), sticky="w")

# --- Khối hiển thị X Y Z theo hàng ngang gọn ---
frame_xyz = tk.Frame(frame_inputs, bg="#f0f0f5")
frame_xyz.grid(row=5, column=0, columnspan=6, padx=10, pady=5, sticky="w")

label_x = tk.Label(frame_xyz, text="X:", font=font_label, bg="#f0f0f5")
label_x.pack(side=tk.LEFT, padx=(0, 2))
entry_x = tk.Entry(frame_xyz, font=font_entry, width=8, state='readonly')
entry_x.pack(side=tk.LEFT, padx=(0, 10))

label_y = tk.Label(frame_xyz, text="Y:", font=font_label, bg="#f0f0f5")
label_y.pack(side=tk.LEFT, padx=(0, 2))
entry_y = tk.Entry(frame_xyz, font=font_entry, width=8, state='readonly')
entry_y.pack(side=tk.LEFT, padx=(0, 10))

label_z = tk.Label(frame_xyz, text="Z:", font=font_label, bg="#f0f0f5")
label_z.pack(side=tk.LEFT, padx=(0, 2))
entry_z = tk.Entry(frame_xyz, font=font_entry, width=8, state='readonly')
entry_z.pack(side=tk.LEFT, padx=(0, 10))

# --- Khối INVERSE ---
label_inv_kinematic = tk.Label(frame_inputs, text="INVERSE KINEMATIC", font=("Helvetica", 17, "bold"), bg="#f0f0f5", fg="#333")
label_inv_kinematic.grid(row=6, column=0, columnspan=3, padx=10, pady=(20, 5), sticky="w")

frame_x_ik = tk.Frame(frame_inputs, bg="#f0f0f5")
frame_x_ik.grid(row=7, column=0, columnspan=6, padx=10, pady=5, sticky="w")

label_x_ik = tk.Label(frame_x_ik, text="X:", font=font_label, bg="#f0f0f5")
label_x_ik.pack(side=tk.LEFT, padx=(0, 2))
entry_x_ik = tk.Entry(frame_x_ik, font=font_entry, width=8)
entry_x_ik.pack(side=tk.LEFT, padx=(0, 10))
label_theta1_ik = tk.Label(frame_x_ik, text="Theta1:", font=font_label, bg="#f0f0f5")
label_theta1_ik.pack(side=tk.LEFT, padx=(0, 4))
entry_theta1_ik = tk.Entry(frame_x_ik, font=font_entry, width=8, state='readonly')
entry_theta1_ik.pack(side=tk.LEFT, padx=(0, 10))

frame_y_ik = tk.Frame(frame_inputs, bg="#f0f0f5")
frame_y_ik.grid(row=8, column=0, columnspan=6, padx=10, pady=5, sticky="w")

label_y_ik = tk.Label(frame_y_ik, text="Y:", font=font_label, bg="#f0f0f5")
label_y_ik.pack(side=tk.LEFT, padx=(0, 2))
entry_y_ik = tk.Entry(frame_y_ik, font=font_entry, width=8)
entry_y_ik.pack(side=tk.LEFT, padx=(0, 10))
label_theta2_ik = tk.Label(frame_y_ik, text="Theta2:", font=font_label, bg="#f0f0f5")
label_theta2_ik.pack(side=tk.LEFT, padx=(0, 4))
entry_theta2_ik = tk.Entry(frame_y_ik, font=font_entry, width=8, state='readonly')
entry_theta2_ik.pack(side=tk.LEFT, padx=(0, 10))

frame_z_ik = tk.Frame(frame_inputs, bg="#f0f0f5")
frame_z_ik.grid(row=9, column=0, columnspan=6, padx=10, pady=5, sticky="w")

label_z_ik = tk.Label(frame_z_ik, text="Z:", font=font_label, bg="#f0f0f5")
label_z_ik.pack(side=tk.LEFT, padx=(0, 2))
entry_z_ik = tk.Entry(frame_z_ik, font=font_entry, width=8)
entry_z_ik.pack(side=tk.LEFT, padx=(0, 10))
label_theta3_ik = tk.Label(frame_z_ik, text="Theta3:", font=font_label, bg="#f0f0f5")
label_theta3_ik.pack(side=tk.LEFT, padx=(0, 4))
entry_theta3_ik = tk.Entry(frame_z_ik, font=font_entry, width=8, state='readonly')
entry_theta3_ik.pack(side=tk.LEFT, padx=(0, 10))

# Nút tính toán Inverse Kinematic
btn_calc_ik = tk.Button(
    frame_inputs, text="CAL IK", command=calculate_inv_kinematic,
    font=font_button, bg="#607D8B", fg="white", width=12
)
btn_calc_ik.grid(row=10, column=0, columnspan=2, pady=10)

# Nút điều khiển bên phải
# frame_buttons = tk.Frame(frame_main, bg="#f0f0f5")
# frame_buttons.pack(side=tk.RIGHT, padx=20, fill="y") # fill="y" để frame buttons cao bằng frame inputs
frame_buttons = tk.Frame(frame_inputs, bg="#f0f0f5")
frame_buttons.grid(row=7, column=5, rowspan=5, padx=(20, 0), sticky="n")
# ||||||||||||||||||||||||||||
# button_home = tk.Button(frame_buttons, text="SET HOME", command=set_home,
#                         font=font_button, bg="#4CAF50", fg="white", width=15)
# button_home.pack(pady=5, fill="x") # fill="x" để nút rộng bằng frame
# |||||||||||||||||||||||||
# button_home = tk.Button(frame_inputs, text="SET HOME", command=set_home,
#                         font=font_button, bg="#4CAF50", fg="white", width=12)
# button_home.grid(row=0, column=3, padx=10, pady=(20, 5), sticky="e")
# button_send = tk.Button(frame_buttons, text="SEND DATA", command=send_angles,
#                         font=font_button, bg="#008CBA", fg="white", width=15)
# button_send.pack(pady=5, fill="x")
#
# button_stop = tk.Button(frame_buttons, text="STOP", command=stop,
#                         font=font_button, bg="#f44336", fg="white", width=15)
# button_stop.pack(pady=5, fill="x")
#
# button_hut = tk.Button(frame_buttons, text="UP", command=hut_namcham,
#                        font=font_button, bg="#009688", fg="white", width=15)
# button_hut.pack(pady=5, fill="x")
#
# button_tha = tk.Button(frame_buttons, text="DOWN", command=tha_namcham,
#                        font=font_button, bg="#795548", fg="white", width=15)
# button_tha.pack(pady=5, fill="x")
#
# # --- Ô nhập điểm đầu và điểm cuối cho quỹ đạo ---
# label_traj = tk.Label(frame_buttons, text="TRAJECTORY POINTS", font=("Helvetica", 15, "bold"),
#                       bg="#f0f0f5", fg="#333")
# label_traj.pack(pady=(15, 5))
#
# # Điểm đầu P0
# label_p0 = tk.Label(frame_buttons, text="P0: (X0, Y0, Z0)", font=font_label, bg="#f0f0f5")
# label_p0.pack()
#
# frame_p0 = tk.Frame(frame_buttons, bg="#f0f0f5")
# frame_p0.pack(pady=3)
#
# entry_x0 = tk.Entry(frame_p0, font=font_entry, width=6)
# entry_x0.pack(side=tk.LEFT, padx=2)
#
# entry_y0 = tk.Entry(frame_p0, font=font_entry, width=6)
# entry_y0.pack(side=tk.LEFT, padx=2)
#
# entry_z0 = tk.Entry(frame_p0, font=font_entry, width=6)
# entry_z0.pack(side=tk.LEFT, padx=2)
#
# # Điểm cuối Pf
# label_pf = tk.Label(frame_buttons, text="Pf: (Xf, Yf, Zf, tf)", font=font_label, bg="#f0f0f5")
# label_pf.pack()
#
# frame_pf = tk.Frame(frame_buttons, bg="#f0f0f5")
# frame_pf.pack(pady=3)
#
# entry_xf = tk.Entry(frame_pf, font=font_entry, width=6)
# entry_xf.pack(side=tk.LEFT, padx=2)
#
# entry_yf = tk.Entry(frame_pf, font=font_entry, width=6)
# entry_yf.pack(side=tk.LEFT, padx=2)
#
# entry_zf = tk.Entry(frame_pf, font=font_entry, width=6)
# entry_zf.pack(side=tk.LEFT, padx=2)
#
# entry_tf = tk.Entry(frame_pf, font=font_entry, width=6)
# entry_tf.pack(side=tk.LEFT, padx=2)
# |||||||||||||||||||
frame_controls = tk.Frame(frame_inputs, bg="#f0f0f5")
frame_controls.grid(row=1, column=3, rowspan=9, padx=(20, 5), sticky="n")

button_home = tk.Button(frame_controls, text="SET HOME", command=set_home,
                        font=font_button, bg="#4CAF50", fg="white", width=12)
button_home.pack(pady=5)

button_send = tk.Button(frame_controls, text="SEND DATA", command=send_angles,
                        font=font_button, bg="#008CBA", fg="white", width=12)
button_send.pack(pady=5)

button_stop = tk.Button(frame_controls, text="STOP", command=stop,
                        font=font_button, bg="#f44336", fg="white", width=12)
button_stop.pack(pady=5)

button_hut = tk.Button(frame_controls, text="UP", command=hut_namcham,
                       font=font_button, bg="#009688", fg="white", width=12)
button_hut.pack(pady=5)

button_tha = tk.Button(frame_controls, text="DOWN", command=tha_namcham,
                       font=font_button, bg="#795548", fg="white", width=12)
button_tha.pack(pady=5)

# --- Trajectory inputs ---
label_traj = tk.Label(frame_controls, text="TRAJECTORY POINTS", font=("Helvetica", 15, "bold"), bg="#f0f0f5", fg="#333")
label_traj.pack(pady=(12, 5))

# P0
label_p0 = tk.Label(frame_controls, text="P0: (X0, Y0, Z0)", font=font_label, bg="#f0f0f5")
label_p0.pack()
frame_p0 = tk.Frame(frame_controls, bg="#f0f0f5")
frame_p0.pack(pady=3)
entry_x0 = tk.Entry(frame_p0, font=font_entry, width=6); entry_x0.pack(side=tk.LEFT, padx=2)
entry_y0 = tk.Entry(frame_p0, font=font_entry, width=6); entry_y0.pack(side=tk.LEFT, padx=2)
entry_z0 = tk.Entry(frame_p0, font=font_entry, width=6); entry_z0.pack(side=tk.LEFT, padx=2)

# Pf
label_pf = tk.Label(frame_controls, text="Pf: (Xf, Yf, Zf, tf)", font=font_label, bg="#f0f0f5")
label_pf.pack()
frame_pf = tk.Frame(frame_controls, bg="#f0f0f5")
frame_pf.pack(pady=3)
entry_xf = tk.Entry(frame_pf, font=font_entry, width=6); entry_xf.pack(side=tk.LEFT, padx=2)
entry_yf = tk.Entry(frame_pf, font=font_entry, width=6); entry_yf.pack(side=tk.LEFT, padx=2)
entry_zf = tk.Entry(frame_pf, font=font_entry, width=6); entry_zf.pack(side=tk.LEFT, padx=2)
entry_tf = tk.Entry(frame_pf, font=font_entry, width=6); entry_tf.pack(side=tk.LEFT, padx=2)

# |||||||||||||||||||||
# # Nút chạy quỹ đạo
# button_traj = tk.Button(frame_buttons, text="RUN TRAJECTORY", command=send_trajectory,
#                         font=font_button, bg="#FF5722", fg="white", width=15)
# button_traj.pack(pady=10, fill="x")
# ||||||||||||||||||||||||
button_traj = tk.Button(frame_inputs, text="RUN TRAJECTORY", command=send_trajectory,
                        font=font_button, bg="#FF5722", fg="white", width=15)
button_traj.grid(row=10, column=3, padx=10, pady=10, sticky="e")
# Frame chứa text box ở giữa
frame_text_center = tk.Frame(window, bg="#f0f0f5")
frame_text_center.pack(pady=20) # Thêm pady để có khoảng cách trên dưới

text_box = tk.Text(frame_text_center, font=("Courier New", 11), width=60, height=10)
text_box.pack(side=tk.LEFT, fill="both", expand=True) # fill và expand để text box lấp đầy frame

scrollbar = tk.Scrollbar(frame_text_center, command=text_box.yview)
scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
text_box.config(yscrollcommand=scrollbar.set)

# Thread đọc Serial (vẫn giữ nguyên vị trí)
serial_thread = threading.Thread(target=read_serial)
serial_thread.daemon = True
serial_thread.start()

window.mainloop()