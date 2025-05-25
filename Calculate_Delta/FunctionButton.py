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