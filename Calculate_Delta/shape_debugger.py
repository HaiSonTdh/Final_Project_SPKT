import cv2
import numpy as np


def side_length_ratio(approx):
    sides = []
    for i in range(4):
        pt1 = approx[i][0]
        pt2 = approx[(i + 1) % 4][0]
        length = np.linalg.norm(pt1 - pt2)
        sides.append(length)
    max_len = max(sides)
    min_len = min(sides)
    return min_len / max_len if max_len != 0 else 0

def adaptive_epsilon(cnt):
    perimeter = cv2.arcLength(cnt, True)
    hull = cv2.convexHull(cnt)
    area = cv2.contourArea(cnt)
    hull_area = cv2.contourArea(hull)
    solidity = float(area) / hull_area if hull_area > 0 else 0

    if solidity < 0.85:
        eps_factor = 0.03
    elif solidity < 0.95:
        eps_factor = 0.02
    else:
        eps_factor = 0.01
    return eps_factor * perimeter


def detect_shape(cnt):
    epsilon = adaptive_epsilon(cnt)
    approx = cv2.approxPolyDP(cnt, epsilon, True)
    shape = "Unknown"

    hull = cv2.convexHull(cnt)
    area = cv2.contourArea(cnt)
    hull_area = cv2.contourArea(hull)
    solidity = float(area) / hull_area if hull_area > 0 else 0

    vertices = len(approx)

    (x, y, w, h) = cv2.boundingRect(cnt)
    aspect_ratio = float(w) / h if h != 0 else 1.0

    rect = cv2.minAreaRect(cnt)
    angle = rect[2]
    if angle < -45:
        angle += 90
    angle_deviation = min(abs(angle), abs(90 - angle))

    # Debug print
    print(
        f"Vertices: {vertices}, Solidity: {solidity:.2f}, Aspect Ratio: {aspect_ratio:.2f}, Angle Dev: {angle_deviation:.2f}, Epsilon: {epsilon:.2f}")

    # Detection logic
    if 3 <= vertices <= 5 and solidity > 0.85:
        shape = "Triangle"
    elif vertices == 4:
        if 0.9 <= aspect_ratio <= 1.1 and side_length_ratio(approx) > 0.85 and solidity > 0.9:
            shape = "Square"
        else:
            shape = "Rectangle"
    elif 10 <= vertices <= 15 and solidity < 0.85:
        shape = "Star"
    elif vertices > 15 and solidity > 0.95:
        shape = "Circle"

    return shape, approx


def main():
    cap = cv2.VideoCapture(1)  # Hoặc thay 0 bằng đường dẫn video

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        # Preprocess
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        blur = cv2.GaussianBlur(gray, (5, 5), 0)
        _, thresh = cv2.threshold(blur, 60, 255, cv2.THRESH_BINARY)

        contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        for cnt in contours:
            if cv2.contourArea(cnt) < 500:
                continue  # Bỏ qua vật thể nhỏ

            shape, approx = detect_shape(cnt)
            cv2.drawContours(frame, [approx], -1, (0, 255, 0), 2)
            M = cv2.moments(cnt)
            if M["m00"] != 0:
                cX = int(M["m10"] / M["m00"])
                cY = int(M["m01"] / M["m00"])
                cv2.putText(frame, shape, (cX - 30, cY), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)

        cv2.imshow("Shape Debugger", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
