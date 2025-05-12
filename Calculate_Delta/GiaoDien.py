import tkinter as tk
from tkinter import messagebox
import serial
import time
import threading
import Kinematic
from PIL import Image, ImageTk

# Setup Serial
# ser = serial.Serial('COM5', 9600)
# time.sleep(1)

# Hàm điều khiển Arduino
def send_angles():
    try:
        theta1 = float(entry_theta1.get())
        theta2 = float(entry_theta2.get())
        theta3 = float(entry_theta3.get())

        if not (0 <= theta1 <= 60):
            messagebox.showerror("Error", "Theta 1 phải trong khoảng 0 đến 90")
            return
        if not (0 <= theta2 <= 60):
            messagebox.showerror("Error", "Theta 2 phải trong khoảng 0 đến 45")
            return
        if not (0 <= theta3 <= 60):
            messagebox.showerror("Error", "Theta 3 phải trong khoảng 0 đến 60")
            return

        data = f"{theta1}A{theta2}B{theta3}C\r"
        print(f"Gửi: {data.strip()}")
        ser.write(data.encode())

    except ValueError:
        messagebox.showerror("Error", "Vui lòng nhập đúng định dạng số!")
    try:
        result = Kinematic.forward_kinematic(theta1, theta2, theta3)
        if result[0] is False:
            messagebox.showerror("Error", "Không thể tính vị trí. Kiểm tra lại góc đầu vào.")
            return

        _, x, y, z = result # bỏ qua giá trị đầu vì nó là true, false

        entry_x.config(state='normal')             #cho phép thay đổi dữ liệu
        entry_y.config(state='normal')
        entry_z.config(state='normal')
        entry_x.delete(0, tk.END)             #xóa dữ liệu cũ
        entry_y.delete(0, tk.END)
        entry_z.delete(0, tk.END)
        entry_x.insert(0, f"{x:.2f}")  #ghi dữ liệu mới
        entry_y.insert(0, f"{y:.2f}")
        entry_z.insert(0, f"{z:.2f}")
        entry_x.config(state='readonly')      #khóa lại để người dùng k sửa đc
        entry_y.config(state='readonly')
        entry_z.config(state='readonly')
    except Exception as e:
        print(f"Lỗi khi tính forward kinematic: {e}")
        messagebox.showerror("Lỗi", "Không thể tính vị trí. Kiểm tra lại hàm forward_kinematic.")

def calculate_inv_kinematic():
    try:
        x_val = float(entry_input_x.get())
        y_val = float(entry_input_y.get())
        z_val = float(entry_input_z.get())

        angles = Kinematic.inverse_kinematic(x_val, y_val, z_val)

        entry_out1.config(state=tk.NORMAL)
        entry_out1.delete(0, tk.END)
        entry_out1.insert(0, f"{angles[0]:.2f}")
        entry_out1.config(state='readonly')

        entry_out2.config(state=tk.NORMAL)
        entry_out2.delete(0, tk.END)
        entry_out2.insert(0, f"{angles[1]:.2f}")
        entry_out2.config(state='readonly')

        entry_out3.config(state=tk.NORMAL)
        entry_out3.delete(0, tk.END)
        entry_out3.insert(0, f"{angles[2]:.2f}")
        entry_out3.config(state='readonly')

    except ValueError:
        messagebox.showerror("Lỗi", "Vui lòng nhập giá trị số hợp lệ cho X, Y, Z.")
    except ValueError as e:
        messagebox.showerror("Lỗi tính toán", str(e))
    except Exception as e:
        messagebox.showerror("Lỗi không xác định", f"Đã xảy ra lỗi: {e}")

def set_home():
    ser.write(bytes('h' + '\r', 'utf-8'))
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
        ser.write(f"{val:.2f}A\r".encode())
        print(f"Gửi: {val:.2f}A")
    except ValueError:
        messagebox.showerror("Error", "Theta 1 không hợp lệ!")

def send_theta2():
    try:
        val = float(entry_theta2.get())
        ser.write(f"{val:.2f}B\r".encode())
        print(f"Gửi: {val:.2f}B")
    except ValueError:
        messagebox.showerror("Error", "Theta 2 không hợp lệ!")

def send_theta3():
    try:
        val = float(entry_theta3.get())
        ser.write(f"{val:.2f}C\r".encode())
        print(f"Gửi: {val:.2f}C")
    except ValueError:
        messagebox.showerror("Error", "Theta 3 không hợp lệ!")


# Giao diện chính
window = tk.Tk()
window.title("Gửi góc điều khiển tới Arduino")
window.geometry("850x750")
window.configure(bg="#f0f0f5")  # Màu nền nhẹ nhàng

font_title = ("Arial", 20, "bold")
font_label = ("Arial", 12)
font_entry = ("Arial", 12)
font_button = ("Arial", 12, "bold")

# Hiển thị ảnh khoa đào tạo
try:
    image = Image.open("Banner0.png")  # Thay bằng tên file ảnh của bạn
    image = image.resize((550, 100))  # Resize ảnh nếu cần
    photo = ImageTk.PhotoImage(image)
    label_image = tk.Label(window, image=photo, bg="#f0f0f5")
    label_image.image = photo  # Giữ tham chiếu ảnh
    label_image.pack(pady=(10, 5))
except Exception as e:
    print(f"Lỗi khi tải ảnh: {e}")
title = tk.Label(window, text="CONTROL PANEL", font=font_title, bg="#f0f0f5", fg="#333")
title.pack(pady=10)

# Frame chính chứa các phần bên trái và bên phải
frame_main = tk.Frame(window, bg="#f0f0f5")
frame_main.pack(pady=10, padx=20, fill="x") # Thêm padx và fill để frame chính rộng hơn

# Inputs bên trái
frame_inputs = tk.Frame(frame_main, bg="#f0f0f5")
frame_inputs.pack(side=tk.LEFT, fill="y") # fill="y" để frame inputs cao bằng frame buttons

label_for_kinematic = tk.Label(frame_inputs, text="FORWARD KINEMATIC", font=font_label, bg="#f0f0f5", fg="#333")
label_for_kinematic.grid(row=0, column=0, columnspan=3, padx=10, pady=(20, 5), sticky="w")

# Theta 1
label_theta1 = tk.Label(frame_inputs, text="Theta 1 (°):", font=font_label, bg="#f0f0f5")
label_theta1.grid(row=1, column=0, padx=10, pady=5, sticky="w")
entry_theta1 = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_theta1.grid(row=1, column=1, pady=5)
button_t1 = tk.Button(frame_inputs, text="Gửi", command=send_theta1, font=font_button, bg="#9C27B0", fg="white", width=6)
button_t1.grid(row=1, column=2, padx=5)

# Theta 2
label_theta2 = tk.Label(frame_inputs, text="Theta 2 (°):", font=font_label, bg="#f0f0f5")
label_theta2.grid(row=2, column=0, padx=10, pady=5, sticky="w")
entry_theta2 = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_theta2.grid(row=2, column=1, pady=5)
button_t2 = tk.Button(frame_inputs, text="Gửi", command=send_theta2, font=font_button, bg="#FF9800", fg="white", width=6)
button_t2.grid(row=2, column=2, padx=5)

# Theta 3
label_theta3 = tk.Label(frame_inputs, text="Theta 3 (°):", font=font_label, bg="#f0f0f5")
label_theta3.grid(row=3, column=0, padx=10, pady=5, sticky="w")
entry_theta3 = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_theta3.grid(row=3, column=1, pady=5)
button_t3 = tk.Button(frame_inputs, text="Gửi", command=send_theta3, font=font_button, bg="#3F51B5", fg="white", width=6)
button_t3.grid(row=3, column=2, padx=5)

# Nhãn "Vị trí (mm):"
label_result = tk.Label(frame_inputs, text="Tọa độ (mm):", font=font_label, bg="#f0f0f5")
label_result.grid(row=4, column=0, columnspan=3, padx=10, pady=(20, 5), sticky="w")

# X, Y, Z nằm ngang hàng và canh đều
label_x = tk.Label(frame_inputs, text="X:", font=font_label, bg="#f0f0f5")
label_x.grid(row=5, column=0, padx=(10, 2), pady=5, sticky="e")
entry_x = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
entry_x.grid(row=5, column=1, padx=(0, 20), pady=5) # Tăng padx bên phải

label_y = tk.Label(frame_inputs, text="Y:", font=font_label, bg="#f0f0f5")
label_y.grid(row=5, column=2, padx=(20, 2), pady=5, sticky="e") # Tăng padx bên trái
entry_y = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
entry_y.grid(row=5, column=3, padx=(0, 20), pady=5) # Tăng padx bên phải

label_z = tk.Label(frame_inputs, text="Z:", font=font_label, bg="#f0f0f5")
label_z.grid(row=5, column=4, padx=(20, 2), pady=5, sticky="e") # Tăng padx bên trái
entry_z = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
entry_z.grid(row=5, column=5, padx=(0, 10), pady=5)

label_inv_kinematic = tk.Label(frame_inputs, text="INVERSE KINEMATIC", font=font_label, bg="#f0f0f5", fg="#333")
label_inv_kinematic.grid(row=6, column=0, columnspan=3, padx=10, pady=(20, 5), sticky="w")

# Nhập X, Y, Z (Inverse Kinematic)
label_input_x = tk.Label(frame_inputs, text="X:", font=font_label, bg="#f0f0f5")
label_input_x.grid(row=7, column=0, padx=10, pady=5, sticky="w")
entry_input_x = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_input_x.grid(row=7, column=1, padx=5, pady=5)

label_input_y = tk.Label(frame_inputs, text="Y:", font=font_label, bg="#f0f0f5")
label_input_y.grid(row=8, column=0, padx=10, pady=5, sticky="w")
entry_input_y = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_input_y.grid(row=8, column=1, padx=5, pady=5)

label_input_z = tk.Label(frame_inputs, text="Z:", font=font_label, bg="#f0f0f5")
label_input_z.grid(row=9, column=0, padx=10, pady=5, sticky="w")
entry_input_z = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_input_z.grid(row=9, column=1, padx=5, pady=5)

# Nút tính toán Inverse Kinematic
btn_calc_ik = tk.Button(
    frame_inputs, text="Tính IK", command=calculate_inv_kinematic,
    font=font_button, bg="#607D8B", fg="white", width=12
)
btn_calc_ik.grid(row=10, column=0, columnspan=2, pady=10)

# Hiển thị kết quả Inverse Kinematic
label_out1 = tk.Label(frame_inputs, text="Theta 1:", font=font_label, bg="#f0f0f5")
label_out1.grid(row=7, column=2, padx=5, pady=5, sticky="w")
entry_out1 = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
entry_out1.grid(row=7, column=3, padx=(0, 10), pady=5)

label_out2 = tk.Label(frame_inputs, text="Theta 2:", font=font_label, bg="#f0f0f5")
label_out2.grid(row=8, column=2, padx=5, pady=5, sticky="w")
entry_out2 = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
entry_out2.grid(row=8, column=3, padx=(0, 10), pady=5)

label_out3 = tk.Label(frame_inputs, text="Theta 3:", font=font_label, bg="#f0f0f5")
label_out3.grid(row=9, column=2, padx=5, pady=5, sticky="w")
entry_out3 = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
entry_out3.grid(row=9, column=3, padx=(0, 10), pady=5)

# # Hiển thị kết quả Inverse Kinematic
# label_out1 = tk.Label(frame_inputs, text="Theta 1:", font=font_label, bg="#f0f0f5")
# label_out1.grid(row=7, column=2, padx=5, pady=5, sticky="w")
# entry_out1 = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
# entry_out1.grid(row=7, column=3, padx=(0, 10), pady=5)
#
# label_out2 = tk.Label(frame_inputs, text="Theta 2:", font=font_label, bg="#f0f0f5")
# label_out2.grid(row=8, column=2, padx=5, pady=5, sticky="w")
# entry_out2 = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
# entry_out2.grid(row=8, column=3, padx=(0, 10), pady=5)
#
# label_out3 = tk.Label(frame_inputs, text="Theta 3:", font=font_label, bg="#f0f0f5")
# label_out3.grid(row=9, column=2, padx=5, pady=5, sticky="w")
# entry_out3 = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
# entry_out3.grid(row=9, column=3, padx=(0, 10), pady=5)


# Nút điều khiển bên phải
frame_buttons = tk.Frame(frame_main, bg="#f0f0f5")
frame_buttons.pack(side=tk.RIGHT, padx=20, fill="y") # fill="y" để frame buttons cao bằng frame inputs

button_home = tk.Button(frame_buttons, text="SET HOME", command=set_home,
                        font=font_button, bg="#4CAF50", fg="white", width=15)
button_home.pack(pady=5, fill="x") # fill="x" để nút rộng bằng frame

button_send = tk.Button(frame_buttons, text="SEND DATA", command=send_angles,
                        font=font_button, bg="#008CBA", fg="white", width=15)
button_send.pack(pady=5, fill="x")

button_stop = tk.Button(frame_buttons, text="STOP", command=stop,
                        font=font_button, bg="#f44336", fg="white", width=15)
button_stop.pack(pady=5, fill="x")

button_hut = tk.Button(frame_buttons, text="UP", command=hut_namcham,
                       font=font_button, bg="#009688", fg="white", width=15)
button_hut.pack(pady=5, fill="x")

button_tha = tk.Button(frame_buttons, text="DOWN", command=tha_namcham,
                       font=font_button, bg="#795548", fg="white", width=15)
button_tha.pack(pady=5, fill="x")

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

window.mainloop()# Tiêu đề (nằm ngay dưới ảnh)
title = tk.Label(window, text="CONTROL PANEL", font=font_title, bg="#f0f0f5", fg="#333")
title.pack(pady=10)

# Frame chính chứa các phần bên trái và bên phải
frame_main = tk.Frame(window, bg="#f0f0f5")
frame_main.pack(pady=10, padx=20, fill="x") # Thêm padx và fill để frame chính rộng hơn

# Inputs bên trái
frame_inputs = tk.Frame(frame_main, bg="#f0f0f5")
frame_inputs.pack(side=tk.LEFT, fill="y") # fill="y" để frame inputs cao bằng frame buttons

label_for_kinematic = tk.Label(frame_inputs, text="FORWARD KINEMATIC", font=font_label, bg="#f0f0f5", fg="#333")
label_for_kinematic.grid(row=0, column=0, columnspan=3, padx=10, pady=(20, 5), sticky="w")

# Theta 1
label_theta1 = tk.Label(frame_inputs, text="Theta 1 (°):", font=font_label, bg="#f0f0f5")
label_theta1.grid(row=1, column=0, padx=10, pady=5, sticky="w")
entry_theta1 = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_theta1.grid(row=1, column=1, pady=5)
button_t1 = tk.Button(frame_inputs, text="Gửi", command=send_theta1, font=font_button, bg="#9C27B0", fg="white", width=6)
button_t1.grid(row=1, column=2, padx=5)

# Theta 2
label_theta2 = tk.Label(frame_inputs, text="Theta 2 (°):", font=font_label, bg="#f0f0f5")
label_theta2.grid(row=2, column=0, padx=10, pady=5, sticky="w")
entry_theta2 = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_theta2.grid(row=2, column=1, pady=5)
button_t2 = tk.Button(frame_inputs, text="Gửi", command=send_theta2, font=font_button, bg="#FF9800", fg="white", width=6)
button_t2.grid(row=2, column=2, padx=5)

# Theta 3
label_theta3 = tk.Label(frame_inputs, text="Theta 3 (°):", font=font_label, bg="#f0f0f5")
label_theta3.grid(row=3, column=0, padx=10, pady=5, sticky="w")
entry_theta3 = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_theta3.grid(row=3, column=1, pady=5)
button_t3 = tk.Button(frame_inputs, text="Gửi", command=send_theta3, font=font_button, bg="#3F51B5", fg="white", width=6)
button_t3.grid(row=3, column=2, padx=5)

# Nhãn "Vị trí (mm):"
label_result = tk.Label(frame_inputs, text="Tọa độ (mm):", font=font_label, bg="#f0f0f5")
label_result.grid(row=4, column=0, columnspan=3, padx=10, pady=(20, 5), sticky="w")

# X, Y, Z nằm ngang hàng và gần nhau hơn
label_x = tk.Label(frame_inputs, text="X:", font=font_label, bg="#f0f0f5")
label_x.grid(row=5, column=0, padx=(10, 2), pady=5, sticky="w")
entry_x = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
entry_x.grid(row=5, column=1, padx=(0, 5), pady=5)

label_y = tk.Label(frame_inputs, text="Y:", font=font_label, bg="#f0f0f5")
label_y.grid(row=5, column=2, padx=(5, 2), pady=5, sticky="w")
entry_y = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
entry_y.grid(row=5, column=3, padx=(0, 5), pady=5)

label_z = tk.Label(frame_inputs, text="Z:", font=font_label, bg="#f0f0f5")
label_z.grid(row=5, column=4, padx=(5, 2), pady=5, sticky="w")
entry_z = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
entry_z.grid(row=5, column=5, padx=(0, 10), pady=5)

label_inv_kinematic = tk.Label(frame_inputs, text="INVERSE KINEMATIC", font=font_label, bg="#f0f0f5", fg="#333")
label_inv_kinematic.grid(row=6, column=0, columnspan=3, padx=10, pady=(20, 5), sticky="w")

# Nhập X, Y, Z (Inverse Kinematic)
label_input_x = tk.Label(frame_inputs, text="X:", font=font_label, bg="#f0f0f5")
label_input_x.grid(row=7, column=0, padx=10, pady=5, sticky="w")
entry_input_x = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_input_x.grid(row=7, column=1, padx=5, pady=5)

label_input_y = tk.Label(frame_inputs, text="Y:", font=font_label, bg="#f0f0f5")
label_input_y.grid(row=8, column=0, padx=10, pady=5, sticky="w")
entry_input_y = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_input_y.grid(row=8, column=1, padx=5, pady=5)

label_input_z = tk.Label(frame_inputs, text="Z:", font=font_label, bg="#f0f0f5")
label_input_z.grid(row=9, column=0, padx=10, pady=5, sticky="w")
entry_input_z = tk.Entry(frame_inputs, font=font_entry, width=10)
entry_input_z.grid(row=9, column=1, padx=5, pady=5)

# Hiển thị kết quả Inverse Kinematic
label_out1 = tk.Label(frame_inputs, text="Theta 1:", font=font_label, bg="#f0f0f5")
label_out1.grid(row=7, column=2, padx=5, pady=5, sticky="w")
entry_out1 = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
entry_out1.grid(row=7, column=3, padx=(0, 10), pady=5)

label_out2 = tk.Label(frame_inputs, text="Theta 2:", font=font_label, bg="#f0f0f5")
label_out2.grid(row=8, column=2, padx=5, pady=5, sticky="w")
entry_out2 = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
entry_out2.grid(row=8, column=3, padx=(0, 10), pady=5)

label_out3 = tk.Label(frame_inputs, text="Theta 3:", font=font_label, bg="#f0f0f5")
label_out3.grid(row=9, column=2, padx=5, pady=5, sticky="w")
entry_out3 = tk.Entry(frame_inputs, font=font_entry, width=8, state='readonly')
entry_out3.grid(row=9, column=3, padx=(0, 10), pady=5)


# Nút điều khiển bên phải
frame_buttons = tk.Frame(frame_main, bg="#f0f0f5")
frame_buttons.pack(side=tk.RIGHT, padx=20, fill="y") # fill="y" để frame buttons cao bằng frame inputs

button_home = tk.Button(frame_buttons, text="SET HOME", command=set_home,
                        font=font_button, bg="#4CAF50", fg="white", width=15)
button_home.pack(pady=5, fill="x") # fill="x" để nút rộng bằng frame

button_send = tk.Button(frame_buttons, text="SEND DATA", command=send_angles,
                        font=font_button, bg="#008CBA", fg="white", width=15)
button_send.pack(pady=5, fill="x")

button_stop = tk.Button(frame_buttons, text="STOP", command=stop,
                        font=font_button, bg="#f44336", fg="white", width=15)
button_stop.pack(pady=5, fill="x")

button_hut = tk.Button(frame_buttons, text="UP", command=hut_namcham,
                       font=font_button, bg="#009688", fg="white", width=15)
button_hut.pack(pady=5, fill="x")

button_tha = tk.Button(frame_buttons, text="DOWN", command=tha_namcham,
                       font=font_button, bg="#795548", fg="white", width=15)
button_tha.pack(pady=5, fill="x")

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
