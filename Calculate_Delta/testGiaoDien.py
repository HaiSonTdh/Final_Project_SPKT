import tkinter as tk
from tkinter import messagebox
import threading
import time
# import serial
import Kinematic
from PIL import Image, ImageTk

# ===== Giả lập Serial để test (xoá khi dùng thực tế) =====
class DummySerial:
    def write(self, data): print("Serial.write:", data)
    def readline(self): return b'Encoder feedback\n'
    def in_waiting(self): return True
ser = DummySerial()

# ===== Hàm xử lý =====
def send_theta(index, entry, label):
    try:
        val = float(entry.get())
        ser.write(f"{val:.2f}{label}\r".encode())
        print(f"Gửi: {val:.2f}{label}")
    except ValueError:
        messagebox.showerror("Lỗi", f"Theta {index} không hợp lệ!")

def send_angles():
    try:
        t1 = float(entry_theta1.get())
        t2 = float(entry_theta2.get())
        t3 = float(entry_theta3.get())
        if not (0 <= t1 <= 60 and 0 <= t2 <= 60 and 0 <= t3 <= 60):
            messagebox.showerror("Lỗi", "Các góc phải trong khoảng 0 - 60 độ")
            return
        ser.write(f"{t1}A{t2}B{t3}C\r".encode())
        result = Kinematic.forward_kinematic(t1, t2, t3)
        if not result[0]:
            messagebox.showerror("Lỗi", "Không thể tính vị trí.")
            return
        _, x, y, z = result
        for e, v in zip([entry_x, entry_y, entry_z], [x, y, z]):
            e.config(state='normal')
            e.delete(0, tk.END)
            e.insert(0, f"{v:.2f}")
            e.config(state='readonly')
    except ValueError:
        messagebox.showerror("Lỗi", "Nhập sai định dạng số.")

def set_home(): ser.write(b'h\r')
def stop(): ser.write(b's\r')
def hut_namcham(): ser.write(b'u\r')
def tha_namcham(): ser.write(b'd\r')

def read_serial():
    while True:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8').rstrip()
            text_box.insert(tk.END, line + "\n")
            text_box.see(tk.END)
        time.sleep(0.1)

# ===== GIAO DIỆN =====
window = tk.Tk()
window.title("Gửi góc điều khiển tới Arduino")
window.geometry("700x700")
window.configure(bg="#f0f0f5")

# ===== Banner ảnh =====
try:
    image = Image.open("Banner0.png")
    image = image.resize((680, 100))
    photo = ImageTk.PhotoImage(image)
    label_image = tk.Label(window, image=photo, bg="#f0f0f5")
    label_image.image = photo
    label_image.pack(pady=5)
except:
    pass

# ===== TIÊU ĐỀ =====
tk.Label(window, text="Forward kinematic", font=("Arial", 16, "bold"), bg="#f0f0f5").pack()

# ===== FRAME CHÍNH =====
main_frame = tk.Frame(window, bg="#f0f0f5")
main_frame.pack(pady=10)

# ===== VÙNG NHẬP GÓC & GỬI =====
theta_frame = tk.Frame(main_frame, bg="#f0f0f5")
theta_frame.grid(row=0, column=0, padx=10)

entry_theta1 = tk.Entry(theta_frame, width=10, bg="lightgray", justify="center")
entry_theta1.grid(row=0, column=0, padx=5, pady=3)
btn1 = tk.Button(theta_frame, text="Send theta 1", bg="yellow", width=15, command=lambda: send_theta(1, entry_theta1, "A"))
btn1.grid(row=0, column=1, padx=5)

entry_theta2 = tk.Entry(theta_frame, width=10, bg="lightgray", justify="center")
entry_theta2.grid(row=1, column=0, padx=5, pady=3)
btn2 = tk.Button(theta_frame, text="Send theta 2", bg="yellow", width=15, command=lambda: send_theta(2, entry_theta2, "B"))
btn2.grid(row=1, column=1, padx=5)

entry_theta3 = tk.Entry(theta_frame, width=10, bg="lightgray", justify="center")
entry_theta3.grid(row=2, column=0, padx=5, pady=3)
btn3 = tk.Button(theta_frame, text="Send theta 3", bg="yellow", width=15, command=lambda: send_theta(3, entry_theta3, "C"))
btn3.grid(row=2, column=1, padx=5)

# ===== VỊ TRÍ KẾT QUẢ =====
position_frame = tk.Frame(main_frame, bg="#f0f0f5")
position_frame.grid(row=1, column=0, pady=20)
tk.Label(window, text="Inverse kinematic", font=("Arial", 16, "bold"), bg="#f0f0f5").pack()

for i, (label, var) in enumerate([("X", "entry_x"), ("Y", "entry_y"), ("Z", "entry_z")]):
    tk.Label(position_frame, text=f"{label}:", font=("Arial", 12), bg="#f0f0f5").grid(row=i, column=0, padx=5, sticky="e")
    e = tk.Entry(position_frame, width=20, bg="lightgray", justify="center", state="readonly")
    e.grid(row=i, column=1, pady=3)
    globals()[var] = e

# ===== VÙNG NÚT BÊN PHẢI =====
right_frame = tk.Frame(main_frame, bg="#f0f0f5")
right_frame.grid(row=0, column=1, rowspan=2, padx=30, sticky="n")

control_buttons = [
    ("SET HOME", set_home),
    ("UP", hut_namcham),
    ("SEND DATA", send_angles),
    ("DOWN", tha_namcham),
    ("STOP", stop),
]

for i, (label, cmd) in enumerate(control_buttons):
    btn = tk.Button(right_frame, text=label, command=cmd, bg="yellow", font=("Arial", 10, "bold"), width=15)
    btn.grid(row=i, column=0, pady=5)

# ===== ENCODER =====
tk.Label(window, text="Encoder", font=("Arial", 14, "bold"), bg="#f0f0f5").pack(pady=(20, 5))
text_box = tk.Text(window, width=70, height=8, font=("Courier New", 11))
text_box.pack()

# ===== THREAD ĐỌC SERIAL =====
threading.Thread(target=read_serial, daemon=True).start()
window.mainloop()
