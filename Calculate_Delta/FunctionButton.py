import tkinter as tk
from tkinter import messagebox
import Kinematic # Giữ lại nếu các hàm khác trong FunctionButton.py có dùng
import cv2
from PIL import Image, ImageTk
import ObjectDetection # Import module ObjectDetection đã sửa
import time

# Biến cục bộ cho module này (dùng cho camera)
# Đổi tên để tránh trùng với biến ở main nếu có
bh_cap = None
bh_camera_running = False
# Kích thước hiển thị mong muốn trên GUI
DISPLAY_WIDTH = 720
DISPLAY_HEIGHT = 370

# Lưu ý:
# - 'send_command_func' sẽ là hàm send_command_to_serial từ file chính
# - Các 'entry_...' là các đối tượng widget Entry từ file chính
# - Các 'btn_...' là các đối tượng widget Button từ file chính (nếu cần thay đổi text/bg của chúng)

def send_angles_handler(entry_theta1, entry_theta2, entry_theta3,
                        entry_x, entry_y, entry_z, ser_object):  # Truyền ser_object
    try:
        theta1 = float(entry_theta1.get())
        theta2 = float(entry_theta2.get())
        theta3 = float(entry_theta3.get())

        if not (0 <= theta1 <= 60):
            messagebox.showerror("Error", "Theta 1 phải trong khoảng 0 đến 60")  # Sửa giới hạn nếu cần
            return
        if not (0 <= theta2 <= 60):
            messagebox.showerror("Error", "Theta 2 phải trong khoảng 0 đến 60")  # Sửa giới hạn nếu cần
            return
        if not (0 <= theta3 <= 60):
            messagebox.showerror("Error", "Theta 3 phải trong khoảng 0 đến 60")  # Sửa giới hạn nếu cần
            return

        try:
            result = Kinematic.forward_kinematic(theta1, theta2, theta3)
            if result[0] is False:
                messagebox.showerror("Error", "Không thể tính vị trí. Kiểm tra lại góc đầu vào.")
                return

            _, x, y, z = result

            if not (-100 <= x <= 87):
                messagebox.showerror("Lỗi", f"X={x:.2f} nằm ngoài giới hạn robot")
                return
            if not (-80 <= y <= 130):
                messagebox.showerror("Lỗi", f"Y={y:.2f} nằm ngoài giới hạn robot")
                return
            if not (-397 <= z <= -307.38):
                messagebox.showerror("Lỗi", f"Z={z:.2f} nằm ngoài giới hạn robot")
                return

            data = f"{theta1}A{theta2}B{theta3}C\r"
            print(f"Gửi: {data.strip()}")
            if ser_object and ser_object.is_open:
                ser_object.write(data.encode())
            else:
                messagebox.showwarning("Serial Port Error", "Serial port not available.")
                return

            entry_x.config(state='normal')
            entry_y.config(state='normal')
            entry_z.config(state='normal')
            entry_x.delete(0, tk.END);
            entry_x.insert(0, f"{x:.2f}")
            entry_y.delete(0, tk.END);
            entry_y.insert(0, f"{y:.2f}")
            entry_z.delete(0, tk.END);
            entry_z.insert(0, f"{z:.2f}")
            entry_x.config(state='readonly')
            entry_y.config(state='readonly')
            entry_z.config(state='readonly')

        except Exception as e:
            print(f"Lỗi khi tính forward kinematic: {e}")
            messagebox.showerror("Lỗi", "Không thể tính vị trí. Kiểm tra lại hàm forward_kinematic.")

    except ValueError:
        messagebox.showerror("Error", "Vui lòng nhập đúng định dạng là số!")

def calculate_inv_kinematic_handler(entry_x_ik, entry_y_ik, entry_z_ik,
                                    entry_theta1_ik, entry_theta2_ik, entry_theta3_ik):
    try:
        x_val = float(entry_x_ik.get())
        y_val = float(entry_y_ik.get())
        z_val = float(entry_z_ik.get())

        angles = Kinematic.inverse_kinematic(x_val, y_val, z_val)

        entry_theta1_ik.config(state=tk.NORMAL);
        entry_theta1_ik.delete(0, tk.END);
        entry_theta1_ik.insert(0, f"{angles[0]:.2f}");
        entry_theta1_ik.config(state='readonly')
        entry_theta2_ik.config(state=tk.NORMAL);
        entry_theta2_ik.delete(0, tk.END);
        entry_theta2_ik.insert(0, f"{angles[1]:.2f}");
        entry_theta2_ik.config(state='readonly')
        entry_theta3_ik.config(state=tk.NORMAL);
        entry_theta3_ik.delete(0, tk.END);
        entry_theta3_ik.insert(0, f"{angles[2]:.2f}");
        entry_theta3_ik.config(state='readonly')

    except ValueError:
        messagebox.showerror("Lỗi", "Vui lòng nhập giá trị số hợp lệ cho X, Y, Z.")
    except Exception as e:  # Bắt lỗi cụ thể từ Kinematic.inverse_kinematic nếu có
        messagebox.showerror("Lỗi tính toán", str(e))


def send_trajectory_handler(entry_x0, entry_y0, entry_z0, entry_c0,
                            entry_xf, entry_yf, entry_zf, entry_tf,
                            send_command_func):  # send_command_func là hàm send_command_to_serial
    try:
        x0_val = float(entry_x0.get())
        y0_val = float(entry_y0.get())
        z0_val = float(entry_z0.get())
        c0_val = entry_c0.get().strip().upper()
        xf_val = float(entry_xf.get())
        yf_val = float(entry_yf.get())
        zf_val = float(entry_zf.get())
        tf_val = float(entry_tf.get())

        # Kiểm tra giới hạn
        if not (-100 <= x0_val <= 87): messagebox.showerror("Lỗi", f"X0={x0_val:.2f} nằm ngoài giới hạn robot"); return
        if not (-80 <= y0_val <= 130): messagebox.showerror("Lỗi", f"Y0={y0_val:.2f} nằm ngoài giới hạn robot"); return
        if not (-397 <= z0_val <= -307.38): messagebox.showerror("Lỗi",
                                                                 f"Z0={z0_val:.2f} nằm ngoài giới hạn robot"); return
        if not (-100 <= xf_val <= 87): messagebox.showerror("Lỗi", f"Xf={xf_val:.2f} nằm ngoài giới hạn robot"); return
        if not (-80 <= yf_val <= 130): messagebox.showerror("Lỗi", f"Yf={yf_val:.2f} nằm ngoài giới hạn robot"); return
        if not (-397 <= zf_val <= -307.38): messagebox.showerror("Lỗi",
                                                                 f"Zf={zf_val:.2f} nằm ngoài giới hạn robot"); return

        base_data_segment = f"P0:{x0_val},{y0_val},{z0_val};Pf:{xf_val},{yf_val},{zf_val};T:{tf_val}"
        data_to_send = ""

        if c0_val and c0_val not in ['R', 'G', 'Y']:
            messagebox.showerror("Lỗi", f"Nhập C0 là R,G hoặc Y")
            return
        else:
            data_to_send = f"{base_data_segment}\r"

        if send_command_func(data_to_send):
            entry_x0.delete(0, tk.END);
            entry_x0.insert(0, str(xf_val))
            entry_y0.delete(0, tk.END);
            entry_y0.insert(0, str(yf_val))
            entry_z0.delete(0, tk.END);
            entry_z0.insert(0, str(zf_val))
            entry_xf.delete(0, tk.END)
            entry_yf.delete(0, tk.END)
            entry_zf.delete(0, tk.END)
            entry_tf.delete(0, tk.END)
            entry_c0.delete(0, tk.END)

    except ValueError:
        messagebox.showerror("Lỗi", "Vui lòng nhập giá trị số hợp lệ cho tọa độ và thời gian.")
    except Exception as e:
        messagebox.showerror("Lỗi", f"Đã xảy ra lỗi: {str(e)}")

# def set_home_handler(send_command_func, entry_x, entry_y, entry_z):
#     if send_command_func('h\r'):
#         try:
#             result = Kinematic.forward_kinematic(0, 0, 0)
#             entry_x.config(state='normal'); entry_x.delete(0, tk.END); entry_x.insert(0, f"{result[0]:.2f}"); entry_x.config(state='readonly')
#             entry_y.config(state='normal'); entry_y.delete(0, tk.END); entry_y.insert(0, f"{result[1]:.2f}"); entry_y.config(state='readonly')
#             entry_z.config(state='normal'); entry_z.delete(0, tk.END); entry_z.insert(0, f"{result[2]:.2f}"); entry_z.config(state='readonly')
#
#             # Re-enable lại các nút điều khiển
#             import __main__  # Truy cập biến từ file chính
#             for btn in __main__.controllable_buttons:
#                 btn.config(state=tk.NORMAL)
#
#         except Exception as e:
#             print(f"Lỗi khi tính toán hoặc cập nhật: {e}")
def set_home_handler(send_command_func, entry_x, entry_y, entry_z):
    if send_command_func('h\r'):
        try:
            result = Kinematic.forward_kinematic(0, 0, 0)
            entry_x.config(state='normal')
            entry_x.delete(0, tk.END)
            entry_x.insert(0, f"{result[0]:.2f}")
            entry_x.config(state='readonly')

            entry_y.config(state='normal')
            entry_y.delete(0, tk.END)
            entry_y.insert(0, f"{result[1]:.2f}")
            entry_y.config(state='readonly')

            entry_z.config(state='normal')
            entry_z.delete(0, tk.END)
            entry_z.insert(0, f"{result[2]:.2f}")
            entry_z.config(state='readonly')

            # Re-enable lại các nút điều khiển
            import __main__  # Truy cập biến từ file chính
            for btn in __main__.controllable_buttons:
                btn.config(state=tk.NORMAL)

            return True  #  Thành công
        except Exception as e:
            print(f"Lỗi khi tính toán hoặc cập nhật: {e}")
            return False  # Có lỗi xảy ra khi xử lý
    else:
        return False  # Gửi lệnh thất bại


def simple_command_handler(send_command_func, command):
    send_command_func(command)


def start_camera_handler(label_cam_widget, serial_object):  # Thêm serial_object
    global bh_cap, bh_camera_running

    if not bh_camera_running:
        try:
            bh_cap = cv2.VideoCapture(0)  # Hoặc index camera của bạn

            if not bh_cap or not bh_cap.isOpened():
                messagebox.showerror("Camera Error", "Không thể mở camera. Hãy kiểm tra kết nối.")
                if bh_cap:
                    bh_cap.release()
                bh_cap = None
                return

            # Thiết lập kích thước frame từ camera, nên giống với kích thước mà ObjectDetection.py kỳ vọng
            # process_frame_for_detection đang làm việc với frame 640x480 (do ROI_Y2 = 480)
            bh_cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
            bh_cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

            bh_camera_running = True
            ObjectDetection.reset_detection_state()  # Reset trạng thái nhận diện khi bắt đầu

            update_frame_handler(label_cam_widget, serial_object)  # Truyền serial_object

        except Exception as e:
            messagebox.showerror("Camera Error", f"Lỗi khi khởi động camera: {e}")
            bh_camera_running = False
            if bh_cap and bh_cap.isOpened():
                bh_cap.release()
            bh_cap = None


def stop_camera_stream_handler(label_cam_widget):
    global bh_cap, bh_camera_running
    bh_camera_running = False  # Tín hiệu dừng vòng lặp update_frame_handler
    if bh_cap is not None:
        bh_cap.release()
        bh_cap = None

    ObjectDetection.reset_detection_state()  # Reset trạng thái khi dừng camera

    # Xóa ảnh khỏi label, hiển thị ảnh đen
    try:
        if label_cam_widget.winfo_exists():
            black_img = Image.new('RGB', (DISPLAY_WIDTH, DISPLAY_HEIGHT), color='black')
            imgtk = ImageTk.PhotoImage(image=black_img)
            label_cam_widget.imgtk = imgtk
            label_cam_widget.config(image=imgtk)
    except tk.TclError:  # Widget có thể đã bị hủy
        pass


def update_frame_handler(label_cam_widget, serial_object):  # Thêm serial_object
    global bh_cap, bh_camera_running

    if bh_camera_running and bh_cap and bh_cap.isOpened():
        ret, frame_from_cam = bh_cap.read()
        if ret:
            # Gọi hàm xử lý từ ObjectDetection
            # Hàm này sẽ trả về frame đã được vẽ vời các thông tin nhận diện
            processed_frame = ObjectDetection.process_frame_for_detection(frame_from_cam, serial_object)

            if processed_frame is not None:
                # Resize frame đã xử lý về kích thước hiển thị mong muốn
                frame_display_resized = cv2.resize(processed_frame, (DISPLAY_WIDTH, DISPLAY_HEIGHT))
                frame_rgb = cv2.cvtColor(frame_display_resized, cv2.COLOR_BGR2RGB)
                img = Image.fromarray(frame_rgb)
                imgtk = ImageTk.PhotoImage(image=img)

                if label_cam_widget.winfo_exists():
                    label_cam_widget.imgtk = imgtk
                    label_cam_widget.config(image=imgtk)
            else:
                print("Processed frame is None.")  # Xử lý lỗi nếu cần

            # Lặp lại việc cập nhật frame
            if label_cam_widget.winfo_exists() and bh_camera_running:
                label_cam_widget.after(10, lambda: update_frame_handler(label_cam_widget, serial_object))
            else:  # Nếu widget không còn hoặc camera đã dừng
                if bh_camera_running:  # Nếu camera vẫn được cho là đang chạy nhưng widget mất
                    stop_camera_stream_handler(label_cam_widget)  # Dừng hẳn
        else:
            print("Không đọc được frame từ camera.")
            # Có thể dừng camera ở đây nếu đọc frame thất bại liên tục
            # stop_camera_stream_handler(label_cam_widget)
            if label_cam_widget.winfo_exists() and bh_camera_running:  # Vẫn thử lại nếu camera chưa bị stop
                label_cam_widget.after(10, lambda: update_frame_handler(label_cam_widget, serial_object))

# def start_camera_handler(label_cam_widget):
#     global bh_cap, bh_camera_running # Sử dụng các biến của module button_handlers
#
#     if not bh_camera_running:
#         try:
#             # Giống Hàm 2: chỉ thử mở camera 0
#             bh_cap = cv2.VideoCapture(0)
#
#             # Kiểm tra xem camera có thực sự mở được không (điều này Hàm 2 không làm, nhưng rất nên có)
#             if not bh_cap or not bh_cap.isOpened():
#                 messagebox.showerror("Camera Error", "Không thể mở camera (index 0). Hãy kiểm tra kết nối.")
#                 if bh_cap: # Nếu bh_cap được tạo nhưng không mở được, thử release
#                     bh_cap.release()
#                 bh_cap = None
#                 bh_camera_running = False # Đảm bảo trạng thái là false
#                 return # Thoát nếu không mở được camera
#
#             # Tiếp tục với logic cài đặt và chạy, giống Hàm 2 (và Hàm 1 gốc)
#             bh_cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
#             bh_cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
#             bh_camera_running = True
#
#             # Gọi hàm update_frame_handler (đã được điều chỉnh để phù hợp với button_handlers.py)
#             update_frame_handler(label_cam_widget)
#
#         except Exception as e:
#             # Bắt các lỗi khác có thể xảy ra trong quá trình set thuộc tính hoặc lỗi không mong muốn
#             messagebox.showerror("Camera Error", f"Lỗi khi khởi động camera: {e}")
#             bh_camera_running = False
#             if bh_cap and bh_cap.isOpened(): # Chỉ release nếu nó đã được mở
#                 bh_cap.release()
#             bh_cap = None # Đặt lại để đảm bảo an toàn
#
# def stop_camera_stream_handler(label_cam_widget):
#     global bh_cap, bh_camera_running
#     bh_camera_running = False
#     if bh_cap is not None:
#         bh_cap.release()
#         bh_cap = None
#     # Xóa ảnh khỏi label, có thể đặt lại kích thước nếu muốn
#     black_img = Image.new('RGB', (720, 370), color='black')
#     imgtk = ImageTk.PhotoImage(image=black_img)
#     label_cam_widget.imgtk = imgtk
#     label_cam_widget.config(image=imgtk)
#
#
# def update_frame_handler(label_cam_widget):
#     global bh_cap, bh_camera_running
#     if bh_camera_running and bh_cap and bh_cap.isOpened():
#         ret, frame = bh_cap.read()
#         if ret:
#             frame_resized = cv2.resize(frame, (720, 370))
#             frame_rgb = cv2.cvtColor(frame_resized, cv2.COLOR_BGR2RGB)
#             img = Image.fromarray(frame_rgb)
#             imgtk = ImageTk.PhotoImage(image=img)
#             label_cam_widget.imgtk = imgtk
#             label_cam_widget.config(image=imgtk)
#         # Sử dụng label_cam_widget.after thay vì window.after
#         label_cam_widget.after(10, lambda: update_frame_handler(label_cam_widget))
#     elif not bh_camera_running and label_cam_widget.winfo_exists():  # Nếu camera đã dừng, dừng cập nhật
#         stop_camera_stream_handler(label_cam_widget)


def toggle_namcham_handler(current_magnet_state, btn_namcham_widget, send_command_func):
    cmd = 'u\r' if current_magnet_state == 0 else 'd\r'
    if send_command_func(cmd):
        if current_magnet_state == 0:
            btn_namcham_widget.config(text="ON MAG", bg="#3bd952")
            return 1  # Trả về trạng thái mới
        else:
            btn_namcham_widget.config(text="OFF MAG", bg="#eb3b3b")
            return 0  # Trả về trạng thái mới
    return None  # Trả về None nếu gửi lệnh thất bại, để không thay đổi trạng thái


def toggle_conveyor_handler(current_conveyor_state, btn_bangtai_widget, send_command_func):
    # Giả sử 'w' là ON và 'z' là OFF cho băng tải, kiểm tra lại mã Arduino/STM32 của bạn
    # Thông thường, các lệnh điều khiển sẽ khác nhau (ví dụ: 'E' cho Enable, 'D' cho Disable)
    # cmd = 'E\r' if current_conveyor_state == 0 else 'D\r' # Ví dụ
    cmd = 'w\r' if current_conveyor_state == 0 else 'z\r'  # Theo code gốc của bạn
    if send_command_func(cmd):
        if current_conveyor_state == 0:
            btn_bangtai_widget.config(text="ON CONV", bg="#3bd952")
            return 1
        else:
            btn_bangtai_widget.config(text="OFF CONV", bg="#eb3b3b")
            return 0
    return None