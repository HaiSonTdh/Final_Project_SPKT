import tkinter as tk
from tkinter import messagebox
import serial
import time
import threading
import Kinematic
import cv2
from PIL import Image, ImageTk

# from PIL import Image, ImageTk, ImageDraw, ImageFont # ImageDraw, ImageFont removed for simplicity

# Setup Serial
try:
    ser = serial.Serial('COM6', 9600)
    time.sleep(1)
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    ser = None
    # messagebox.showerror("Serial Error", f"Could not open COM5: {e}\nSerial functionality will be disabled.")

# Giao diện chính
window = tk.Tk()
window.title("GUI")
window.configure(bg="#f0f0f5")
# window.geometry("1200x850") # Bạn có thể thử đặt kích thước cửa sổ cố định nếu cần

font_title = ("Arial", 20, "bold")
font_label = ("Arial", 12)
font_entry = ("Arial", 12)
font_button = ("Arial", 12, "bold")

controllable_buttons = []
mode_var = tk.StringVar(value="manual")

def toggle_mode():
    mode = mode_var.get()
    new_state = tk.DISABLED if mode == "auto" else tk.NORMAL

    if mode == "manual":
        radio_manual.config(relief=tk.SUNKEN, bg="#f4f716")
        radio_auto.config(relief=tk.RAISED, bg="#f0f0f5")
    else:  # auto mode
        radio_auto.config(relief=tk.SUNKEN, bg="#f4f716")
        radio_manual.config(relief=tk.RAISED, bg="#f0f0f5")

    for btn in controllable_buttons:
        if btn:
            try:
                btn.config(state=new_state)
            except tk.TclError:
                pass


def send_command_to_serial(command_str):
    if ser and ser.is_open:
        try:
            ser.write(command_str.encode())
            print(f"Gửi: {command_str.strip()}")
            return True
        except serial.SerialException as e:
            messagebox.showerror("Serial Error", f"Error writing to serial port: {e}")
            return False
    else:
        # messagebox.showwarning("Serial Port Error", "Serial port not available.") # Can be noisy
        print("Serial port not available for sending command.")
        return False


def send_angles():
    try:
        theta1 = float(entry_theta1.get())
        theta2 = float(entry_theta2.get())
        theta3 = float(entry_theta3.get())

        if not (0 <= theta1 <= 60):
            messagebox.showerror("Error", "Theta 1 phải trong khoảng 0 đến 45")
            return
        if not (0 <= theta2 <= 60):
            messagebox.showerror("Error", "Theta 2 phải trong khoảng 0 đến 45")
            return
        if not (0 <= theta3 <= 60):
            messagebox.showerror("Error", "Theta 3 phải trong khoảng 0 đến 45")
            return

        try:
            result = Kinematic.forward_kinematic(theta1, theta2, theta3)
            if result[0] is False:
                messagebox.showerror("Error", "Không thể tính vị trí. Kiểm tra lại góc đầu vào.")
                return

            _, x, y, z = result  # Bỏ qua giá trị đầu vì nó là True/False

            # Giới hạn tọa độ (ví dụ)
            if not (-100 <= x <= 87):
                messagebox.showerror("Lỗi", f"X={x:.2f} nằm ngoài giới hạn robot")
                return
            if not (-80 <= y <= 130):
                messagebox.showerror("Lỗi", f"Y={y:.2f} nằm ngoài giới hạn robot")
                return
            if not (-397 <= z <= -307.38):
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
        messagebox.showerror("Error", "Vui lòng nhập đúng định dạng là số!")


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
def send_trajectory():
    try:
        x0_val = float(entry_x0.get())
        y0_val = float(entry_y0.get())
        z0_val = float(entry_z0.get())
        c0_val = entry_c0.get().strip()
        xf_val = float(entry_xf.get())
        yf_val = float(entry_yf.get())
        zf_val = float(entry_zf.get())
        tf_val = float(entry_tf.get())
        if not (-100 <= x0_val <= 87):
            messagebox.showerror("Lỗi", f"X={x0_val:.2f} nằm ngoài giới hạn robot")
            return
        if not (-80 <= y0_val <= 130):
            messagebox.showerror("Lỗi", f"Y={y0_val:.2f} nằm ngoài giới hạn robot")
            return
        if not (-397 <= z0_val <= -307.38):
            messagebox.showerror("Lỗi", f"Z={z0_val:.2f} nằm ngoài giới hạn robot")
            return
        if not (-100 <= xf_val <= 87):
            messagebox.showerror("Lỗi", f"X={xf_val:.2f} nằm ngoài giới hạn robot")
            return
        if not (-80 <= yf_val <= 130):
            messagebox.showerror("Lỗi", f"Y={yf_val:.2f} nằm ngoài giới hạn robot")
            return
        if not (-397 <= zf_val <= -307.38):
            messagebox.showerror("Lỗi", f"Z={zf_val:.2f} nằm ngoài giới hạn robot")
            return
        base_data_segment = f"P0:{x0_val},{y0_val},{z0_val};Pf:{xf_val},{yf_val},{zf_val};T:{tf_val}"
        data_to_send = ""

        if c0_val:
            if len(c0_val) > 1: c0_val = c0_val[0]
            data_to_send = f"{base_data_segment};C:{c0_val}\r"
        else:
            data_to_send = f"{base_data_segment}\r"

        if send_command_to_serial(data_to_send):
            entry_x0.delete(0, tk.END);
            entry_x0.insert(0, str(xf_val))
            entry_y0.delete(0, tk.END);
            entry_y0.insert(0, str(yf_val))
            entry_z0.delete(0, tk.END);
            entry_z0.insert(0, str(zf_val))
            entry_xf.delete(0, tk.END);
            entry_yf.delete(0, tk.END);
            entry_zf.delete(0, tk.END)
            entry_tf.delete(0, tk.END);
            entry_c0.delete(0, tk.END)

    except ValueError:
        messagebox.showerror("Lỗi", "Vui lòng nhập giá trị số hợp lệ cho tọa độ và thời gian.")
    except Exception as e:
        messagebox.showerror("Lỗi", f"Đã xảy ra lỗi: {str(e)}")


def set_home():
    if send_command_to_serial('h\r'):
        try:
            result = Kinematic.forward_kinematic(0, 0, 0)
            if result and result[0]:
                _, x, y, z = result
                entry_x.config(state='normal');
                entry_y.config(state='normal');
                entry_z.config(state='normal')
                entry_x.delete(0, tk.END);
                entry_x.insert(0, f"{x:.2f}")
                entry_y.delete(0, tk.END);
                entry_y.insert(0, f"{y:.2f}")
                entry_z.delete(0, tk.END);
                entry_z.insert(0, f"{z:.2f}")
                entry_x.config(state='readonly');
                entry_y.config(state='readonly');
                entry_z.config(state='readonly')
        except Exception as e:
            print(f"Error calculating home position kinematics: {e}")


def stop_robot(): send_command_to_serial('s\r')


def move_z_plus(): send_command_to_serial('z\r')


def move_z_minus(): send_command_to_serial('c\r')


def move_y_plus(): send_command_to_serial('y\r')


def move_y_minus(): send_command_to_serial('i\r')


def move_x_plus(): send_command_to_serial('x\r')


def move_x_minus(): send_command_to_serial('v\r')

def start_all(): send_command_to_serial('j\r')
def read_serial():
    while True:
        if ser and ser.is_open and ser.in_waiting > 0:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').rstrip()
                if line:
                    print(f"Nhận: {line}")
                    window.after(0, lambda l=line: (
                        text_box.insert(tk.END, l + "\n"),
                        text_box.see(tk.END)
                    ))
            except serial.SerialException as e:
                print(f"Serial read error: {e}");
                break
            except Exception as e:
                print(f"Error processing serial data: {e}")
        time.sleep(0.1)


cap = None
camera_running = False

def start_camera():
    global cap, camera_running
    if not camera_running:
        cap = cv2.VideoCapture(0)
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
        camera_running = True
        update_frame()  # Bắt đầu cập nhật khung hình

def stop_camera_stream():
    global cap, camera_running
    camera_running = False
    if cap is not None:
        cap.release()
    label_cam.config(image='', bg="black")  # Xóa ảnh, giữ khung đen


def update_frame():
    global cap, camera_running
    if camera_running and cap.isOpened():
        ret, frame = cap.read()
        if ret:
            frame = cv2.resize(frame, (720, 370))  # Đảm bảo đúng kích thước
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            img = Image.fromarray(frame)
            imgtk = ImageTk.PhotoImage(image=img)
            label_cam.imgtk = imgtk  # Giữ tham chiếu
            label_cam.config(image=imgtk)
        label_cam.after(10, update_frame)  # Gọi lại sau 10ms

def emergency_stop(): send_command_to_serial('w\r')


magnet_state = 0


def toggle_namcham():
    global magnet_state
    cmd = 'u\r' if magnet_state == 0 else 'd\r'
    if send_command_to_serial(cmd):
        if magnet_state == 0:
            btn_namcham.config(text="ON MAG", bg="#3bd952");
            magnet_state = 1
        else:
            btn_namcham.config(text="OFF MAG", bg="#eb3b3b");
            magnet_state = 0


conveyor_state = 0


def toggle_conveyor():
    global conveyor_state
    cmd = 'w\r' if conveyor_state == 0 else 'z\r'  # Ensure 'w' and 'z' are correct for conveyor
    if send_command_to_serial(cmd):
        if conveyor_state == 0:
            btn_bangtai.config(text="ON CONV", bg="#3bd952");
            conveyor_state = 1
        else:
            btn_bangtai.config(text="OFF CONV", bg="#eb3b3b");
            conveyor_state = 0


# --- GUI LAYOUT DEFINITION ---

# Section 1: Header Image
label_image = tk.Label(window, bg="#f0f0f5")
try:
    image_path = "Banner0.png"
    try:
        image = Image.open(image_path)
    except FileNotFoundError:
        print(f"Warning: Image file '{image_path}' not found. Using placeholder.")
        image = Image.new('RGB', (850, 100), color='skyblue')  # Simpler placeholder
    image = image.resize((850, 100))
    photo = ImageTk.PhotoImage(image)
    label_image.config(image=photo)
    label_image.image = photo
except Exception as e:
    print(f"Lỗi khi tải ảnh banner: {e}")
    # label_image might remain empty or you can set a text placeholder

# Section 2: Title
title = tk.Label(window, text="ROBOT DELTA", font=font_title, bg="#f0f0f5", fg="#333")

# Section 3: Mode Selection (MANUAL/AUTO)
frame_mode_selection_container = tk.Frame(window, bg="#f0f0f5")
frame_mode_selection = tk.Frame(frame_mode_selection_container, bg="#f0f0f5")
radio_manual = tk.Radiobutton(frame_mode_selection, text="MANUAL", variable=mode_var, value="manual",
                              command=toggle_mode, activebackground="#c0e0c0", indicatoron=0, width=10,
                              font=font_button, relief=tk.RAISED, bd=2)
radio_auto = tk.Radiobutton(frame_mode_selection, text="AUTO", variable=mode_var, value="auto",
                            command=toggle_mode, activebackground="#ffe0b0", indicatoron=0, width=10, font=font_button,
                            relief=tk.RAISED, bd=2)
radio_manual.pack(side=tk.LEFT, padx=(0, 5))
radio_auto.pack(side=tk.LEFT, padx=5)
frame_mode_selection.pack(side=tk.LEFT) # Sẽ pack vào frame_mode_selection_container sau

# Section 4: Bottom Bar (Text Box and Control Buttons) - DEFINE IT BEFORE MAIN CONTENT FOR PACKING ORDER
bottom_bar_frame = tk.Frame(window, bg="#f0f0f5")

frame_text_bottom_left = tk.Frame(bottom_bar_frame, bg="#f0f0f5")
text_box = tk.Text(frame_text_bottom_left, font=("Courier New", 11), width=60, height=5) # Giữ height=5
scrollbar = tk.Scrollbar(frame_text_bottom_left, command=text_box.yview)
text_box.config(yscrollcommand=scrollbar.set)
text_box.pack(side=tk.LEFT, fill="both", expand=True)
scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
frame_text_bottom_left.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(0, 5))  # Text box on right

frame_control_buttons_bottom = tk.Frame(bottom_bar_frame, bg="#f0f0f5")
col1 = tk.Frame(frame_control_buttons_bottom, bg="#f0f0f5");
col1.grid(row=0, column=0, padx=5)
btn_startall = tk.Button(col1, text="START", command=start_all, font=font_button, bg="#4CAF50", fg="white", width=10);
btn_startall.pack(pady=2, fill=tk.X) # Giảm pady
btn_stop = tk.Button(col1, text="STOP", command=stop_robot, font=font_button, bg="#f44336", fg="white", width=10);
btn_stop.pack(pady=2, fill=tk.X) # Giảm pady
btn_emg = tk.Button(col1, text="EMG", command=emergency_stop, font=font_button, bg="#B71C1C", fg="white", width=10);
btn_emg.pack(pady=2, fill=tk.X) # Giảm pady

col2 = tk.Frame(frame_control_buttons_bottom, bg="#f0f0f5");
col2.grid(row=0, column=1, padx=5)
btn_z_plus = tk.Button(col2, text="Z+", command=move_z_plus, font=font_button, bg="#b5b0a7", fg="black", width=8);
btn_z_plus.pack(pady=2, fill=tk.X) # Giảm pady
btn_z_minus = tk.Button(col2, text="Z-", command=move_z_minus, font=font_button, bg="#b5b0a7", fg="black", width=8);
btn_z_minus.pack(pady=2, fill=tk.X) # Giảm pady
btn_y_plus = tk.Button(col2, text="Y+", command=move_y_plus, font=font_button, bg="#b5b0a7", fg="black", width=8);
btn_y_plus.pack(pady=2, fill=tk.X) # Giảm pady

col3 = tk.Frame(frame_control_buttons_bottom, bg="#f0f0f5");
col3.grid(row=0, column=2, padx=5)
btn_x_minus = tk.Button(col3, text="X-", command=move_x_minus, font=font_button, bg="#b5b0a7", fg="black", width=8);
btn_x_minus.pack(pady=2, fill=tk.X) # Giảm pady
btn_x_plus = tk.Button(col3, text="X+", command=move_x_plus, font=font_button, bg="#b5b0a7", fg="black", width=8);
btn_x_plus.pack(pady=2, fill=tk.X) # Giảm pady
btn_y_minus = tk.Button(col3, text="Y-", command=move_y_minus, font=font_button, bg="#b5b0a7", fg="black", width=8);
btn_y_minus.pack(pady=2, fill=tk.X) # Giảm pady

col4 = tk.Frame(frame_control_buttons_bottom, bg="#f0f0f5");
col4.grid(row=0, column=3, padx=5)
btn_home = tk.Button(col4, text="SET HOME", command=set_home, font=font_button, bg="#edaa1a", fg="white", width=8);
btn_home.pack(pady=2, fill=tk.X) # Giảm pady
btn_namcham = tk.Button(col4, text="OFF MAG", command=toggle_namcham, font=font_button, bg="#eb3b3b", fg="white",
                        width=8);
btn_namcham.pack(pady=2, fill=tk.X) # Giảm pady
btn_bangtai = tk.Button(col4, text="OFF CONV", command=toggle_conveyor, font=font_button, bg="#eb3b3b", fg="white",
                        width=8);
btn_bangtai.pack(pady=2, fill=tk.X) # Giảm pady
frame_control_buttons_bottom.pack(side=tk.LEFT, padx=(5, 0))  # Control buttons on left

# Section 5: Main Content Area (Kinematics, Trajectory, Camera)
frame_main = tk.Frame(window, bg="#f0f0f5")

# Main Content -> Left Side (Inputs: Forward/Inverse Kinematics, Trajectory)
frame_inputs = tk.Frame(frame_main, bg="#f0f0f5")
# Forward Kinematics
label_for_kinematic = tk.Label(frame_inputs, text="FORWARD KINEMATIC", font=("Helvetica", 17, "bold"), bg="#f0f0f5", fg="#333")
label_for_kinematic.grid(row=0, column=0, columnspan=3, padx=10, pady=(10, 5), sticky="w")
label_theta1 = tk.Label(frame_inputs, text="Theta 1 (°):", font=font_label, bg="#f0f0f5"); label_theta1.grid(row=1, column=0, padx=10, pady=5, sticky="w")
entry_theta1 = tk.Entry(frame_inputs, font=font_entry, width=10); entry_theta1.grid(row=1, column=1, pady=5)
label_theta2 = tk.Label(frame_inputs, text="Theta 2 (°):", font=font_label, bg="#f0f0f5"); label_theta2.grid(row=2, column=0, padx=10, pady=5, sticky="w")
entry_theta2 = tk.Entry(frame_inputs, font=font_entry, width=10); entry_theta2.grid(row=2, column=1, pady=5)
btn_run = tk.Button(frame_inputs, text="RUN", command=send_angles, font=font_button, bg="#b5b0a7", fg="black", width=8)
btn_run.grid(row=2, column=2, padx=5)
label_theta3 = tk.Label(frame_inputs, text="Theta 3 (°):", font=font_label, bg="#f0f0f5"); label_theta3.grid(row=3, column=0, padx=10, pady=5, sticky="w")
entry_theta3 = tk.Entry(frame_inputs, font=font_entry, width=10); entry_theta3.grid(row=3, column=1, pady=5)
# Position Display
label_result = tk.Label(frame_inputs, text="POSITION (mm):", font=("Helvetica", 17, "bold"), bg="#f0f0f5"); label_result.grid(row=4, column=0, columnspan=3, padx=10, pady=(15, 5), sticky="w") # Giảm pady top
frame_xyz = tk.Frame(frame_inputs, bg="#f0f0f5"); frame_xyz.grid(row=5, column=0, columnspan=3, padx=10, pady=5, sticky="w")
label_x = tk.Label(frame_xyz, text="X:", font=font_label, bg="#f0f0f5"); label_x.pack(side=tk.LEFT, padx=(0, 2))
entry_x = tk.Entry(frame_xyz, font=font_entry, width=8, state='readonly'); entry_x.pack(side=tk.LEFT, padx=(0, 10))
label_y = tk.Label(frame_xyz, text="Y:", font=font_label, bg="#f0f0f5"); label_y.pack(side=tk.LEFT, padx=(0, 2))
entry_y = tk.Entry(frame_xyz, font=font_entry, width=8, state='readonly'); entry_y.pack(side=tk.LEFT, padx=(0, 10))
label_z = tk.Label(frame_xyz, text="Z:", font=font_label, bg="#f0f0f5"); label_z.pack(side=tk.LEFT, padx=(0, 2))
entry_z = tk.Entry(frame_xyz, font=font_entry, width=8, state='readonly'); entry_z.pack(side=tk.LEFT, padx=(0, 10))
# Inverse Kinematics
label_inv_kinematic = tk.Label(frame_inputs, text="INVERSE KINEMATIC", font=("Helvetica", 17, "bold"), bg="#f0f0f5", fg="#333"); label_inv_kinematic.grid(row=6, column=0, columnspan=3, padx=10, pady=(15, 5), sticky="w") # Giảm pady top
frame_x_ik = tk.Frame(frame_inputs, bg="#f0f0f5"); frame_x_ik.grid(row=7, column=0, columnspan=3, padx=10, pady=2, sticky="w") # Giảm pady
label_x_ik = tk.Label(frame_x_ik, text="X:", font=font_label, bg="#f0f0f5"); label_x_ik.pack(side=tk.LEFT, padx=(0, 2))
entry_x_ik = tk.Entry(frame_x_ik, font=font_entry, width=8); entry_x_ik.pack(side=tk.LEFT, padx=(0, 10))
label_theta1_ik = tk.Label(frame_x_ik, text="Theta1:", font=font_label, bg="#f0f0f5"); label_theta1_ik.pack(side=tk.LEFT, padx=(0, 4))
entry_theta1_ik = tk.Entry(frame_x_ik, font=font_entry, width=8, state='readonly'); entry_theta1_ik.pack(side=tk.LEFT, padx=(0, 10))
frame_y_ik = tk.Frame(frame_inputs, bg="#f0f0f5"); frame_y_ik.grid(row=8, column=0, columnspan=3, padx=10, pady=2, sticky="w") # Giảm pady
label_y_ik = tk.Label(frame_y_ik, text="Y:", font=font_label, bg="#f0f0f5"); label_y_ik.pack(side=tk.LEFT, padx=(0, 2))
entry_y_ik = tk.Entry(frame_y_ik, font=font_entry, width=8); entry_y_ik.pack(side=tk.LEFT, padx=(0, 10))
label_theta2_ik = tk.Label(frame_y_ik, text="Theta2:", font=font_label, bg="#f0f0f5"); label_theta2_ik.pack(side=tk.LEFT, padx=(0, 4))
entry_theta2_ik = tk.Entry(frame_y_ik, font=font_entry, width=8, state='readonly'); entry_theta2_ik.pack(side=tk.LEFT, padx=(0, 10))
frame_z_ik = tk.Frame(frame_inputs, bg="#f0f0f5"); frame_z_ik.grid(row=9, column=0, columnspan=3, padx=10, pady=2, sticky="w") # Giảm pady
label_z_ik = tk.Label(frame_z_ik, text="Z:", font=font_label, bg="#f0f0f5"); label_z_ik.pack(side=tk.LEFT, padx=(0, 2))
entry_z_ik = tk.Entry(frame_z_ik, font=font_entry, width=8); entry_z_ik.pack(side=tk.LEFT, padx=(0, 10))
label_theta3_ik = tk.Label(frame_z_ik, text="Theta3:", font=font_label, bg="#f0f0f5"); label_theta3_ik.pack(side=tk.LEFT, padx=(0, 4))
entry_theta3_ik = tk.Entry(frame_z_ik, font=font_entry, width=8, state='readonly'); entry_theta3_ik.pack(side=tk.LEFT, padx=(0, 10))

# *** SỬA Ở ĐÂY ***
# Giảm pady và ipady cho nút CAL IK
btn_calc_ik = tk.Button(frame_inputs, text="CAL IK", command=calculate_inv_kinematic, font=font_button, bg="#b5b0a7", fg="black", width=12)
btn_calc_ik.grid(row=10, column=0, columnspan=2, pady=(5, 7), sticky="w", padx=10, ipady=1) # pady=(top, bottom), giảm ipady

# Trajectory Points
label_traj = tk.Label(frame_inputs, text="TRAJECTORY POINTS", font=("Helvetica", 17, "bold"), bg="#f0f0f5", fg="#333")
label_traj.grid(row=6, column=3, columnspan=3, padx=10, pady=(15, 5), sticky="w") # Giảm pady top
frame_controls = tk.Frame(frame_inputs, bg="#f0f0f5"); frame_controls.grid(row=7, column=3, columnspan=3, rowspan=3, padx=(10, 5), pady= (0,2), sticky="nw") # Giảm pady bottom
label_p0 = tk.Label(frame_controls, text="P0: (X0, Y0, Z0) | C0", font=font_label, bg="#f0f0f5"); label_p0.pack(anchor="w", padx=5)
frame_p0 = tk.Frame(frame_controls, bg="#f0f0f5"); frame_p0.pack(pady=2, anchor="w", padx=5) # Giảm pady
entry_x0 = tk.Entry(frame_p0, font=font_entry, width=7, justify='center'); entry_x0.pack(side=tk.LEFT, padx=3); entry_x0.insert(0, "0.0")
entry_y0 = tk.Entry(frame_p0, font=font_entry, width=7, justify='center'); entry_y0.pack(side=tk.LEFT, padx=3); entry_y0.insert(0, "0.0")
entry_z0 = tk.Entry(frame_p0, font=font_entry, width=7, justify='center'); entry_z0.pack(side=tk.LEFT, padx=3); entry_z0.insert(0, "0.0")
entry_c0 = tk.Entry(frame_p0, font=font_entry, width=7, justify='center'); entry_c0.pack(side=tk.LEFT, padx=3)
label_pf = tk.Label(frame_controls, text="Pf: (Xf, Yf, Zf) | tf", font=font_label, bg="#f0f0f5"); label_pf.pack(anchor="w", padx=5)
frame_pf = tk.Frame(frame_controls, bg="#f0f0f5"); frame_pf.pack(pady=2, anchor="w", padx=5) # Giảm pady
entry_xf = tk.Entry(frame_pf, font=font_entry, width=7, justify='center'); entry_xf.pack(side=tk.LEFT, padx=3)
entry_yf = tk.Entry(frame_pf, font=font_entry, width=7, justify='center'); entry_yf.pack(side=tk.LEFT, padx=3)
entry_zf = tk.Entry(frame_pf, font=font_entry, width=7, justify='center'); entry_zf.pack(side=tk.LEFT, padx=3)
entry_tf = tk.Entry(frame_pf, font=font_entry, width=7, justify='center'); entry_tf.pack(side=tk.LEFT, padx=3)

# *** SỬA Ở ĐÂY ***
# Giảm pady và ipady cho nút RUN TRAJECTORY
button_traj = tk.Button(frame_inputs, text="RUN TRAJECTORY", command=send_trajectory, font=font_button, bg="#b5b0a7", fg="black", width=15)
button_traj.grid(row=10, column=3, columnspan=3, padx=10, pady=(5, 7), sticky="w", ipady=1) # pady=(top, bottom), giảm ipady

frame_inputs.pack(side=tk.LEFT, fill="y", padx=(0,10))

# Main Content -> Right Side (Camera)
frame_right_zone = tk.Frame(frame_main, bg="#f0f0f5")
frame_right_zone.pack(side=tk.RIGHT, fill="both", expand=True, padx=10) # expand True để frame này cũng cố gắng chiếm không gian

# Đặt frame_camera vào trong frame_right_zone
frame_camera = tk.Frame(frame_right_zone, bg="#e0e0e0", bd=2, relief=tk.SUNKEN)
# Pack frame_camera để nó mở rộng
frame_camera.pack(pady=(0, 5), fill="both", expand=True)
label_cam = tk.Label(frame_camera, bg="black")  # Placeholder size
# Pack label_cam để nó mở rộng và căn giữa
label_cam.pack(padx=10, pady=(10, 5), anchor="center", fill="both", expand=True)

frame_cam_buttons = tk.Frame(frame_camera, bg="#e0e0e0")
btn_start_cam = tk.Button(frame_cam_buttons, text="START CAMERA", command=start_camera, font=font_button, bg="#4CAF50",
                          fg="white", width=16)
btn_start_cam.pack(side=tk.LEFT, padx=5, pady=5) # Thêm pady cho nút camera
btn_stop_cam = tk.Button(frame_cam_buttons, text="STOP CAMERA", command=stop_camera_stream, font=font_button,
                         bg="#f44336", fg="white", width=16)
btn_stop_cam.pack(side=tk.LEFT, padx=5, pady=5) # Thêm pady cho nút camera
frame_cam_buttons.pack(side=tk.BOTTOM, anchor="center", padx=10, pady=(5,10)) # pady dưới cùng


# --- PACKING THE MAIN LAYOUT SECTIONS INTO THE WINDOW ---
# Order is important: top fixed, bottom fixed, then central expanding
label_image.pack(side=tk.TOP, fill=tk.X, pady=(10, 5))
title.pack(side=tk.TOP, pady=(5,10)) # Giảm pady trên của title

# *** SỬA Ở ĐÂY ***
# Giảm pady cho frame_mode_selection_container
frame_mode_selection_container.pack(side=tk.TOP, fill=tk.X, padx=20, pady=(0, 5))

# *** SỬA Ở ĐÂY ***
# Giảm pady cho bottom_bar_frame
bottom_bar_frame.pack(side=tk.BOTTOM, fill=tk.X, padx=10, pady=(5, 5))

frame_main.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=20, pady=0)

# --- FINAL SETUP ---
controllable_buttons.extend([
    btn_run, button_traj, btn_namcham, btn_bangtai,
    btn_z_plus, btn_z_minus, btn_y_plus, btn_y_minus,
    btn_x_plus, btn_x_minus, btn_calc_ik # Thêm btn_calc_ik vào đây nếu nó cũng bị disable ở mode auto
])
toggle_mode()

if ser:
    serial_thread = threading.Thread(target=read_serial, daemon=True)
    serial_thread.start()
else:
    text_box.insert(tk.END, "Serial port (COM5) not available. Check connection.\n")

window.mainloop()

if ser and ser.is_open:
    ser.close()